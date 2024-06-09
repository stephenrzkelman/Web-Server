#include "constants.h"
#include "markdown_parser.h"

#include <cmark.h>
#include <iostream>

MarkdownParser::MarkdownParser(std::string format_path){
  html_style_line_ = "<link rel=\"stylesheet\" href=\"" + format_path + "\">\n";
}

std::string MarkdownParser::parse_markdown(std::string markdown){
  // CMARK_OPT_UNSAFE allows inline-html in the markdown
  cmark_node *node = cmark_parse_document(markdown.c_str(), markdown.length(), CMARK_OPT_UNSAFE);
  char *html_cstr = cmark_render_html(node, CMARK_OPT_UNSAFE);
  std::string html = MARKDOWN_HTML_START;
  html += html_style_line_;
  html += MARKDOWN_CLOSE_HEAD;
  html += MARKDOWN_OPEN_BODY;
  html += std::string(html_cstr);
  html += MARKDOWN_HTML_END;
  return html;
}
