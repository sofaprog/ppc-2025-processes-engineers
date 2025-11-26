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

  int chunk_size = text_length / size;
  int remainder = text_length % size;
  int start = rank * chunk_size + std::min(rank, remainder);
  int end = start + chunk_size + (rank < remainder ? 1 : 0);
  end = std::min(end, text_length);

  int local_count = 0;
  bool local_ends_with_punct = false;
  bool prev_is_punct = false;

  for (int i = start; i < end; i++) {
    char c = text[i];
    if (c == '.' || c == '!' || c == '?') {
      if (!prev_is_punct) {
        local_count++;
      }
      prev_is_punct = true;
    } else {
      prev_is_punct = false;
    }
  }

  if (end > start && end <= text_length) {
    char last_char = text[end - 1];
    local_ends_with_punct = (last_char == '.' || last_char == '!' || last_char == '?');
  }

  if (size > 1) {
    bool receives_punct_from_prev = false;

    if (rank > 0) {
      MPI_Recv(&receives_punct_from_prev, 1, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    if (rank < size - 1) {
      MPI_Send(&local_ends_with_punct, 1, MPI_C_BOOL, rank + 1, 0, MPI_COMM_WORLD);
    }

    if (rank > 0 && receives_punct_from_prev && start < text_length) {
      char first_char = text[start];
      if (first_char == '.' || first_char == '!' || first_char == '?') {
        if (local_count > 0) {
          local_count--;
        }
      }
    }
  }

  int global_count = 0;
  MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  MPI_Bcast(&global_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = global_count;

  return true;
}

bool SentencesCounterMPI::PostProcessingImpl() {
  return true;
}

}  // namespace makoveeva_s_number_of_sentence
