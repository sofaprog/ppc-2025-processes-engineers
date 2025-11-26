#include "makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <string>

#include "makoveeva_s_number_of_sentence/common/include/common.hpp"

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

  // Распределение работы между процессами
  int chunk_size = text_length / size;
  int remainder = text_length % size;

  int start = rank * chunk_size + std::min(rank, remainder);
  int end = (rank + 1) * chunk_size + std::min(rank + 1, remainder);
  end = std::min(end, text_length);

  // Локальный подсчет предложений
  int local_count = 0;
  bool in_sentence_end = false;

  for (int i = start; i < end; i++) {
    char c = text[i];
    if (c == '.' || c == '!' || c == '?') {
      if (!in_sentence_end) {
        local_count++;
        in_sentence_end = true;
      }
    } else {
      in_sentence_end = false;
    }
  }

  // Определяем, заканчивается ли наш чанк знаком препинания
  bool ends_with_punct = false;
  if (end > start) {
    char last_char = text[end - 1];
    ends_with_punct = (last_char == '.' || last_char == '!' || last_char == '?');
  }

  // Собираем информацию о границах
  bool *all_ends_with_punct = nullptr;
  bool *all_starts_after_punct = nullptr;

  if (rank == 0) {
    all_ends_with_punct = new bool[size];
    all_starts_after_punct = new bool[size];
  }

  MPI_Gather(&ends_with_punct, 1, MPI_C_BOOL, all_ends_with_punct, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

  // Определяем, начинается ли чанк после знака препинания
  bool starts_after_punct = false;
  if (start > 0) {
    char prev_char = text[start - 1];
    starts_after_punct = (prev_char == '.' || prev_char == '!' || prev_char == '?');
  }

  MPI_Gather(&starts_after_punct, 1, MPI_C_BOOL, all_starts_after_punct, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

  // Корректируем общий счет на root процессе
  int global_count = 0;
  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    // Корректируем двойной подсчет разделенных предложений
    for (int i = 1; i < size; i++) {
      if (all_ends_with_punct[i - 1] && all_starts_after_punct[i]) {
        // Если предыдущий чанк заканчивается знаком препинания,
        // а текущий начинается после знака препинания - это одно предложение
        global_count--;
      }
    }

    delete[] all_ends_with_punct;
    delete[] all_starts_after_punct;
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
