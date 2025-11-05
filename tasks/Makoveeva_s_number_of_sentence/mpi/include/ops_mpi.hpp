#pragma once

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "task/include/task.hpp"

namespace makoveeva_s_number_of_sentence {

class SentencesCounterMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit SentencesCounterMPI(const InType &in);

  // ДОБАВИТЬ публичные методы для тестов
  bool validation() {
    return ValidationImpl();
  }
  bool pre_processing() {
    return PreProcessingImpl();
  }
  bool run() {
    return RunImpl();
  }
  bool post_processing() {
    return PostProcessingImpl();
  }

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace makoveeva_s_number_of_sentence
