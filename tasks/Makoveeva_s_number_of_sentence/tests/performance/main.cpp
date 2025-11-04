#include <gtest/gtest.h>
#include <string>

#include "makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"
#include "makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

namespace makoveeva_s_number_of_sentence {

// Простые performance тесты
TEST(makoveeva_s_number_of_sentence, seq_performance_pipeline) {
    std::string test_text = "This is a test sentence. And another one! How about a question? ";
    // Увеличим текст для performance теста
    for (int i = 0; i < 10; i++) {
        test_text += test_text;
    }
    
    for (int i = 0; i < 10; i++) {  // Уменьшим количество итераций для скорости
        auto task = SentencesCounterSEQ(test_text);
        EXPECT_TRUE(task.validation());
        EXPECT_TRUE(task.pre_processing());
        EXPECT_TRUE(task.run());
        EXPECT_TRUE(task.post_processing());
    }
}

TEST(makoveeva_s_number_of_sentence, seq_performance_task) {
    std::string test_text = "Short text. For performance!";
    // Увеличим текст
    for (int i = 0; i < 5; i++) {
        test_text += test_text;
    }
    
    auto task = SentencesCounterSEQ(test_text);
    EXPECT_TRUE(task.validation());
    EXPECT_TRUE(task.pre_processing());
    
    for (int i = 0; i < 100; i++) {  // Только run()
        EXPECT_TRUE(task.run());
    }
    
    EXPECT_TRUE(task.post_processing());
}

} // namespace makoveeva_s_number_of_sentence