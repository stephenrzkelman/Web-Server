#include "constants.h"
#include "markdown_parser.h"

#include <cmark.h>
#include <iostream>

MarkdownParser::MarkdownParser(){}

std::string MarkdownParser::parse_markdown(std::string markdown){
  // CMARK_OPT_UNSAFE allows inline-html in the markdown
  cmark_node *node = cmark_parse_document(markdown.c_str(), markdown.length(), CMARK_OPT_UNSAFE);
  char *html = cmark_render_html(node, CMARK_OPT_UNSAFE);
  return std::string(html);
}
