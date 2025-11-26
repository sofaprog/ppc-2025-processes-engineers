#include <gtest/gtest.h>

#include <array>
#include <cctype>
#include <cstddef>
#include <functional>
#include <string>
#include <tuple>

#include "makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"

namespace makoveeva_s_number_of_sentence {

class MakoveevaSNumberOfSentenceRunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    std::string input = std::get<0>(test_param);
    std::string expected = std::get<1>(test_param);

    std::string sanitized;
    sanitized.reserve(input.length() + expected.length() + 20);

    for (char ch : input) {
      if (std::isalnum(static_cast<unsigned char>(ch)) != 0) {
        sanitized += ch;
      } else {
        sanitized += '_';
      }
    }

    std::size_t input_hash = std::hash<std::string>{}(input);
    sanitized += "_count_";
    sanitized += expected;
    sanitized += "_";
    sanitized += std::to_string(input_hash % 10000);

    return sanitized;
  }

 protected:
  void SetUp() override {
    auto params = std::get<2>(GetParam());
    input_data_ = std::get<0>(params);
    expected_output_ = std::stoi(std::get<1>(params));
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_output_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_ = 0;
};

namespace {

TEST_P(MakoveevaSNumberOfSentenceRunFuncTestsProcesses, CountSentences) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 24> kTestParam = {
    // Базовые случаи
    std::make_tuple("Simple case.", "1"), std::make_tuple("First! Second?", "2"),
    std::make_tuple("Test one. Test two! Test three?", "3"), std::make_tuple("", "0"),
    std::make_tuple("Text without delimiters", "0"), std::make_tuple("Alpha. Beta. Gamma.", "3"),

    // Специальные случаи с последовательными знаками
    std::make_tuple("Ellipsis... Exclamation!!", "2"), std::make_tuple("Sentence. Another! Different? Final.", "4"),
    std::make_tuple("Just ellipsis...", "1"), std::make_tuple("Wow! Amazing!", "2"),
    std::make_tuple("What? Yes! Indeed.", "3"), std::make_tuple("Incredible!!! Fantastic!!!", "2"),

    // Граничные случаи и краевые условия
    std::make_tuple("X.Y.Z.W.V.U.T.S.R.Q.", "10"),
    std::make_tuple("Very long text without any sentence endings whatsoever", "0"),
    std::make_tuple("Begin. Continue! Finish?", "3"), std::make_tuple("Z.", "1"), std::make_tuple("?!.", "1"),
    std::make_tuple(".?!", "1"), std::make_tuple("Word . Another !", "2"), std::make_tuple("M.N.O.P", "3"),
    std::make_tuple("test..example.", "2"), std::make_tuple("text...sample.", "2"),
    std::make_tuple("data!!!result.", "2"), std::make_tuple("input.!?output.", "2")};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<SentencesCounterMPI, InType>(kTestParam, "makoveeva_s_number_of_sentence"),
                   ppc::util::AddFuncTask<SentencesCounterSEQ, InType>(kTestParam, "makoveeva_s_number_of_sentence"));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    MakoveevaSNumberOfSentenceRunFuncTestsProcesses::PrintFuncTestName<MakoveevaSNumberOfSentenceRunFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(SentenceCountingTests, MakoveevaSNumberOfSentenceRunFuncTestsProcesses, kGtestValues,
                         kPerfTestName);

}  // namespace

}  // namespace makoveeva_s_number_of_sentence
