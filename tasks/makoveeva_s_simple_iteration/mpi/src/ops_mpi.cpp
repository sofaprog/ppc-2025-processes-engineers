#include "makoveeva_s_simple_iteration/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "makoveeva_s_simple_iteration/common/include/common.hpp"

namespace makoveeva_s_simple_iteration {

namespace {
constexpr double kRelaxationFactor = 0.5;
constexpr double kEpsilon = 1e-6;
constexpr int kMaxIterations = 1000;

void CalculateRowsDistribution(int n, int size, std::vector<int> &row_counts, std::vector<int> &row_displs,
                               std::vector<int> &matrix_counts, std::vector<int> &matrix_displs) {
  int row_offset = 0;
  int matrix_offset = 0;
  for (int proc = 0; proc < size; ++proc) {
    int base_rows = n / size;
    int extra = (proc < (n % size)) ? 1 : 0;
    int proc_rows = base_rows + extra;

    row_counts[proc] = proc_rows;
    row_displs[proc] = row_offset;
    matrix_counts[proc] = proc_rows * n;
    matrix_displs[proc] = matrix_offset;

    row_offset += proc_rows;
    matrix_offset += proc_rows * n;
  }
}

void InitializeMatrixAndVector(std::vector<double> &flat_matrix, std::vector<double> &b, int n) {
  const auto matrix_size = static_cast<size_t>(n) * static_cast<size_t>(n);
  flat_matrix.resize(matrix_size, 0.0);
  b.resize(static_cast<size_t>(n), 0.0);

  for (int i = 0; i < n; ++i) {
    const auto i_idx = static_cast<size_t>(i);
    const auto n_idx = static_cast<size_t>(n);

    flat_matrix[(i_idx * n_idx) + i_idx] = static_cast<double>(n) + 5.0;

    for (int j = 0; j < n; ++j) {
      if (i != j) {
        const auto j_idx = static_cast<size_t>(j);
        flat_matrix[(i_idx * n_idx) + j_idx] = 1.0 / (static_cast<double>(std::abs(i - j)) + 1.0);
      }
    }

    for (int j = 0; j < n; ++j) {
      const auto j_idx = static_cast<size_t>(j);
      b[i_idx] += flat_matrix[(i_idx * n_idx) + j_idx] * static_cast<double>(j + 1);
    }
  }
}

double ComputeLocalProduct(const std::vector<double> &local_matrix, const std::vector<double> &x,
                           const std::vector<double> &local_b, std::vector<double> &local_x_new, int local_rows,
                           int start_row, int n) {
  double local_diff = 0.0;

  for (int i = 0; i < local_rows; ++i) {
    const auto i_idx = static_cast<size_t>(i);
    const auto n_idx = static_cast<size_t>(n);

    double sum = 0.0;
    for (int j = 0; j < n; ++j) {
      const auto j_idx = static_cast<size_t>(j);
      sum += local_matrix[(i_idx * n_idx) + j_idx] * x[j_idx];
    }

    const int global_i = start_row + i;
    const auto global_idx = static_cast<size_t>(global_i);

    local_x_new[i_idx] =
        x[global_idx] + (kRelaxationFactor * (local_b[i_idx] - sum) / local_matrix[(i_idx * n_idx) + global_idx]);

    double diff = local_x_new[i_idx] - x[global_idx];
    local_diff += diff * diff;
  }

  return local_diff;
}

int ComputeFinalResult(const std::vector<double> &x, int n) {
  double sum = 0.0;
  for (int i = 0; i < n; ++i) {
    sum += x[static_cast<size_t>(i)];
  }
  return static_cast<int>(std::round(sum));
}

}  // namespace

MakoveevaSSimpleIterationMPI::MakoveevaSSimpleIterationMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool MakoveevaSSimpleIterationMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int is_valid = 0;
  if (rank == 0) {
    is_valid = ((GetInput() > 0) && (GetOutput() == 0)) ? 1 : 0;
  }

  MPI_Bcast(&is_valid, 1, MPI_INT, 0, MPI_COMM_WORLD);
  return is_valid != 0;
}

bool MakoveevaSSimpleIterationMPI::PreProcessingImpl() {
  return true;
}

bool MakoveevaSSimpleIterationMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const int n = GetInput();
  if (n <= 0) {
    return false;
  }

  std::vector<int> row_counts(static_cast<size_t>(size));
  std::vector<int> row_displs(static_cast<size_t>(size));
  std::vector<int> matrix_counts(static_cast<size_t>(size));
  std::vector<int> matrix_displs(static_cast<size_t>(size));

  CalculateRowsDistribution(n, size, row_counts, row_displs, matrix_counts, matrix_displs);

  int local_rows = row_counts[rank];
  int start_row = row_displs[rank];

  std::vector<double> flat_matrix;
  std::vector<double> b;

  if (rank == 0) {
    InitializeMatrixAndVector(flat_matrix, b, n);
  }

  std::vector<double> local_matrix(static_cast<size_t>(local_rows) * static_cast<size_t>(n));
  MPI_Scatterv(flat_matrix.data(), matrix_counts.data(), matrix_displs.data(), MPI_DOUBLE, local_matrix.data(),
               local_rows * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  std::vector<double> local_b(static_cast<size_t>(local_rows));
  MPI_Scatterv(b.data(), row_counts.data(), row_displs.data(), MPI_DOUBLE, local_b.data(), local_rows, MPI_DOUBLE, 0,
               MPI_COMM_WORLD);
  std::vector<double> x(static_cast<size_t>(n), 0.0);
  std::vector<double> x_new(static_cast<size_t>(n), 0.0);
  std::vector<double> local_x_new(static_cast<size_t>(local_rows), 0.0);

  bool converged = false;

  for (int iteration = 0; iteration < kMaxIterations && !converged; ++iteration) {
    double local_diff = ComputeLocalProduct(local_matrix, x, local_b, local_x_new, local_rows, start_row, n);

    if (rank == 0) {
      for (int i = 0; i < local_rows; ++i) {
        const auto idx = static_cast<size_t>(start_row + i);
        x_new[idx] = local_x_new[static_cast<size_t>(i)];
      }

      for (int proc = 1; proc < size; ++proc) {
        int proc_rows = row_counts[proc];
        int proc_start = row_displs[proc];
        MPI_Recv(x_new.data() + proc_start, proc_rows, MPI_DOUBLE, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    } else {
      MPI_Send(local_x_new.data(), local_rows, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Bcast(x_new.data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double global_diff = 0.0;
    MPI_Reduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    int converged_flag = 0;
    if (rank == 0) {
      global_diff = std::sqrt(global_diff);
      converged_flag = (global_diff < kEpsilon) ? 1 : 0;
    }

    MPI_Bcast(&converged_flag, 1, MPI_INT, 0, MPI_COMM_WORLD);

    x = x_new;
    converged = (converged_flag != 0);
  }

  if (rank == 0) {
    GetOutput() = ComputeFinalResult(x, n);
  }

  MPI_Bcast(&GetOutput(), 1, MPI_INT, 0, MPI_COMM_WORLD);

  return converged;
}

bool MakoveevaSSimpleIterationMPI::PostProcessingImpl() {
  return GetOutput() > 0;
}

}  // namespace makoveeva_s_simple_iteration
