#pragma once

// ВАЖНО: Сначала включаем common.hpp, а потом task.hpp
#include "makoveeva_s_simple_iteration/common/include/common.hpp"
// Убери лишний include task.hpp, если он уже есть в common.hpp
// #include "task/include/task.hpp"  // УБРАТЬ, если BaseTask уже определён в common.hpp

namespace makoveeva_s_simple_iteration {

class MakoveevaSSimpleIterationSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  
  explicit MakoveevaSSimpleIterationSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace makoveeva_s_simple_iteration