#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include <string>

const std::string MARKDOWN_HTML_START = "<!DOCTYPE html>\n"
                                        "<html lang=\"en\">\n"
                                        "<head>\n";
const std::string MARKDOWN_CLOSE_HEAD = "</head>\n";
const std::string MARKDOWN_OPEN_BODY = "<body>\n";
const std::string MARKDOWN_HTML_END = "</body>\n"
                                        "</html>\n";

class MarkdownParser {
public:
    MarkdownParser(std::string format_path);
    // treating the string argument as a markdown file, parse into html
    // return the html equivalent of the original markdown
    std::string parse_markdown(std::string markdown);
private:
    std::string html_style_line_;
};

#endif // MARKDOWN_PARSER_H