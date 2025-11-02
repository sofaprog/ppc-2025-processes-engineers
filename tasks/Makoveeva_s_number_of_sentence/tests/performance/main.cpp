#include <gtest/gtest.h>
#include <string>

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"
#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"

namespace Makoveeva_s_number_of_sentence {

// Performance тест для полного пайплайна
TEST(makoveeva_s_number_of_sentence_seq, test_pipeline_run) {
    std::string test_text = "This is a test sentence. And another one! How about a question? ";
    test_text = test_text + test_text + test_text;
    
    auto task = SentencesCounterSEQ(test_text);
    
    // Используем публичные методы
    for (int i = 0; i < 100; i++) {
        EXPECT_TRUE(task.validation());
        EXPECT_TRUE(task.pre_processing());
        EXPECT_TRUE(task.run());
        EXPECT_TRUE(task.post_processing());
    }
}

// Performance тест только для выполнения задачи
TEST(makoveeva_s_number_of_sentence_seq, test_task_run) {
    std::string test_text = "Short. Text! For? Performance. Testing! ";
    test_text = test_text + test_text;
    
    auto task = SentencesCounterSEQ(test_text);
    task.pre_processing(); // Один раз инициализируем
    
    // Только run()
    for (int i = 0; i < 500; i++) {
        EXPECT_TRUE(task.run());
    }
}

// MPI performance тесты
TEST(makoveeva_s_number_of_sentence_mpi, test_pipeline_run) {
    std::string test_text = "MPI test sentence. Performance! Testing? ";
    
    auto task = SentencesCounterMPI(test_text);
    
    for (int i = 0; i < 50; i++) {
        EXPECT_TRUE(task.validation());
        EXPECT_TRUE(task.pre_processing());
        EXPECT_TRUE(task.run());
        EXPECT_TRUE(task.post_processing());
    }
}

TEST(makoveeva_s_number_of_sentence_mpi, test_task_run) {
    std::string test_text = "MPI task. Performance! Test? ";
    
    auto task = SentencesCounterMPI(test_text);
    task.pre_processing();
    
    for (int i = 0; i < 200; i++) {
        EXPECT_TRUE(task.run());
    }
}

} // namespace Makoveeva_s_number_of_sentence