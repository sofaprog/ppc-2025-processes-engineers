#include <gtest/gtest.h>

#include <string>

#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

TEST(MakoveevaSNumberOfSentence, SeqPerformancePipeline) {
  std::string test_text = "This is a test sentence. And another one! How about a question? ";

  for (int i = 0; i < 10; i++) {
    test_text += test_text;
  }

  for (int i = 0; i < 10; i++) {
    auto task = SentencesCounterSEQ(test_text);
    EXPECT_TRUE(task.Validation());
    EXPECT_TRUE(task.PreProcessing());
    EXPECT_TRUE(task.Run());
    EXPECT_TRUE(task.PostProcessing());
  }
}

TEST(MakoveevaSNumberOfSentence, SeqPerformanceTask) {
  std::string test_text = "Short text. For performance!";

  for (int i = 0; i < 5; i++) {
    test_text += test_text;
  }

  auto task = SentencesCounterSEQ(test_text);
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());

  for (int i = 0; i < 100; i++) {
    EXPECT_TRUE(task.Run());
  }

  EXPECT_TRUE(task.PostProcessing());
}

}  // namespace makoveeva_s_number_of_sentence
