#include "markdown_parser.h"
#include "gtest/gtest.h"
#include <string>
#include <fstream>
#include <sstream>

TEST(MarkdownParserTest, MarkdownToHTMLConversion) {
    std::stringstream md_buffer;
    std::ifstream md_stream("./markdown/sample.md");
    md_buffer << md_stream.rdbuf();
    std::string md_string = md_buffer.str();
    std::stringstream html_buffer;
    std::ifstream html_stream("./markdown/sample.html");
    html_buffer << html_stream.rdbuf();
    std::string html_string = html_buffer.str();
    MarkdownParser parser("stylesheet");
    std::string parsed_markdown = parser.parse_markdown(md_string);
    EXPECT_EQ(parsed_markdown, html_string);
}