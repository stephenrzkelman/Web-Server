#include "gtest/gtest.h"
#include "handlers/error_handler.h"

const std::string NOT_FOUND_HEADER = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";

class ErrorHandlerTest : public testing::Test {
  protected:
    void SetUp() override{}
    http_request parseRequest(boost::asio::mutable_buffer request){
      boost::system::error_code error;
      boost::beast::http::request_parser<boost::beast::http::string_body> parser;
      parser.put(request,error);
      if (!error && parser.is_done()) {
        return parser.get();
      }
      else {
        return http_request();
      }
    }
    void error_handle_test(
      boost::asio::mutable_buffer request,
      std::string expected_response
    ){
      std::shared_ptr<RequestHandler> errorHandler;
      errorHandler.reset(new ErrorHandler());
      
      //Defined to help parse requests
      http_request data = parseRequest(request);

      errorHandler->handleRequest(data);
      EXPECT_EQ(errorHandler->getLastResponse(), expected_response);
    }
};

// A request for an invalid URL should return a 404 header response with no body
TEST_F(ErrorHandlerTest, GibberishRequest) {
  std::string reqText = "GET /gibberish HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";
  error_handle_test(boost::asio::buffer(reqText), NOT_FOUND_HEADER);
}