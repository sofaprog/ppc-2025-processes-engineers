#include "makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

#include <mpi.h>
#include <string>

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
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  const std::string& text = GetInput();
  std::size_t text_length = text.length();
  
  std::size_t chunk_size = text_length / size;
  std::size_t start = rank * chunk_size;
  std::size_t end = (rank == size - 1) ? text_length : start + chunk_size;
  
  std::size_t local_count = 0;
  for (std::size_t i = start; i < end; i++) {
    char c = text[i];
    if (c == '.' || c == '!' || c == '?') {
      local_count++;
    }
  }
  
  std::size_t global_count = 0;
  MPI_Reduce(&local_count, &global_count, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if (rank == 0) {
    GetOutput() = global_count;
  }
  
  return true;
}

bool SentencesCounterMPI::PostProcessingImpl() {
  return true;
}

}  // namespace makoveeva_s_number_of_sentence