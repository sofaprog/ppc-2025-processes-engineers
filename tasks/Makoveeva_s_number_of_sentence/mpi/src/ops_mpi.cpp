#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <string>

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"

namespace makoveeva_s_number_of_sentence {

SentencesCounterMPI::SentencesCounterMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SentencesCounterMPI::ValidationImpl() {
  return (GetOutput() == 0);
}

bool SentencesCounterMPI::PreProcessingImpl() {
  return true;
}

bool SentencesCounterMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::string local_text;
  int text_length = 0;

  if (rank == 0) {
    local_text = GetInput();
    text_length = local_text.length();
  }

  MPI_Bcast(&text_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    local_text.resize(text_length);
  }

  MPI_Bcast(&local_text[0], text_length, MPI_CHAR, 0, MPI_COMM_WORLD);

  int chunk_size = text_length / size;
  int remainder = text_length % size;

  int start = rank * chunk_size + std::min(rank, remainder);
  int end = start + chunk_size + (rank < remainder ? 1 : 0);

  int local_count = 0;
  for (int i = start; i < end && i < text_length; i++) {
    char c = local_text[i];
    if (c == '.' || c == '!' || c == '?') {
      local_count++;
    }
  }

  int global_count = 0;
  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    GetOutput() = global_count;
  }

  return true;
}

bool SentencesCounterMPI::PostProcessingImpl() {
  return true;
}

}  // namespace makoveeva_s_number_of_sentence
