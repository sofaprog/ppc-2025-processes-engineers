#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace Makoveeva_s_number_of_sentence {

using InType = std::string;  // Входной тип - строка с текстом
using OutType = std::size_t; // Выходной тип - количество предложений
using TestType = std::tuple<std::string, std::string>; // Тестовые параметры
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace Makoveeva_s_number_of_sentence