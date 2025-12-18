#include "makoveeva_s_simple_iteration/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace makoveeva_s_simple_iteration {

namespace {
constexpr double kW = 0.5;
constexpr double kEps = 1e-6;
constexpr int kMaxIter = 1000;

void BuildSystem(int n, std::vector<double> *a, std::vector<double> *b) {
  const auto matrix_size = static_cast<size_t>(n) * static_cast<size_t>(n);
  a->assign(matrix_size, 0.0);
  b->assign(static_cast<size_t>(n), 0.0);

  for (int i = 0; i < n; ++i) {
    const auto i_idx = static_cast<size_t>(i);
    const auto n_idx = static_cast<size_t>(n);

    (*a)[(i_idx * n_idx) + i_idx] = static_cast<double>(n) + 5.0;

    for (int j = 0; j < n; ++j) {
      if (i != j) {
        const auto j_idx = static_cast<size_t>(j);
        (*a)[(i_idx * n_idx) + j_idx] = 1.0 / (static_cast<double>(std::abs(i - j)) + 1.0);
      }
    }

    for (int j = 0; j < n; ++j) {
      const auto j_idx = static_cast<size_t>(j);
      (*b)[i_idx] += (*a)[(i_idx * n_idx) + j_idx] * static_cast<double>(j + 1);
    }
  }
}

bool SolveByIteration(int n, const std::vector<double> &a, const std::vector<double> &b, std::vector<double> *x) {
  std::vector<double> x_new(static_cast<size_t>(n), 0.0);

  bool converged = false;
  for (int iter = 0; iter < kMaxIter && !converged; ++iter) {
    double error_sq = 0.0;

    for (int i = 0; i < n; ++i) {
      const auto i_idx = static_cast<size_t>(i);
      const auto n_idx = static_cast<size_t>(n);

      double sum = 0.0;
      for (int j = 0; j < n; ++j) {
        const auto j_idx = static_cast<size_t>(j);
        sum += a[(i_idx * n_idx) + j_idx] * (*x)[j_idx];
      }

      x_new[i_idx] = (*x)[i_idx] + (kW * (b[i_idx] - sum) / a[(i_idx * n_idx) + i_idx]);

      const double diff = x_new[i_idx] - (*x)[i_idx];
      error_sq += diff * diff;
    }

    converged = (std::sqrt(error_sq) < kEps);
    x->swap(x_new);
  }

  return converged;
}

int ComputeAnswer(const std::vector<double> &x) {
  double sum = 0.0;
  for (double v : x) {
    sum += v;
  }
  return static_cast<int>(std::round(sum));
}

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

  std::vector<double> a;
  std::vector<double> b;
  BuildSystem(n, &a, &b);

  std::vector<double> x(static_cast<size_t>(n), 0.0);
  const bool converged = SolveByIteration(n, a, b, &x);

  GetOutput() = ComputeAnswer(x);
  return converged;
}

bool MakoveevaSSimpleIterationSEQ::PostProcessingImpl() {
  return GetOutput() > 0;
}

}  // namespace makoveeva_s_simple_iteration
