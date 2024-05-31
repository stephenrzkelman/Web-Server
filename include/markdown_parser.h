#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include <string>
#include <optional>

class MarkdownParser {
public:
    MarkdownParser();
    // treating the string argument as a markdown file, parse into html
    // if parse is successful, return the html equivalent of the original markdown
    // otherwise, return std::nullopt
    std::optional<std::string> parse_markdown(std::string markdown);
};

#endif // MARKDOWN_PARSER_H