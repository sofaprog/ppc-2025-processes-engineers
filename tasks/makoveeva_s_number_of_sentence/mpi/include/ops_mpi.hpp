#pragma once

#include <string>
#include <utility>

#include "makoveeva_s_number_of_sentence/common/include/common.hpp"

namespace makoveeva_s_number_of_sentence {

class SentencesCounterMPI : public ppc::task::Task<std::string, int> {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit SentencesCounterMPI(const std::string &input);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static bool IsSentenceDelimiter(char symbol);
  static std::pair<int, int> ComputeSegmentRange(int total_length, int process_total, int process_identifier);
};

}  // namespace makoveeva_s_number_of_sentence
