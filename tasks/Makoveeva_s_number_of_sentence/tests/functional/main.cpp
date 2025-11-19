#include <gtest/gtest.h>

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

// Вспомогательная функция для проверки SEQ
void TestSentencesCounterSEQ(const std::string &text, std::size_t expected_count) {
  auto task = SentencesCounterSEQ(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected_count);
}

// Вспомогательная функция для проверки MPI
void TestSentencesCounterMPI(const std::string &text, std::size_t expected_count) {
  auto task = SentencesCounterMPI(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected_count);
}

TEST(makoveeva_s_number_of_sentence, seq_empty_text) {
  TestSentencesCounterSEQ("", 0UL);
}

TEST(makoveeva_s_number_of_sentence, seq_single_sentence) {
  TestSentencesCounterSEQ("Hello world.", 1UL);
}

TEST(makoveeva_s_number_of_sentence, seq_multiple_sentences) {
  TestSentencesCounterSEQ("First! Second? Third.", 3UL);
}

TEST(makoveeva_s_number_of_sentence, seq_no_sentences) {
  TestSentencesCounterSEQ("Just text without sentence endings", 0UL);
}

TEST(makoveeva_s_number_of_sentence, seq_mixed_endings) {
  TestSentencesCounterSEQ("Test one. Test two! Test three?", 3UL);
}

TEST(makoveeva_s_number_of_sentence, mpi_basic_test) {
  TestSentencesCounterMPI("Test sentence. Another one!", 2UL);
}

}  // namespace makoveeva_s_number_of_sentence
