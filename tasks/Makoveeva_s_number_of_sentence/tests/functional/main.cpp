#include <gtest/gtest.h>
#include <mpi.h>

#include <string>
#include <tuple>

#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

class SentencesCounterSEQTest : public testing::TestWithParam<std::tuple<std::string, int>> {};

TEST_P(SentencesCounterSEQTest, CountsCorrectly_seq) {
  auto [text, expected] = GetParam();
  auto task = SentencesCounterSEQ(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected);
}

class SentencesCounterMPITest : public testing::TestWithParam<std::tuple<std::string, int>> {
 protected:
  static void SetUpTestSuite() {
    MPI_Init(nullptr, nullptr);
  }

  static void TearDownTestSuite() {
    MPI_Finalize();
  }
};

TEST_P(SentencesCounterMPITest, CountsCorrectly_mpi) {
  auto [text, expected] = GetParam();
  auto task = SentencesCounterMPI(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    EXPECT_EQ(task.GetOutput(), expected);
  }
}

INSTANTIATE_TEST_SUITE_P(SentenceTest_seq, SentencesCounterSEQTest,
                         testing::Values(std::make_tuple("", 0), std::make_tuple("Hello world.", 1),
                                         std::make_tuple("First! Second? Third.", 3),
                                         std::make_tuple("Just text without sentence endings", 0),
                                         std::make_tuple("Test one. Test two! Test three?", 3),
                                         std::make_tuple("A.B.C", 2), std::make_tuple("Wow! Amazing?", 2)));

INSTANTIATE_TEST_SUITE_P(SentenceTests_mpi, SentencesCounterMPITest,
                         testing::Values(std::make_tuple("", 0), std::make_tuple("Hello world.", 1),
                                         std::make_tuple("First! Second? Third.", 3),
                                         std::make_tuple("Test sentence. Another one!", 2)));

}  // namespace makoveeva_s_number_of_sentence
