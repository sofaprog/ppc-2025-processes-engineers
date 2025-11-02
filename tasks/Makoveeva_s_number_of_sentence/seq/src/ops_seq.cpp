#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

#include <string>

namespace Makoveeva_s_number_of_sentence {

SentencesCounterSEQ::SentencesCounterSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SentencesCounterSEQ::ValidationImpl() {
  // Проверяем что входная строка не пустая и выход = 0
  return (!GetInput().empty()) && (GetOutput() == 0);
}

bool SentencesCounterSEQ::PreProcessingImpl() {
  // Инициализация - можно пропустить или подготовить данные
  return true;
}

bool SentencesCounterSEQ::RunImpl() {
  // ОСНОВНОЙ АЛГОРИТМ ПОДСЧЕТА ПРЕДЛОЖЕНИЙ
  const std::string& text = GetInput();
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
  // Пост-обработка не нужна для этой задачи
  return true;
}

}  // namespace Makoveeva_s_number_of_sentence