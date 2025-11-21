#include <gtest/gtest.h>
#include <mpi.h>

#include <string>

#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

class MPITest : public ::testing::Test {
 protected:
  void SetUp() override {
    MPI_Init(nullptr, nullptr);
  }

  void TearDown() override {
    MPI_Finalize();
  }
};

static void TestSentencesCounterSEQ(const std::string &text, int expected_count);
static void TestSentencesCounterMPI(const std::string &text, int expected_count);

static void TestSentencesCounterSEQ(const std::string &text, int expected_count) {
  auto task = SentencesCounterSEQ(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  EXPECT_EQ(task.GetOutput(), expected_count);
}

static void TestSentencesCounterMPI(const std::string &text, int expected_count) {
  auto task = SentencesCounterMPI(text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());

  int rank=0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    EXPECT_EQ(task.GetOutput(), expected_count);
  }
}

TEST(MakoveevaSNumberOfSentence, SeqEmptyText) {
  TestSentencesCounterSEQ("", 0);
}

TEST(MakoveevaSNumberOfSentence, SeqSingleSentence) {
  TestSentencesCounterSEQ("Hello world.", 1);
}

TEST(MakoveevaSNumberOfSentence, SeqMultipleSentences) {
  TestSentencesCounterSEQ("First! Second? Third.", 3);
}

TEST(MakoveevaSNumberOfSentence, SeqNoSentences) {
  TestSentencesCounterSEQ("Just text without sentence endings", 0);
}

TEST(MakoveevaSNumberOfSentence, SeqMixedEndings) {
  TestSentencesCounterSEQ("Test one. Test two! Test three?", 3);
}

TEST_F(MPITest, MpiBasicTest) {
  TestSentencesCounterMPI("Test sentence. Another one!", 2);
}

}  // namespace makoveeva_s_number_of_sentence
