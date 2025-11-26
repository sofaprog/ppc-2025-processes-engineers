#include "makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

#include <string>

namespace makoveeva_s_number_of_sentence {

SentencesCounterSEQ::SentencesCounterSEQ(const std::string &input) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = input;
  GetOutput() = 0;
}

bool SentencesCounterSEQ::ValidationImpl() {
  return true;
}

bool SentencesCounterSEQ::PreProcessingImpl() {
  return true;
}

bool SentencesCounterSEQ::RunImpl() {
  const std::string &text = GetInput();
  int count = 0;

  for (size_t i = 0; i < text.length(); ++i) {
    char c = text[i];
    if (c == '.' || c == '!' || c == '?') {
      count++;
    }
  }

  GetOutput() = count;
  return true;
}

bool SentencesCounterSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace makoveeva_s_number_of_sentence
