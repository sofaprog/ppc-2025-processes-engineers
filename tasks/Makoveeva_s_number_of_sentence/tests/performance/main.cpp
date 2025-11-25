#include <gtest/gtest.h>

#include <array>
#include <string>

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace makoveeva_s_number_of_sentence {

class MakoveevaSNumberOfSentencePerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  OutType expected_output_ = 0;

  void SetUp() override {
    const int paragraphs = 1000;
    const int sentences_per_paragraph = 50;
    const int words_per_sentence = 8;

    input_data_.clear();
    expected_output_ = 0;

    for (int p = 0; p < paragraphs; ++p) {
      for (int s = 0; s < sentences_per_paragraph; ++s) {
        input_data_ += static_cast<char>('A' + ((p + s) % 26));

        for (int w = 0; w < words_per_sentence; ++w) {
          int base_char = (p * 100 + s * 10 + w) % 26;
          int word_len = 3 + ((p + s + w) % 6);

          for (int c = 0; c < word_len; ++c) {
            input_data_ += static_cast<char>('a' + ((base_char + c) % 26));
          }

          if (w < words_per_sentence - 1) {
            input_data_ += ' ';
          }
        }

        constexpr std::array<char, 3> kPunctuation = {'.', '!', '?'};
        char punctuation = kPunctuation[(p + s) % 3];
        input_data_ += punctuation;
        expected_output_++;

        if (s < sentences_per_paragraph - 1) {
          input_data_ += ' ';
        }
      }

      if (p < paragraphs - 1) {
        input_data_ += "\n\n";
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return expected_output_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(MakoveevaSNumberOfSentencePerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, SentencesCounterMPI, SentencesCounterSEQ>(
    PPC_SETTINGS_makoveeva_s_number_of_sentence);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = MakoveevaSNumberOfSentencePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, MakoveevaSNumberOfSentencePerfTests, kGtestValues, kPerfTestName);

}  // namespace makoveeva_s_number_of_sentence
