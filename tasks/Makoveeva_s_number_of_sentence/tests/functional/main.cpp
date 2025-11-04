#include <gtest/gtest.h>
#include <string>

#include "makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"
#include "makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

namespace makoveeva_s_number_of_sentence {

// Функциональные тесты для sequential версии
TEST(makoveeva_s_number_of_sentence, seq_empty_text) {
    auto task = SentencesCounterSEQ("");
    // Используем публичные методы вместо Impl
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

// Базовый тест для MPI
TEST(makoveeva_s_number_of_sentence, mpi_basic_test) {
    auto task = SentencesCounterMPI("Test sentence.");
    EXPECT_TRUE(task.validation());
    EXPECT_TRUE(task.pre_processing());
    EXPECT_TRUE(task.run());
    EXPECT_TRUE(task.post_processing());
    EXPECT_TRUE(task.GetOutput() >= 0);
}

} // namespace makoveeva_s_number_of_sentence