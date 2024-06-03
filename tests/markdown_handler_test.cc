#include "filesystem/fake_filesystem.h"
#include "handlers/markdown_handler.h"
#include "gtest/gtest.h"
#include <memory>

class MarkdownHandlerTest : public testing::Test {
protected:
  void SetUp() override {}
};

// Generate some IDs, then read them
TEST_F(MarkdownHandlerTest, PostGet) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();

  MarkdownHandler markdown_handler("/markdown", {{"data_path", "/mnt/markdown"}},
                      std::move(filesystem));

  const std::string body1 = "This is the first write!";
  http_request ok_request1;
  ok_request1.method(boost::beast::http::verb::post);
  ok_request1.set(boost::beast::http::field::content_type, "text/markdown");
  ok_request1.target("/markdown/test/");
  ok_request1.body() = body1;

  http_response response1 = markdown_handler.handle_request(ok_request1);
  EXPECT_EQ(response1.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response1.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response1.body(), "1");

  const std::string body2 = "Another write";
  http_request ok_request2;
  ok_request2.method(boost::beast::http::verb::post);
  ok_request2.set(boost::beast::http::field::content_type, "text/markdown");
  ok_request2.target("/markdown/test/");
  ok_request2.body() = body2;

  http_response response2 = markdown_handler.handle_request(ok_request2);
  EXPECT_EQ(response2.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response2.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response2.body(), "2");

  // TODO: check that file contents are written to filesystem properly with GET. See crud_handler_test.cc for inspiration
}