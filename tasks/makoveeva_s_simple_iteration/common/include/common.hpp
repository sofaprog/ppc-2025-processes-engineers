#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace makoveeva_s_simple_iteration {

using InType = int;   // Размер системы n (как у одногруппника)
using OutType = int;  // Результат - сумма компонент решения
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace makoveeva_s_simple_iteration
