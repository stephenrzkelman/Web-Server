#include "markdown_parser.h"
#include "gtest/gtest.h"
#include <string>
#include <fstream>
#include <sstream>

class MarkdownParserTest : public testing::Test {
protected:
    void SetUp() override {}
    void parse_success(std::string markdown,
                       std::string expected_html) {
        EXPECT_EQ(parser.parse_markdown(markdown), expected_html);
    }
    MarkdownParser parser;
};

TEST_F(MarkdownParserTest, MarkdownToHTMLConversion) {
    std::stringstream md_buffer;
    std::ifstream md_stream("./markdown/sample.md");
    md_buffer << md_stream.rdbuf();
    std::string md_string = md_buffer.str();
    std::stringstream html_buffer;
    std::ifstream html_stream("./markdown/sample.html");
    html_buffer << html_stream.rdbuf();
    std::string html_string = html_buffer.str();
    parse_success(md_string, html_string);
}