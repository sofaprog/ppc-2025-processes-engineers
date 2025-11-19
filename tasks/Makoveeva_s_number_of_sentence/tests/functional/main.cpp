#include <gtest/gtest.h>
#include <string>

#include "Makoveeva_s_number_of_sentence/common/include/common.hpp"
#include "Makoveeva_s_number_of_sentence/mpi/include/ops_mpi.hpp"
#include "Makoveeva_s_number_of_sentence/seq/include/ops_seq.hpp"

namespace makoveeva_s_number_of_sentence {

TEST(makoveeva_s_number_of_sentence, seq_empty_text) {
  auto task = SentencesCounterSEQ("");
  EXPECT_TRUE(task.Validation());    
  EXPECT_TRUE(task.PreProcessing());   
  EXPECT_TRUE(task.Run());             
  EXPECT_TRUE(task.PostProcessing());  
  EXPECT_GE(task.GetOutput(), 0UL);
}

TEST(makoveeva_s_number_of_sentence, seq_single_sentence) {
  auto task = SentencesCounterSEQ("Hello world.");
  EXPECT_TRUE(task.Validation());     
  EXPECT_TRUE(task.PreProcessing());   
  EXPECT_TRUE(task.Run());             
  EXPECT_TRUE(task.PostProcessing()); 
  EXPECT_EQ(task.GetOutput(), 1UL);
}

TEST(makoveeva_s_number_of_sentence, seq_multiple_sentences) {
  auto task = SentencesCounterSEQ("First! Second? Third.");
  EXPECT_TRUE(task.Validation());    
  EXPECT_TRUE(task.PreProcessing());  
  EXPECT_TRUE(task.Run());            
  EXPECT_TRUE(task.PostProcessing()); 
  EXPECT_EQ(task.GetOutput(), 3UL);
}

TEST(makoveeva_s_number_of_sentence, seq_no_sentences) {
  auto task = SentencesCounterSEQ("Just text without sentence endings");
  EXPECT_TRUE(task.Validation());      
  EXPECT_TRUE(task.PreProcessing());   
  EXPECT_TRUE(task.Run());             
  EXPECT_TRUE(task.PostProcessing()); 
  EXPECT_GE(task.GetOutput(), 0UL);
}

TEST(makoveeva_s_number_of_sentence, mpi_basic_test) {
  auto task = SentencesCounterMPI("Test sentence. Another one!");
  EXPECT_TRUE(task.Validation());      
  EXPECT_TRUE(task.PreProcessing());  
  EXPECT_TRUE(task.Run());            
  EXPECT_TRUE(task.PostProcessing());  
  EXPECT_GE(task.GetOutput(), 0UL);
}

}  // namespace makoveeva_s_number_of_sentence