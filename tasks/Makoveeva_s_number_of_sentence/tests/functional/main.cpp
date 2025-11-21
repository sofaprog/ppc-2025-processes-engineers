#include <gtest/gtest.h>

#include <string>

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

void TestSentencesCounterSEQ(const std::string &text, int expected_count);
void TestSentencesCounterMPI(const std::string &text, int expected_count);

void TestSentencesCounterSEQ(const std::string &text, int expected_count) {
  auto task = SentencesCounterSEQ(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected_count);
}

void TestSentencesCounterMPI(const std::string &text, int expected_count) {
  auto task = SentencesCounterMPI(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected_count);
}

TEST(makoveeva_s_number_of_sentence, seq_empty_text) {
  TestSentencesCounterSEQ("", 0);
}

TEST(makoveeva_s_number_of_sentence, seq_single_sentence) {
  TestSentencesCounterSEQ("Hello world.", 1);
}

TEST(makoveeva_s_number_of_sentence, seq_multiple_sentences) {
  TestSentencesCounterSEQ("First! Second? Third.", 3);
}

TEST(makoveeva_s_number_of_sentence, seq_no_sentences) {
  TestSentencesCounterSEQ("Just text without sentence endings", 0);
}

TEST(makoveeva_s_number_of_sentence, seq_mixed_endings) {
  TestSentencesCounterSEQ("Test one. Test two! Test three?", 3);
}

TEST(makoveeva_s_number_of_sentence, mpi_basic_test) {
  TestSentencesCounterMPI("Test sentence. Another one!", 2);
}

}  // namespace makoveeva_s_number_of_sentence
