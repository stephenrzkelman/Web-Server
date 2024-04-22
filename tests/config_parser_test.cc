#include "gtest/gtest.h"
#include "config_parser.h"
#include <string>

class NginxConfigParserTest : public testing::Test {
  protected:
    void SetUp() override{}
    void parse_success(const char* file_name, std::string expected_config_string) {
      EXPECT_TRUE(parser.Parse(file_name, &out_config));
      EXPECT_EQ(out_config.ToString(0).c_str(), expected_config_string);
    }
    void parse_fail(const char* file_name) {
      EXPECT_FALSE(parser.Parse(file_name, &out_config));
    }

  NginxConfigParser parser;
  NginxConfig out_config;
};

// the basic given config should parse without failure
TEST_F(NginxConfigParserTest, SimpleConfig) {
  parse_success(
    "example_config",
    "foo \"bar\";\nserver {\n  listen 80;\n  server_name foo.com;\n  root /home/ubuntu/sites/foo/;\n}\n"
  );
}

// Parsing a config with an unclosed brace should fail
TEST_F(NginxConfigParserTest, OpenBraceConfig) {
  parse_fail("open_brace_config");
}

// Parsing a config with a dangling close-brace should fail
TEST_F(NginxConfigParserTest, CloseBraceConfig) {
  parse_fail("close_brace_config");
}

// Quoted strings should allow escape-characters
TEST_F(NginxConfigParserTest, QuotesConfig) {
  parse_success(
    "quotes_config",
    "quote1 \'\';\nquote2 \"\";\nescape1 \'\\\'\';\nescape2 \"\\\"\";\nend1 \'hello\' \'world\';\nend2 \"welcome\" \"home\";\n"
  );
}

// Nested braces should pass
TEST_F(NginxConfigParserTest, NestedBraces) {
  parse_success(
    "docker_config",
    "http {\n  server {\n    listen 80;\n  }\n}\n"
  );
}

// Single-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadSingleQuoteFollow) {
  parse_fail("bad_single_quote_follow");
}

// Double-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadDoubleQuoteFollow) {
  parse_fail("bad_double_quote_follow");
}

// \r should be treated as a whitespace, delimiting the end of a token
TEST_F(NginxConfigParserTest, CarriageReturn) {
  parse_fail("crlf_config");
}

// parse should fail on nonexistent config file
TEST_F(NginxConfigParserTest, NonexistentConfig) {
  parse_fail("nonexistent_config");
}

// only "normal-type" tokens should be able to label a block
TEST_F(NginxConfigParserTest, BadBlockLabel) {
  parse_fail("semicolon_before_block_config");
}

// semicolons should only end a line consisting of "normal-type" tokens
TEST_F(NginxConfigParserTest, BadStatementEnd) {
  parse_fail("semicolon_after_block_config");
}

// lines must end with semicolon
TEST_F(NginxConfigParserTest, MissingSemicolon) {
  parse_fail("unterminated_nested_line_config");
  parse_fail("unterminated_outer_line_config");
}

// subcontexts must be labeled by single word/token
TEST_F(NginxConfigParserTest, OverlabeledContext) {
  parse_fail("over_labeled_config");
}

class NginxConfigTest : public testing :: Test {
  protected:
    void SetUp(const char* file_name) {
      full_parsed_config = NginxConfig();
      parser.Parse(file_name, &full_parsed_config);
    }
    void validation_success(){
      EXPECT_TRUE(full_parsed_config.Validate());
    }
    void validation_failure(){
      EXPECT_FALSE(full_parsed_config.Validate());
    }
    void find_port_success(int expected_port){
      EXPECT_EQ(full_parsed_config.findPort(), expected_port);
    }
    void find_port_failure(){
      EXPECT_EQ(full_parsed_config.findPort(), -1);
    }

  NginxConfig full_parsed_config;
  NginxConfigParser parser;
};

// the minimal config specifying only a port inside http{server{...}} should be valid
// the minimal config specifying port 80 inside http{server{...}} should have 80 extracted as the desired port
TEST_F(NginxConfigTest, DockerConfig) {
  SetUp("docker_config");
  validation_success();
  find_port_success(80);
}

// the findPort step should fail for configs with no port or multiple ports specified under the 'listen' directive
TEST_F(NginxConfigTest, IncorrectListenArgCount){
  SetUp("too_many_ports_config");
  find_port_failure();
  SetUp("no_ports_config");
  find_port_failure();
}

// configs with unrecognized directives should fail validation
TEST_F(NginxConfigTest, UnrecognizedDirective){
  SetUp("example_config");
  validation_failure();
  SetUp("port_in_main_config");
  validation_failure();
  SetUp("port_in_http_config");
  validation_failure();
}

// configs with non-unique "listen" directives should fail the findPort step
TEST_F(NginxConfigTest, NonUniqueDirective){
  SetUp("non_unique_listen_config");
  find_port_failure();
}

// if subcontext name erroneously used as directive label, validation should fail
TEST_F(NginxConfigTest, ContextLabeledButNotPresent){
  SetUp("bad_server_line_config");
  validation_failure();
}

// if port number is not a positive integer in the range 0-65535, findPort should "fail"
TEST_F(NginxConfigTest, BadPortProvided){
  SetUp("alphabetical_port_config");
  find_port_failure();
  SetUp("negative_port_config");
  find_port_failure();
  SetUp("decimal_port_config");
  find_port_failure();
  SetUp("port_too_large_config");
  find_port_failure();
}
