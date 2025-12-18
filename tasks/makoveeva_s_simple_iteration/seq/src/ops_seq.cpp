#include "makoveeva_s_simple_iteration/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "makoveeva_s_simple_iteration/common/include/common.hpp"

namespace makoveeva_s_simple_iteration {

namespace {
constexpr double kW = 0.5;
constexpr double kEps = 1e-6;
constexpr int kMaxIter = 1000;
}  // namespace

MakoveevaSSimpleIterationSEQ::MakoveevaSSimpleIterationSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool MakoveevaSSimpleIterationSEQ::ValidationImpl() {
  return (GetInput() > 0) && (GetOutput() == 0);
}

bool MakoveevaSSimpleIterationSEQ::PreProcessingImpl() {
  return true;
}

bool MakoveevaSSimpleIterationSEQ::RunImpl() {
  const int n = GetInput();

  const auto matrix_size = static_cast<size_t>(n) * static_cast<size_t>(n);
  std::vector<double> a(matrix_size, 0.0);
  std::vector<double> b(static_cast<size_t>(n), 0.0);

  for (int i = 0; i < n; ++i) {
    const auto i_idx = static_cast<size_t>(i);
    const auto n_idx = static_cast<size_t>(n);

    a[(i_idx * n_idx) + i_idx] = static_cast<double>(n) + 5.0;

    for (int j = 0; j < n; ++j) {
      if (i != j) {
        const auto j_idx = static_cast<size_t>(j);
        a[(i_idx * n_idx) + j_idx] = 1.0 / (static_cast<double>(std::abs(i - j)) + 1.0);
      }
    }

    for (int j = 0; j < n; ++j) {
      const auto j_idx = static_cast<size_t>(j);
      b[i_idx] += a[(i_idx * n_idx) + j_idx] * static_cast<double>(j + 1);
    }
  }

  std::vector<double> x(static_cast<size_t>(n), 0.0);
  std::vector<double> x_new(static_cast<size_t>(n), 0.0);

  int iter = 0;
  bool converged = false;

  while (iter < kMaxIter && !converged) {
    double error = 0.0;

    for (int i = 0; i < n; ++i) {
      const auto i_idx = static_cast<size_t>(i);
      const auto n_idx = static_cast<size_t>(n);

      double sum = 0.0;
      for (int j = 0; j < n; ++j) {
        const auto j_idx = static_cast<size_t>(j);
        sum += a[(i_idx * n_idx) + j_idx] * x[j_idx];
      }

      x_new[i_idx] = x[i_idx] + (kW * (b[i_idx] - sum) / a[(i_idx * n_idx) + i_idx]);

      const double diff = x_new[i_idx] - x[i_idx];
      error += diff * diff;
    }

    error = std::sqrt(error);
    if (error < kEps) {
      converged = true;
    }

    x.swap(x_new);
    ++iter;
  }

  double sum = 0.0;
  for (int i = 0; i < n; ++i) {
    sum += x[static_cast<size_t>(i)];
  }

  GetOutput() = static_cast<int>(std::round(sum));

  return converged;
}

bool MakoveevaSSimpleIterationSEQ::PostProcessingImpl() {
  return GetOutput() > 0;
}

}  // namespace makoveeva_s_simple_iteration
