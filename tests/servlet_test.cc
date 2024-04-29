#include "gtest/gtest.h"
#include "servlet.h"

#include <string>
#include <tuple>

class ServletTest : public testing::Test {
    protected:
      void SetUp(std::string match_type, std::string match_path, std::string servlet_behavior, std::string servlet_root) {
        servlet = new Servlet(match_type, match_path, servlet_behavior, servlet_root);
      }
      void TearDown(){
        delete servlet;
      }
      void match_success(std::string path, int match_priority, int match_length){
        std::tuple<int, int> match_result = servlet->match_path(path);
        EXPECT_EQ(std::get<0>(match_result), match_priority);
        EXPECT_EQ(std::get<1>(match_result), match_length);
      }
      void match_failure(std::string path){
        EXPECT_EQ(std::get<1>(servlet->match_path(path)), 0);
      }
    
    Servlet* servlet;
};

// exact-matching path should return proper priority and full path length
TEST_F(ServletTest, ExactMatchSuccess) {
  SetUp(EXACT_MATCH, "/echo", ECHO, "");
  match_success("/echo", EXACT_MATCH_PRIORITY, 5);
}

// exact matching path when no match found
TEST_F(ServletTest, ExactMatchFailure) {
  SetUp(EXACT_MATCH, "/echo", ECHO, "");
  match_failure("/echo1");
}

// prefix match tests
TEST_F(ServletTest, PrefixMatchSuccess) {
  SetUp(PREFERRED_PREFIX_MATCH, "/static/", CONTENT, "/etc/files");
  match_success("/static/img.png", PREFERRED_PREFIX_MATCH_PRIORITY, 8);
  TearDown();
  SetUp(STANDARD_PREFIX_MATCH, "/content/", CONTENT, "/etc/files");
  match_success("/content/audio.mp3", STANDARD_PREFIX_MATCH_PRIORITY, 9);
}

TEST_F(ServletTest, PrefixMatchFailure) {
  SetUp(STANDARD_PREFIX_MATCH, "/static/", CONTENT, "/etc/files/");
  match_failure("/static2/doc.pdf");
}

// regex match tests
TEST_F(ServletTest, RegexCaseMatching) {
  SetUp(REGEX_CASE_INSENSITIVE_MATCH, "\\.(png|pdf|jpeg|mp3)", CONTENT, "/etc/files");
  match_success("/static/img.PNG", REGEX_MATCH_PRIORITY, 1);
  match_success("/static/audio.Mp3", REGEX_MATCH_PRIORITY, 1);
  TearDown();
  SetUp(REGEX_CASE_SENSITIVE_MATCH, "\\.(png|pdf|jpeg|mp3)", CONTENT, "/etc/files");
  match_failure("/static/img.PNG");
}

TEST_F(ServletTest, RegexNoCaseMatching) {
  SetUp(REGEX_CASE_INSENSITIVE_MATCH, "^/static[0-9]/", CONTENT, "/etc/files");
  match_success("/static2/img.png", REGEX_MATCH_PRIORITY, 1);
  match_failure("/static/img.png");
  TearDown();
  SetUp(REGEX_CASE_SENSITIVE_MATCH, "^/static[0-9]/", CONTENT, "/etc/files");
  match_success("/static2/img.png", REGEX_MATCH_PRIORITY, 1);
  match_failure("/Static2/img.png");
}
