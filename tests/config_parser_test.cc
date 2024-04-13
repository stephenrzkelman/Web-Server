#include "gtest/gtest.h"
#include "config_parser.h"
#include <string>

class NginxConfigParserTest : public testing::Test {
  protected:
    void SetUp() override{}
    void pass_test(const char* file_name, std::string expected_config_string) {
      EXPECT_TRUE(parser.Parse(file_name, &out_config));
      EXPECT_EQ(out_config.ToString(0).c_str(), expected_config_string);
    }
    void fail_test(const char* file_name) {
      EXPECT_FALSE(parser.Parse(file_name, &out_config));
    }

  NginxConfigParser parser;
  NginxConfig out_config;
};

// the basic given config should parse without failure
TEST_F(NginxConfigParserTest, SimpleConfig) {
  pass_test(
    "example_config",
    "foo \"bar\";\nserver {\n  listen 80;\n  server_name foo.com;\n  root /home/ubuntu/sites/foo/;\n}\n"
  );
}

// Parsing a config with an unclosed brace should fail
TEST_F(NginxConfigParserTest, OpenBraceConfig) {
  fail_test("open_brace_config");
}

// Parsing a config with a dangling close-brace should fail
TEST_F(NginxConfigParserTest, CloseBraceConfig) {
  fail_test("close_brace_config");
}

// Quoted strings should allow escape-characters
TEST_F(NginxConfigParserTest, QuotesConfig) {
  pass_test(
    "quotes_config",
    "quote1 \'\';\nquote2 \"\";\nescape1 \'\\\'\';\nescape2 \"\\\"\";\nend1 \'hello\' \'world\';\nend2 \"welcome\" \"home\";\n"
  );
}

// Single-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadSingleQuoteFollow) {
  fail_test("bad_single_quote_follow");
}

// Double-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadDoubleQuoteFollow) {
  fail_test("bad_double_quote_follow");
}

// \r should be treated as a whitespace, delimiting the end of a token
TEST_F(NginxConfigParserTest, CarriageReturn) {
  fail_test("crlf_config");
}