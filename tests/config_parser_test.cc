#include "gtest/gtest.h"
#include "config_parser.h"
#include "location_data.h"
#include <string>
#include <vector>

const std::string ECHO_HANDLER = "EchoHandler";
const std::string STATIC_HANDLER = "StaticHandler";

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

// unrecognized keywords should cause parse to fail
TEST_F(NginxConfigParserTest, BadKeywordConfig) {
    parse_fail("configs/bad_keyword_config");
}

// unclosed brace should cause parse to fail
TEST_F(NginxConfigParserTest, OpenBraceConfig) {
  parse_fail("configs/open_brace_config");
}

// a dangling close-brace should cause parse to fail
TEST_F(NginxConfigParserTest, CloseBraceConfig) {
  parse_fail("configs/close_brace_config");
}

// single and double quotes should allow escape characters
TEST_F(NginxConfigParserTest, QuotesConfig) {
  parse_success(
    "configs/quotes_config",
    "port '';\nport \"\";\nport '\\'';\nport \"\\\"\";\nlocation 'hello' 'world' {\n}\nlocation \"welcome\" \"home\" {\n}\n"
  );
}

// single-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadSingleQuoteFollow) {
  parse_fail("configs/bad_single_quote_follow");
}

// double-quoted strings should be followed by a space or a semicolon
TEST_F(NginxConfigParserTest, BadDoubleQuoteFollow) {
  parse_fail("configs/bad_double_quote_follow");
}

// \r should be treated as a whitespace, delimiting the end of a token
TEST_F(NginxConfigParserTest, CarriageReturn) {
  parse_fail("configs/crlf_config");
}

// parse should fail on a nonexistent config file
TEST_F(NginxConfigParserTest, NonexistentConfig) {
  parse_fail("configs/nonexistent_config");
}

// only "normal-type" tokens should be able to label a block
// i.e., no semicolons in block label
TEST_F(NginxConfigParserTest, BadBlockLabel) {
  parse_fail("configs/semicolon_before_block_config");
}

// semicolons should only terminate "normal-type" lines
// i.e., no semicolons after a block
TEST_F(NginxConfigParserTest, BadStatementEnd) {
  parse_fail("configs/semicolon_after_block_config");
}

// lines must end with a semicolon
TEST_F(NginxConfigParserTest, MissingSemicolon) {
  parse_fail("configs/unterminated_nested_line_config");
  parse_fail("configs/unterminated_outer_line_config");
}

// config with extra arguments for some keyword should fail to parse
TEST_F(NginxConfigParserTest, TooManyArgs) {
    parse_fail("configs/too_many_args_config");
}

// config with too few arguments for some keyword should fail to parse
TEST_F(NginxConfigParserTest, TooFewArgs) {
    parse_fail("configs/too_few_args_config");
}

// using directive keyword to label block should cause parse to fail
TEST_F(NginxConfigParserTest, DirectiveAsContext) {
    parse_fail("configs/directive_as_context_config");
}

// using context keyword as directive should cause parse to fail
TEST_F(NginxConfigParserTest, ContextAsDirective) {
    parse_fail("configs/context_as_directive_config");
}

// config with comments at the end of some lines should parse successfully
TEST_F(NginxConfigParserTest, Comments) {
    parse_success(
        "configs/comments_config",
        "port 80;\n"
    );
}
class NginxConfigTest : public testing :: Test {
  protected:
    void SetUp(const char* file_name) {
      full_parsed_config = NginxConfig();
      parser.Parse(file_name, &full_parsed_config);
    }
    void find_port_success(int expected_port){
      EXPECT_EQ(full_parsed_config.findPort(), expected_port);
    }
    void find_port_failure(){
      EXPECT_EQ(full_parsed_config.findPort(), -1);
    }
    void find_locations_success(std::unordered_map<std::string, LocationData> expected_locations){
      std::optional<std::unordered_map<std::string, LocationData>> locations = full_parsed_config.findLocations();
      EXPECT_TRUE(locations.has_value());
      EXPECT_EQ(locations.value().size(), expected_locations.size()); // same number of locations
      for(auto loc : locations.value()){
        EXPECT_TRUE(expected_locations.contains(loc.first)); // path exists in expected_locations
        LocationData location_to_test = expected_locations[loc.first]; // expected location with same path
        EXPECT_EQ(location_to_test.handler_, loc.second.handler_); // handlers are the same
        EXPECT_EQ(location_to_test.arg_map_.size(), loc.second.arg_map_.size()); // same number of args
        for(auto arg : loc.second.arg_map_) {
          EXPECT_TRUE(location_to_test.arg_map_.contains(arg.first)); // arg exists in expected
          EXPECT_EQ(location_to_test.arg_map_[arg.first], arg.second); // passed in value to arg matches expected
        }
      }
    }
    void find_locations_failure(){
      std::optional<std::unordered_map<std::string, LocationData>> locations = full_parsed_config.findLocations();
      EXPECT_FALSE(locations.has_value());
    }

  NginxConfig full_parsed_config;
  NginxConfigParser parser;
};

// empty config should parse successfully
// but finding the port should fail
// while finding locations should succeed and find nothing
TEST_F(NginxConfigTest, Empty) {
    SetUp("configs/empty_config");
    find_port_failure();
    find_locations_success({});
}

// config with only a single port line should parse successfully
// and finding the port should succeed
TEST_F(NginxConfigTest, SinglePort) {
    SetUp("configs/single_port_config");
    find_port_success(80);
}

// config with multiple ports should parse successfully
// but finding the port should fail
TEST_F(NginxConfigTest, MultiplePorts) {
    SetUp("configs/multiple_port_config");
    find_port_failure();
}

// config with some distinct locations should parse successfully
// while finding locations should succeed
TEST_F(NginxConfigTest, GoodLocations) {
    SetUp("configs/good_locations_config");
    find_locations_success({
        {"/echo", LocationData(ECHO_HANDLER, {})}, 
        {"/static", LocationData(STATIC_HANDLER, { {"root","/etc/files"} })}
    });
}

// trailing slashes on locations should be "ignored" for the purposes of the stored location data
TEST_F(NginxConfigTest, TrailingSlash) {
    SetUp("configs/trailing_slash_config");
    find_locations_success({
        {"/echo", LocationData(ECHO_HANDLER, {})}, 
        {"/static", LocationData(STATIC_HANDLER, { {"root","/etc/files"} })}
    });
}

// config with a duplicate location should parse successfully
// but finding locations should fail
TEST_F(NginxConfigTest, DuplicateLocations) {
    SetUp("configs/duplicate_locations_config");
    find_locations_failure();
}

// config with port outside range 0-65535 should parse successfully,
// but finding port should fail
TEST_F(NginxConfigTest, InvalidPortNumber) {
    SetUp("configs/alphabetical_port_config");
    find_port_failure();
    SetUp("configs/negative_port_config");
    find_port_failure();
    SetUp("configs/decimal_port_config");
    find_port_failure();
    SetUp("configs/port_too_large_config");
    find_port_failure();
}

// config with invalid handler type should parse successfully,
// but finding locations should fail
TEST_F(NginxConfigTest, InvalidHandlerType) {
    SetUp("configs/invalid_handler_type_config");
    find_locations_failure();
}

// config with static handler taking no roots should parse successfully,
// but finding locations should fail
TEST_F(NginxConfigTest, StaticNoRoots) {
    SetUp("configs/static_no_roots_config");
    find_locations_failure();
}

// config with echo handler taking a root should parse successfully,
// but finding locations should fail
TEST_F(NginxConfigTest, EchoGivenRoot) {
    SetUp("configs/echo_given_root_config");
    find_locations_failure();
}

// config with static handler taking more than one root should parse successfully,
// but finding locations should fail
TEST_F(NginxConfigTest, StaticMultipleRoots){
    SetUp("configs/static_multiple_roots_config");
    find_locations_failure();
}

// arguments should be allowed to be quoted
TEST_F(NginxConfigTest, QuotedArgs){
  SetUp("configs/quoted_args_config");
  find_locations_success({
        {"/echo", LocationData(ECHO_HANDLER, {})}, 
        {"/static", LocationData(STATIC_HANDLER, { {"root","\"/etc/files\""} })}
    });
}
