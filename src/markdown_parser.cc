#include "constants.h"
#include "markdown_parser.h"

#include <markdown.h>
#include <sstream>
#include <stack>
#include <iostream>

MarkdownParser::MarkdownParser(){}

std::string MarkdownParser::parse_markdown(std::string markdown){
  markdown::Document document;
  document.read(markdown);
  std::ostringstream html;
  document.write(html);
  return html.str();
}
