#include "handlers/static_handler.h"
#include "gtest/gtest.h"
#include "filesystem/filesystem.h"

const std::string NOT_FOUND_RESPONSE_HEADER = "HTTP/1.1 404 Bad Request\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n";

class StaticHandlerTest : public testing::Test {
protected:
  void SetUp() override {}
  void content_handle_success(std::string rel_path, std::string root_directory,
                              std::string expected_response) {
    std::shared_ptr<RequestHandler> static_handler;
    static_handler.reset(
        new StaticHandler("/static", {{"root", root_directory}}, std::make_unique<FileSystem>()));
    http_request data {boost::beast::http::verb::get,rel_path,11};
    static_handler->handle_request(data);
    EXPECT_EQ(static_handler->getLastResponse(), expected_response);
  }
};

// Handler should take poor request buffer and return a buffer sequence of a bad
// response.
TEST_F(StaticHandlerTest, EmptyFileRequest) {
  std::string EMPTY_OK_RESPONSE_HEADER =
      "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
      "0\r\n\r\n";
  content_handle_success("/static/empty.txt", "../tests/files",
                         EMPTY_OK_RESPONSE_HEADER);
}