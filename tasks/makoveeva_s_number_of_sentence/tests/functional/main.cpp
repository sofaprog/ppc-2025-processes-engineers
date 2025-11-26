#include <mpi.h>

#include <algorithm>
#include <string>

#include "makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

namespace makoveeva_s_number_of_sentence {

SentencesCounterMPI::SentencesCounterMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SentencesCounterMPI::ValidationImpl() {
  return true;
}

bool SentencesCounterMPI::PreProcessingImpl() {
  return true;
}

bool SentencesCounterMPI::RunImpl() {
  int rank = 0, size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::string text;
  int text_length = 0;

  if (rank == 0) {
    text = GetInput();
    text_length = static_cast<int>(text.length());
  }

  MPI_Bcast(&text_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    text.resize(text_length);
  }
  MPI_Bcast(text.data(), text_length, MPI_CHAR, 0, MPI_COMM_WORLD);

  // Если текст пустой
  if (text_length == 0) {
    GetOutput() = 0;
    return true;
  }

  // Распределение работы между процессами
  int chunk_size = text_length / size;
  int remainder = text_length % size;

  int start = rank * chunk_size + std::min(rank, remainder);
  int end = (rank + 1) * chunk_size + std::min(rank + 1, remainder);
  end = std::min(end, text_length);

  // Локальный подсчет предложений
  int local_count = 0;
  bool found_sentence_end = false;

  for (int i = start; i < end; i++) {
    char c = text[i];
    if (c == '.' || c == '!' || c == '?') {
      if (!found_sentence_end) {
        // Нашли конец предложения
        found_sentence_end = true;
        local_count++;
      }
      // Продолжаем искать следующий конец предложения
    } else {
      // Если нашли не-пунктуационный символ, сбрасываем флаг
      found_sentence_end = false;
    }
  }

  // Теперь нужно обработать границы между процессами
  // Основная проблема: если предложение заканчивается на границе между процессами

  int global_count = 0;
  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  // Корректировка для случая, когда несколько процессов "видят" одно и то же предложение
  if (size > 1) {
    // Собираем информацию о том, заканчивается ли каждый чанк знаком препинания
    // и начинается ли следующий чанк после знака препинания

    bool current_ends_with_punct = false;
    if (end > start) {
      char last_char = text[end - 1];
      current_ends_with_punct = (last_char == '.' || last_char == '!' || last_char == '?');
    }

    bool next_starts_with_punct = false;
    if (rank < size - 1 && end < text_length) {
      char first_char_next = text[end];
      next_starts_with_punct = (first_char_next == '.' || first_char_next == '!' || first_char_next == '?');
    }

    // Обмениваемся информацией о границах
    bool prev_ends_with_punct = false;
    if (rank > 0) {
      MPI_Recv(&prev_ends_with_punct, 1, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    if (rank < size - 1) {
      MPI_Send(&current_ends_with_punct, 1, MPI_C_BOOL, rank + 1, 0, MPI_COMM_WORLD);
    }

    // Корректируем счет на root процессе
    if (rank == 0) {
      // Дополнительная корректировка для случаев типа "abc..def"
      // где точка на границе процессов может быть учтена дважды
      int correction = 0;

      // Собираем информацию от всех процессов о границах
      bool *ends_with_punct_arr = new bool[size];
      bool *starts_with_punct_arr = new bool[size];

      MPI_Gather(&current_ends_with_punct, 1, MPI_C_BOOL, ends_with_punct_arr, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

      bool starts_with_punct =
          (start > 0) ? (text[start - 1] == '.' || text[start - 1] == '!' || text[start - 1] == '?') : false;
      MPI_Gather(&starts_with_punct, 1, MPI_C_BOOL, starts_with_punct_arr, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

      // Корректируем: если предыдущий чанк заканчивается точкой и текущий начинается после точки,
      // и при этом в текущем чанке первый символ не точка, то это одно предложение
      for (int i = 1; i < size; i++) {
        if (ends_with_punct_arr[i - 1] && starts_with_punct_arr[i]) {
          // Проверяем, не является ли это разделенным предложением
          int prev_chunk_end = (i)*chunk_size + std::min(i, remainder) - 1;
          int current_chunk_start = i * chunk_size + std::min(i, remainder);

          if (prev_chunk_end >= 0 && current_chunk_start < text_length) {
            char prev_char = text[prev_chunk_end];
            char current_char = text[current_chunk_start];

            // Если оба знака препинания, но это разные предложения (например, "abc..def")
            // то не корректируем. Корректируем только если это одно предложение,
            // разделенное между процессами
            if (prev_char == current_char) {
              // Это случаи типа ".." - две точки подряд, значит два разных предложения
              // Не корректируем
            } else {
              // Возможно, это одно предложение, разделенное между процессами
              correction--;
            }
          }
        }
      }

      global_count += correction;
      delete[] ends_with_punct_arr;
      delete[] starts_with_punct_arr;
    }
  }

  // Распространяем результат на все процессы
  MPI_Bcast(&global_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = global_count;
  return true;
}

bool SentencesCounterMPI::PostProcessingImpl() {
  return true;
}

}  // namespace makoveeva_s_number_of_sentence
