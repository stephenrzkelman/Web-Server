#include "filesystem/fake_filesystem.h"
#include "handlers/markdown_handler.h"
#include "gtest/gtest.h"
#include <memory>

const std::string MD_HTML_PREFIX = "<!DOCTYPE html>\n"
                                  "<html lang=\"en\">\n"
                                  "<head>\n"
                                  "<link rel=\"stylesheet\" href=\"stylesheet\">\n"
                                  "</head>\n"
                                  "<body>\n";
const std::string MD_HTML_SUFFIX = "</body>\n"
                                  "</html>\n";

class MarkdownHandlerTest : public testing::Test {
protected:
  void SetUp() override {}
};

TEST_F(MarkdownHandlerTest, PostSuccess) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/markdown/marge", "");
  filesystem->write("/mnt/markdown/homer", "");
  filesystem->write("/mnt/markdown/lisa", "");

  // define handler
  MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::post);
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
  std::string expected_html_body = "<p>NEW BODY BURNS</p>\n";
  std::string expected_response = MD_HTML_PREFIX + expected_html_body + MD_HTML_SUFFIX;
  EXPECT_EQ(response_get.body(), expected_response);
}

TEST_F(MarkdownHandlerTest, PutFailureDirectory) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/markdown/marge", "");
  filesystem->write("/mnt/markdown/homer", "");
  filesystem->write("/mnt/markdown/lisa", "");

  // define handler
  MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );

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

TEST_F(MarkdownHandlerTest, PostSuccessNewFile) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/markdown/marge", "");
  filesystem->write("/mnt/markdown/homer", "");
  filesystem->write("/mnt/markdown/lisa", "");

  // define handler
  MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::post);
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
  std::string expected_html_body = "<p>NEW BODY BURNS</p>\n";
  std::string expected_response = MD_HTML_PREFIX + expected_html_body + MD_HTML_SUFFIX;
  EXPECT_EQ(response_get.body(), expected_response);

}

// Deleting a directory is not allowed per design specifications of the MarkdownHandler. A bad request response should be returned
TEST_F(MarkdownHandlerTest, DeleteDirectory){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();

  const std::string body = "A file inside of a directory";
  const std::string file_path = "/mnt/markdown/dir/file.md";

  // 1: Write the file to the fake filesystem
  filesystem->write(file_path, body);

  // 2: Create the CrudHandler and make a DELETE request
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );
  http_request delete_request;
  delete_request.method(boost::beast::http::verb::delete_);
  delete_request.target("/markdown/dir");
  http_response response = handler.handle_request(delete_request);

  EXPECT_EQ(response.result(), boost::beast::http::status::bad_request);
  EXPECT_EQ(response.body(), "");
}

// Deleting a file should work, deleting a non-existing file should trigger an error response
TEST_F(MarkdownHandlerTest, DeleteFile){
    std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();

  const std::string body = "File to be deleted";
  const std::string file_path = "/mnt/markdown/file.md";

  // 1: Write the file to the fake filesystem
  filesystem->write(file_path, body);

  // 2: Create the CrudHandler and make a DELETE request
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );
  http_request delete_request;
  delete_request.method(boost::beast::http::verb::delete_);
  delete_request.target("/markdown/file.md");
  http_response response = handler.handle_request(delete_request);

  // 3: Verify that the file is deleted successfully
  EXPECT_EQ(response.result(), boost::beast::http::status::no_content);
  EXPECT_EQ(response.body(), "");

  // 4: Confirm that the file no longer exists in the filesystem
  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/markdown/file.md");
  http_response get_response = handler.handle_request(get_request);
  EXPECT_EQ(get_response.result(), boost::beast::http::status::not_found);

  // 5. Delete something that isn't there since it was just deleted
  http_request bad_del_request;
  bad_del_request.method(boost::beast::http::verb::delete_);
  bad_del_request.target("/markdown/file.md");
  http_response bad_del_response = handler.handle_request(bad_del_request);

  EXPECT_EQ(bad_del_response.result(), boost::beast::http::status::not_found);
  EXPECT_EQ(bad_del_response.body(), "");
}

TEST_F(MarkdownHandlerTest, UnsupportedRequestType){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );  http_request options_request;
  options_request.method(boost::beast::http::verb::options);
  options_request.target("/markdown/file.md");
  http_response options_response = handler.handle_request(options_request);
  EXPECT_EQ(options_response.result(), boost::beast::http::status::method_not_allowed);
}

TEST_F(MarkdownHandlerTest, NonMarkdownGetRequested){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/markdown/file.html");
  http_response get_response = handler.handle_request(get_request);
  EXPECT_EQ(get_response.result(), boost::beast::http::status::bad_request);
}

TEST_F(MarkdownHandlerTest, UpdateNonexistentFile){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );  http_request put_request;
  put_request.method(boost::beast::http::verb::put);
  put_request.set(boost::beast::http::field::content_type, MARKDOWN);
  put_request.target("/markdown/file.md");
  put_request.body() = "NEW BODY";
  http_response put_response = handler.handle_request(put_request);
  EXPECT_EQ(put_response.result(), boost::beast::http::status::not_found);
}

TEST_F(MarkdownHandlerTest, UpdateSuccess){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();
  filesystem->write("/mnt/markdown/file.md", "");
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );  http_request put_request;
  put_request.method(boost::beast::http::verb::put);
  put_request.set(boost::beast::http::field::content_type, MARKDOWN);
  put_request.target("/markdown/file.md");
  put_request.body() = "NEW BODY";
  http_response put_response = handler.handle_request(put_request);
  EXPECT_EQ(put_response.result(), boost::beast::http::status::no_content);
}

TEST_F(MarkdownHandlerTest, PostNonMarkdownFile){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );  http_request post_request;
  post_request.method(boost::beast::http::verb::post);
  post_request.set(boost::beast::http::field::content_type, MARKDOWN);
  post_request.target("/markdown/file.html");
  post_request.body() = "NEW BODY";
  http_response post_response = handler.handle_request(post_request);
  EXPECT_EQ(post_response.result(), boost::beast::http::status::bad_request);
}

TEST_F(MarkdownHandlerTest, PostDirectory){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();
  MarkdownHandler handler("/markdown", {{"data_path", "/mnt/markdown"}}, std::move(filesystem));
  http_request post_request;
  post_request.method(boost::beast::http::verb::post);
  post_request.set(boost::beast::http::field::content_type, MARKDOWN);
  post_request.target("/markdown/directory");
  post_request.body() = "NEW BODY";
  http_response post_response = handler.handle_request(post_request);
  EXPECT_EQ(post_response.result(), boost::beast::http::status::bad_request);
}

TEST_F(MarkdownHandlerTest, PostExistingFile){
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();
  filesystem->write("/mnt/markdown/file.md", "");
MarkdownHandler handler(
                      "/markdown", 
                      {
                        {"data_path", "/mnt/markdown"},
                        {"format_path", "stylesheet"}
                      },
                      std::move(filesystem)
  );  http_request post_request;
  post_request.method(boost::beast::http::verb::post);
  post_request.set(boost::beast::http::field::content_type, MARKDOWN);
  post_request.target("/markdown/file.md");
  post_request.body() = "NEW BODY";
  http_response post_response = handler.handle_request(post_request);
  EXPECT_EQ(post_response.result(), boost::beast::http::status::bad_request);
}