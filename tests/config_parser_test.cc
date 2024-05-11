#include "gtest/gtest.h"
#include "config_parser.h"
#include "location_data.h"
#include "handlers/handler_registration.h"
#include <string>
#include <vector>

const std::string ECHO_REQUEST = "EchoHandler";
const std::string SERVE_CONTENT = "StaticHandler";

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
    "configs/example_config",
    "foo \"bar\";\nserver {\n  listen 80;\n  server_name foo.com;\n  root /home/ubuntu/sites/foo/;\n}\n"
  );
}

// Parsing a config with an unclosed brace should fail
TEST_F(NginxConfigParserTest, OpenBraceConfig) {
  parse_fail("configs/open_brace_config");
}

// Parsing a config with a dangling close-brace should fail
TEST_F(NginxConfigParserTest, CloseBraceConfig) {
  parse_fail("configs/close_brace_config");
}

// Quoted strings should allow escape-characters
TEST_F(NginxConfigParserTest, QuotesConfig) {
  parse_success(
    "configs/quotes_config",
    "quote1 \'\';\nquote2 \"\";\nescape1 \'\\\'\';\nescape2 \"\\\"\";\nend1 \'hello\' \'world\';\nend2 \"welcome\" \"home\";\n"
  );
}

// Nested braces should pass
TEST_F(NginxConfigParserTest, NestedBraces) {
  parse_success(
    "configs/docker_config",
    "server {\n  listen 80;\n  location = /echo {\n    behavior EchoHandler;\n  }\n}\n"
  );
}

// Single-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadSingleQuoteFollow) {
  parse_fail("configs/bad_single_quote_follow");
}

// Double-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadDoubleQuoteFollow) {
  parse_fail("configs/bad_double_quote_follow");
}

// \r should be treated as a whitespace, delimiting the end of a token
TEST_F(NginxConfigParserTest, CarriageReturn) {
  parse_fail("configs/crlf_config");
}

// parse should fail on nonexistent config file
TEST_F(NginxConfigParserTest, NonexistentConfig) {
  parse_fail("configs/nonexistent_config");
}

// only "normal-type" tokens should be able to label a block
TEST_F(NginxConfigParserTest, BadBlockLabel) {
  parse_fail("configs/semicolon_before_block_config");
}

// semicolons should only end a line consisting of "normal-type" tokens
TEST_F(NginxConfigParserTest, BadStatementEnd) {
  parse_fail("configs/semicolon_after_block_config");
}

// lines must end with semicolon
TEST_F(NginxConfigParserTest, MissingSemicolon) {
  parse_fail("configs/unterminated_nested_line_config");
  parse_fail("configs/unterminated_outer_line_config");
}

class NginxConfigTest : public testing :: Test {
  protected:
    void SetUp(const char* file_name) {
      full_parsed_config = NginxConfig();
      HandlerRegistration::register_handlers();
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
    void find_paths_success(std::unordered_map<std::string, LocationData> expected_locations){
      std::unordered_map<std::string, LocationData> locations = full_parsed_config.findPaths();
      EXPECT_EQ(locations.size(), expected_locations.size()); // same number of locations
      for(auto loc : locations){
        EXPECT_FALSE(expected_locations.find(loc.first) == expected_locations.end()); // path exists in expected_locations
        LocationData location_to_test = expected_locations[loc.first]; // expected location with same path
        EXPECT_EQ(location_to_test.handler_, loc.second.handler_); // handlers are the same
        EXPECT_EQ(location_to_test.arg_map_.size(), loc.second.arg_map_.size()); // same number of args
        for(auto arg : loc.second.arg_map_) {
          EXPECT_FALSE(location_to_test.arg_map_.find(arg.first) == location_to_test.arg_map_.end()); // arg exists in expected
          EXPECT_EQ(location_to_test.arg_map_[arg.first], arg.second); // passed in value to arg matches expected
        }
      }
    }
    void find_paths_failure(){
      std::unordered_map<std::string, LocationData> locations = full_parsed_config.findPaths();
      EXPECT_EQ(locations.size(), 0);
    }

  NginxConfig full_parsed_config;
  NginxConfigParser parser;
};

// the minimal config specifying only a port inside http{server{...}} should be valid
// the minimal config specifying port 80 inside http{server{...}} should have 80 extracted as the desired port
TEST_F(NginxConfigTest, DockerConfig) {
  SetUp("configs/docker_config");
  validation_success();
  find_port_success(80);
}

// the findPort step should fail for configs with no port or multiple ports specified under the 'listen' directive
TEST_F(NginxConfigTest, IncorrectListenArgCount){
  SetUp("configs/too_many_ports_config");
  find_port_failure();
  SetUp("configs/no_ports_config");
  find_port_failure();
}

// configs with unrecognized directives should fail validation
TEST_F(NginxConfigTest, UnrecognizedDirective){
  SetUp("configs/example_config");
  validation_failure();
  SetUp("configs/port_in_main_config");
  validation_failure();
}

// configs with non-unique "listen" directives should fail the findPort step
TEST_F(NginxConfigTest, NonUniqueDirective){
  SetUp("configs/non_unique_listen_config");
  find_port_failure();
}

// if subcontext name erroneously used as directive label, validation should fail
TEST_F(NginxConfigTest, ContextLabeledButNotPresent){
  SetUp("configs/bad_server_line_config");
  validation_failure();
}

// if port number is not a positive integer in the range 0-65535, findPort should "fail"
TEST_F(NginxConfigTest, BadPortProvided){
  SetUp("configs/alphabetical_port_config");
  find_port_failure();
  SetUp("configs/negative_port_config");
  find_port_failure();
  SetUp("configs/decimal_port_config");
  find_port_failure();
  SetUp("configs/port_too_large_config");
  find_port_failure();
}

// if a context gets arguments but doesn't require any, it should be flagged
TEST_F(NginxConfigTest, OverlabeledHTTPConfig){
  SetUp("configs/over_labeled_config");
  find_port_failure();
  find_paths_failure();
}

// should fail to find paths in a config which doesn't specify any
TEST_F(NginxConfigTest, NoPaths){
  SetUp("configs/no_paths_config");
  find_paths_failure();
}

// should succeed at extracting single locations in a config which specifies only 1 path
TEST_F(NginxConfigTest, OnePath){
  SetUp("configs/one_path_config");
  find_paths_success({ {"/echo", LocationData(ECHO_REQUEST, {})} });
}

// should succeed at extracting 2 locations from config which specifies 2 paths
TEST_F(NginxConfigTest, TwoPaths){
  SetUp("configs/two_paths_config");
  find_port_success(80);
  find_paths_success({ 
    {"/echo", LocationData(ECHO_REQUEST, {})}, 
    {"/static", LocationData(SERVE_CONTENT, { {"root","/etc/files"} })} });
}

// should fail if any match modifier is invalid
TEST_F(NginxConfigTest, InvalidMatchModifier){
  SetUp("configs/invalid_match_modifier_config");
  find_paths_failure();
}

// should fail if more than one server block is provided
TEST_F(NginxConfigTest, MultipleServerBlocks){
  SetUp("configs/multiple_server_blocks_config");
  find_port_failure();
  find_paths_failure();
}

// should fail if multiple behaviors are specified
TEST_F(NginxConfigTest, AmbiguousBehavior){
  SetUp("configs/ambiguous_behavior_config");
  find_paths_failure();
}

// should fail if unrecognized behavior is specified
TEST_F(NginxConfigTest, UnrecognizedBehavior){
  SetUp("configs/undefined_behavior_config");
  find_paths_failure();
}

// no default behavior should be specified
TEST_F(NginxConfigTest, DefaultBehavior){
  SetUp("configs/default_behavior_config");
  find_paths_failure();
}

// This is due to the fact that handlers are not known ahead of time anymore
// any badly configured locations should cause find paths to fail
TEST_F(NginxConfigTest, BadLocations){
  SetUp("configs/prefix_echo_with_root_config");
  find_paths_failure();
  SetUp("configs/echo_with_root_config");
  find_paths_failure();
  SetUp("too_many_roots_config");
  find_paths_failure();
}
