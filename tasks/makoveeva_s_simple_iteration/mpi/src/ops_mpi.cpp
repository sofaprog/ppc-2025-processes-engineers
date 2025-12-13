#include "makoveeva_s_simple_iteration/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <iostream>
#include <vector>

#include "makoveeva_s_simple_iteration/common/include/common.hpp"
#include "util/include/util.hpp"

namespace makoveeva_s_simple_iteration {

MakoveevaSSimpleIterationMPI::MakoveevaSSimpleIterationMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;  // int, а не vector!
}

bool MakoveevaSSimpleIterationMPI::ValidationImpl() {
  int rank;
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
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = GetInput();  // Размер системы

  if (n <= 0) {
    return false;
  }

  // Главный процесс создаёт тестовую систему
  std::vector<double> A_flat;
  std::vector<double> b;

  if (rank == 0) {
    // Создаём матрицу A и вектор b
    A_flat.resize(n * n, 0.0);
    b.resize(n, 0.0);

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        if (i == j) {
          A_flat[i * n + j] = n + 5.0;
        } else {
          A_flat[i * n + j] = 1.0 / (std::abs(i - j) + 1.0);
        }
      }

      // Генерируем правую часть
      for (int j = 0; j < n; j++) {
        b[i] += A_flat[i * n + j] * (j + 1.0);
      }
    }
  }

  // Рассылаем размер системы
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Выделяем память в остальных процессах
  if (rank != 0) {
    A_flat.resize(n * n);
    b.resize(n);
  }

  // Рассылаем данные
  MPI_Bcast(A_flat.data(), n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(b.data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Параметры метода
  const double w = 0.5;
  const double eps = 1e-6;
  const int max_iter = 1000;

  // Распределяем строки по процессам
  int rows_per_proc = n / size;
  int extra_rows = n % size;

  int start_row = rank * rows_per_proc + std::min(rank, extra_rows);
  int end_row = start_row + rows_per_proc + (rank < extra_rows ? 1 : 0);
  int my_rows = end_row - start_row;

  // Начальное приближение
  std::vector<double> x(n, 0.0);
  std::vector<double> x_new(n, 0.0);
  std::vector<double> my_x_new(my_rows, 0.0);

  int iter = 0;
  bool converged = false;

  while (iter < max_iter && !converged) {
    // Каждый процесс вычисляет свои строки
    for (int local_i = 0; local_i < my_rows; local_i++) {
      int global_i = start_row + local_i;

      double sum = 0.0;
      for (int j = 0; j < n; j++) {
        sum += A_flat[global_i * n + j] * x[j];
      }

      // Формула метода простой итерации
      my_x_new[local_i] = x[global_i] + w * (b[global_i] - sum) / A_flat[global_i * n + global_i];
    }

    // Собираем результаты
    std::vector<int> recv_counts(size);
    std::vector<int> displacements(size);

    for (int i = 0; i < size; i++) {
      int i_rows = n / size + (i < extra_rows ? 1 : 0);
      recv_counts[i] = i_rows;
      displacements[i] = (i == 0) ? 0 : displacements[i - 1] + recv_counts[i - 1];
    }

    MPI_Allgatherv(my_x_new.data(), my_rows, MPI_DOUBLE, x_new.data(), recv_counts.data(), displacements.data(),
                   MPI_DOUBLE, MPI_COMM_WORLD);

    // Вычисляем ошибку
    double local_error = 0.0;
    for (int local_i = 0; local_i < my_rows; local_i++) {
      int global_i = start_row + local_i;
      double diff = x_new[global_i] - x[global_i];
      local_error += diff * diff;
    }

    double global_error = 0.0;
    MPI_Allreduce(&local_error, &global_error, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    global_error = std::sqrt(global_error);

    if (global_error < eps) {
      converged = true;
    }

    x = x_new;
    iter++;
  }

  // Главный процесс вычисляет и сохраняет результат
  if (rank == 0) {
    // Вычисляем сумму компонент решения
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
      sum += x[i];
    }

    GetOutput() = static_cast<int>(std::round(sum));

    std::cout << "MPI (n=" << n << ", processes=" << size << "): " << (converged ? "Converged" : "Not converged")
              << " in " << iter << " iterations" << std::endl;
  }

  // Рассылаем результат всем процессам
  int result = 0;
  if (rank == 0) {
    result = GetOutput();
  }
  MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    GetOutput() = result;
  }

  MPI_Barrier(MPI_COMM_WORLD);

  return converged;
}

bool MakoveevaSSimpleIterationMPI::PostProcessingImpl() {
  return GetOutput() > 0;
}

}  // namespace makoveeva_s_simple_iteration
