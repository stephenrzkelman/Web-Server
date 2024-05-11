#include "gtest/gtest.h"
#include "handlers/static_handler.h"
#include <boost/filesystem.hpp>

const std::string NOT_FOUND_RESPONSE_HEADER = 
"HTTP/1.1 404 Bad Request\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n";

class StaticHandlerTest : public testing::Test {
  protected:
    void SetUp() override{}
    void content_handle_success(
        std::string rel_path,
        std::string root_directory,
        std::string expected_response
    ){
        std::shared_ptr<RequestHandler> static_handler;
        static_handler.reset(new StaticHandler({ {"root", root_directory} }));
        request_data data;
        data.relative_path = rel_path;
        static_handler->handleRequest(data);
        EXPECT_EQ(static_handler->getLastResponse(), expected_response);
    }
};

// Handler should take poor request buffer and return a buffer sequence of a bad response.
TEST_F(StaticHandlerTest, EmptyFileRequest) {
  std::string EMPTY_OK_RESPONSE_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
  content_handle_success("/empty.txt", "../tests/files", EMPTY_OK_RESPONSE_HEADER);
}