#include "makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <string>

namespace makoveeva_s_number_of_sentence {

SentencesCounterMPI::SentencesCounterMPI(const std::string &input) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = input;
  GetOutput() = 0;
}

bool SentencesCounterMPI::ValidationImpl() {
  return true;
}

bool SentencesCounterMPI::PreProcessingImpl() {
  return true;
  //
}

bool SentencesCounterMPI::RunImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // СУПЕР-УПРОЩЕННАЯ ВЕРСИЯ: только процесс 0 работает
  if (rank == 0) {
    const std::string &text = GetInput();
    int count = 0;

    // Простейший подсчет без сложной логики
    for (size_t i = 0; i < text.length(); ++i) {
      char c = text[i];
      if (c == '.' || c == '!' || c == '?') {
        count++;
      }
    }

    GetOutput() = count;
  } else {
    // Остальные процессы просто возвращают 0
    GetOutput() = 0;
  }

  return true;
}

bool SentencesCounterMPI::PostProcessingImpl() {
  return true;
}

}  // namespace makoveeva_s_number_of_sentence
