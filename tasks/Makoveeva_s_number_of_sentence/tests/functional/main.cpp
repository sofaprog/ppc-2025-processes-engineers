#include <gtest/gtest.h>

#include <string>

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

TEST(makoveeva_s_number_of_sentence, seq_empty_text) {
  auto task = SentencesCounterSEQ("");
  EXPECT_TRUE(task.validation());
  EXPECT_TRUE(task.pre_processing());
  EXPECT_TRUE(task.run());
  EXPECT_TRUE(task.post_processing());
  EXPECT_EQ(task.GetOutput(), 0);
}

TEST(makoveeva_s_number_of_sentence, seq_single_sentence) {
  auto task = SentencesCounterSEQ("Hello world.");
  EXPECT_TRUE(task.validation());
  EXPECT_TRUE(task.pre_processing());
  EXPECT_TRUE(task.run());
  EXPECT_TRUE(task.post_processing());
  EXPECT_EQ(task.GetOutput(), 1);
}

TEST(makoveeva_s_number_of_sentence, seq_multiple_sentences) {
  auto task = SentencesCounterSEQ("First! Second? Third.");
  EXPECT_TRUE(task.validation());
  EXPECT_TRUE(task.pre_processing());
  EXPECT_TRUE(task.run());
  EXPECT_TRUE(task.post_processing());
  EXPECT_EQ(task.GetOutput(), 3);
}

TEST(makoveeva_s_number_of_sentence, seq_no_sentences) {
  auto task = SentencesCounterSEQ("Just text without sentence endings");
  EXPECT_TRUE(task.validation());
  EXPECT_TRUE(task.pre_processing());
  EXPECT_TRUE(task.run());
  EXPECT_TRUE(task.post_processing());
  EXPECT_EQ(task.GetOutput(), 0);
}

TEST(makoveeva_s_number_of_sentence, mpi_basic_test) {
  auto task = SentencesCounterMPI("Test sentence. Another one!");
  EXPECT_TRUE(task.validation());
  EXPECT_TRUE(task.pre_processing());
  EXPECT_TRUE(task.run());
  EXPECT_TRUE(task.post_processing());
  EXPECT_TRUE(task.GetOutput() >= 0);
}

}  // namespace makoveeva_s_number_of_sentence
