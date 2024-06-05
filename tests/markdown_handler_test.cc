#include "filesystem/fake_filesystem.h"
#include "handlers/markdown_handler.h"
#include "gtest/gtest.h"
#include <memory>

class MarkdownHandlerTest : public testing::Test {
protected:
  void SetUp() override {}
};

TEST_F(MarkdownHandlerTest, PutSuccess) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/markdown/marge", "");
  filesystem->write("/mnt/markdown/homer", "");
  filesystem->write("/mnt/markdown/lisa", "");

  // define handler
  MarkdownHandler handler("/markdown", {{"data_path", "/mnt/markdown"}},
                        std::move(filesystem));

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::put);
  ok_request.set(boost::beast::http::field::content_type, MARKDOWN);
  ok_request.target("/markdown/burns.md");
  ok_request.body() = "NEW BODY BURNS";
  ok_request.prepare_payload();

  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::no_content);

  // check conctents
  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/markdown/burns.md");
  http_response response_get = handler.handle_request(get_request);
  EXPECT_EQ(response_get.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response_get.body(), "<p>NEW BODY BURNS</p>\n\n");
}

TEST_F(MarkdownHandlerTest, PutFailureDirectory) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/markdown/marge", "");
  filesystem->write("/mnt/markdown/homer", "");
  filesystem->write("/mnt/markdown/lisa", "");

  // define handler
  MarkdownHandler handler("/markdown", {{"data_path", "/mnt/markdown"}},
                      std::move(filesystem));

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::put);
  ok_request.set(boost::beast::http::field::content_type, MARKDOWN);
  ok_request.target("/mnt/markdown");
  ok_request.body() = "NEW BODY BURNS";
  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::bad_request);
}

TEST_F(MarkdownHandlerTest, PutSuccessNoFile) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/markdown/marge", "");
  filesystem->write("/mnt/markdown/homer", "");
  filesystem->write("/mnt/markdown/lisa", "");

  // define handler
  MarkdownHandler handler("/markdown", {{"data_path", "/mnt/markdown"}},
                      std::move(filesystem));

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::put);
  ok_request.set(boost::beast::http::field::content_type, MARKDOWN);
  ok_request.target("/mnt/markdown/santaslittlehelper.md");
  ok_request.body() = "NEW BODY BURNS";
  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::no_content);

  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/mnt/markdown/santaslittlehelper.md");
  http_response response_get = handler.handle_request(get_request);
  EXPECT_EQ(response_get.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response_get.at(boost::beast::http::field::content_type),
            TEXT_HTML);
  EXPECT_EQ(response_get.body(), "<p>NEW BODY BURNS</p>\n\n");

}