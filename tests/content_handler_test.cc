#include "gtest/gtest.h"
#include "content_request_handler.h"
#include <boost/filesystem.hpp>

const std::string NOT_FOUND_RESPONSE_HEADER = 
"HTTP/1.1 404 Bad Request\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n";

class ContentHandlerTest : public testing::Test {
  protected:
    void SetUp() override{}
    void content_handle_success(
        boost::asio::mutable_buffer request,
        std::string root_directory,
        std::string expected_response
    ){
        std::shared_ptr<request_handler> content_handler;
        std::ifstream file_handler;
        FileReader file_reader = FileReader(file_handler);
        content_handler.reset(new content_request_handler(file_reader));
        request_data data;
        boost::beast::http::request_parser<boost::beast::http::string_body> parser;
        boost::system::error_code error;
        parser.put(request,error);
        data.parsed_request = &parser.get();
        data.root_directory = root_directory;
        content_handler->handleRequest(data);
        EXPECT_EQ(content_handler->getLastResponse(), expected_response);
    }
};

// Handler should take poor request buffer and return a buffer sequence of a bad response.
TEST_F(ContentHandlerTest, EmptyFileRequest) {
  std::string EMPTY_OK_RESPONSE_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
  std::string REQUEST_HEADER = "GET /empty.txt HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\n\r\n";
  content_handle_success(boost::asio::buffer(REQUEST_HEADER), "../tests/files", EMPTY_OK_RESPONSE_HEADER);
}