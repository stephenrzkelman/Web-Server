#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include <string>

class MarkdownParser {
public:
    MarkdownParser();
    // treating the string argument as a markdown file, parse into html
    // return the html equivalent of the original markdown
    std::string parse_markdown(std::string markdown);
};

#endif // MARKDOWN_PARSER_H