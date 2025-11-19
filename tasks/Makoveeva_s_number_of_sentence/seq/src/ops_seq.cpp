#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

#include <cstddef>
#include <string>

#include "task/include/task.hpp"

namespace makoveeva_s_number_of_sentence {

ppc::task::TypeOfTask SentencesCounterSEQ::GetStaticTypeOfTask() {
  return ppc::task::TypeOfTask::kSEQ;
}

SentencesCounterSEQ::SentencesCounterSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SentencesCounterSEQ::ValidationImpl() {
  return (GetOutput() == 0);
}

bool SentencesCounterSEQ::PreProcessingImpl() {
  return true;
}

bool SentencesCounterSEQ::RunImpl() {
  const std::string &text = GetInput();
  std::size_t sentence_count = 0;

  for (char c : text) {
    if (c == '.' || c == '!' || c == '?') {
      sentence_count++;
    }
  }

  GetOutput() = sentence_count;
  return true;
}

bool SentencesCounterSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace makoveeva_s_number_of_sentence
