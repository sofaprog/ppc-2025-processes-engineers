#include "makoveeva_s_simple_iteration/seq/include/ops_seq.hpp"

#include <cmath>
#include <iostream>
#include <vector>

#include "makoveeva_s_simple_iteration/common/include/common.hpp"
#include "util/include/util.hpp"

namespace makoveeva_s_simple_iteration {

MakoveevaSSimpleIterationSEQ::MakoveevaSSimpleIterationSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;  // int, а не vector!
}

bool MakoveevaSSimpleIterationSEQ::ValidationImpl() {
  return (GetInput() > 0) && (GetOutput() == 0);
}

bool MakoveevaSSimpleIterationSEQ::PreProcessingImpl() {
  return true;
}

bool MakoveevaSSimpleIterationSEQ::RunImpl() {
  int n = GetInput();  // Размер системы
  
  // 1. СОЗДАЁМ РЕАЛЬНУЮ СИСТЕМУ
  std::vector<std::vector<double>> A(n, std::vector<double>(n, 0.0));
  std::vector<double> b(n, 0.0);
  
  // Заполняем диагонально доминантную матрицу
  for (int i = 0; i < n; i++) {
    A[i][i] = n + 5.0;  // Большой диагональный элемент
    
    for (int j = 0; j < n; j++) {
      if (i != j) {
        A[i][j] = 1.0 / (std::abs(i - j) + 1.0);
      }
    }
    
    // Генерируем правую часть для нетривиального решения
    for (int j = 0; j < n; j++) {
      b[i] += A[i][j] * (j + 1.0);  // Решение будет x_j = j+1
    }
  }
  
  // 2. МЕТОД ПРОСТОЙ ИТЕРАЦИИ
  std::vector<double> x(n, 0.0);      // Начальное приближение
  std::vector<double> x_new(n, 0.0);
  
  const double w = 0.5;      // Параметр релаксации
  const double eps = 1e-6;
  const int max_iter = 1000;
  
  int iter = 0;
  bool converged = false;
  
  while (iter < max_iter && !converged) {
    double error = 0.0;
    
    for (int i = 0; i < n; i++) {
      // Вычисляем A*x для строки i
      double sum = 0.0;
      for (int j = 0; j < n; j++) {
        sum += A[i][j] * x[j];
      }
      
      // Формула метода простой итерации
      x_new[i] = x[i] + w * (b[i] - sum) / A[i][i];
      
      error += (x_new[i] - x[i]) * (x_new[i] - x[i]);
    }
    
    error = std::sqrt(error);
    if (error < eps) {
      converged = true;
    }
    
    x = x_new;
    iter++;
  }
  
  // 3. ВЫЧИСЛЯЕМ РЕЗУЛЬТАТ (сумма компонент)
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += x[i];
  }
  
  GetOutput() = static_cast<int>(std::round(sum));
  
  std::cout << "SEQ (n=" << n << "): " 
            << (converged ? "Converged" : "Not converged")
            << " in " << iter << " iterations" << std::endl;
  
  return converged;
}

bool MakoveevaSSimpleIterationSEQ::PostProcessingImpl() {
  return GetOutput() > 0;
}

}  // namespace makoveeva_s_simple_iteration