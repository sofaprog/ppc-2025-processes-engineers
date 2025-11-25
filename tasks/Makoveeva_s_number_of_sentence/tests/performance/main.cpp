#include <gtest/gtest.h>

#include <string>
#include <tuple>

#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

class SentencesCounterSEQPerfTest : public testing::TestWithParam<std::tuple<std::string, int>> {};

TEST_P(SentencesCounterSEQPerfTest, Pipeline_perf) {
  auto [text, expected] = GetParam();
  auto task = SentencesCounterSEQ(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected);
}

TEST_P(SentencesCounterSEQPerfTest, TaskRun_perf) {
  auto [text, expected] = GetParam();
  auto task = SentencesCounterSEQ(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());

  for (int i = 0; i < 10; i++) {
    EXPECT_TRUE(task.Run());
  }

  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected);
}

class SentencesCounterMPIPerfTest : public testing::TestWithParam<std::tuple<std::string, int>> {};

TEST_P(SentencesCounterMPIPerfTest, Pipeline_perf) {
  auto [text, expected] = GetParam();
  auto task = SentencesCounterMPI(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
}

TEST_P(SentencesCounterMPIPerfTest, TaskRun_perf) {
  auto [text, expected] = GetParam();
  auto task = SentencesCounterMPI(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());

  for (int i = 0; i < 10; i++) {
    EXPECT_TRUE(task.Run());
  }

  EXPECT_TRUE(task.PostProcessing());
}

INSTANTIATE_TEST_SUITE_P(PerformanceTests_seq, SentencesCounterSEQPerfTest,
                         testing::Values(std::make_tuple("Short text. For performance!", 2),
                                         std::make_tuple("This is a long text with many sentences. "
                                                         "It should be used for performance testing! "
                                                         "How many sentences can we count? "
                                                         "Let's find out. This is another sentence! "
                                                         "And one more? And another. And yet another! "
                                                         "The quick brown fox jumps over the lazy dog.",
                                                         9)));

INSTANTIATE_TEST_SUITE_P(PerformanceTests_mpi, SentencesCounterMPIPerfTest,
                         testing::Values(std::make_tuple("Short text. For performance!", 2),
                                         std::make_tuple("This is a long text with many sentences. "
                                                         "It should be used for performance testing! "
                                                         "How many sentences can we count? "
                                                         "Let's find out. This is another sentence!",
                                                         4)));

}  // namespace makoveeva_s_number_of_sentence
