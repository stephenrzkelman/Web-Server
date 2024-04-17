#include "gtest/gtest.h"
#include "request_handler.h"

const std::string BAD_RESPONSE = 
"HTTP/1.1 400 Bad Request\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n";

class RequestHandlerTest : public testing::Test {
  protected:
    void SetUp() override{}

  request_handler reqHandler;
};

// Handler should take poor request buffer and return a buffer sequence of a bad response.
TEST_F(RequestHandlerTest, BadEchoRequest) {
  std::string reqText = "this is a test\n";
  reqHandler.handleEchoRequest(boost::asio::buffer(reqText));
  EXPECT_EQ(reqHandler.getLastResponse(),BAD_RESPONSE);
}

// Handler should take empty request buffer and return a buffer sequence of a bad response.
TEST_F(RequestHandlerTest, EmptyEchoRequest) {
  std::string reqText = "";
  reqHandler.handleEchoRequest(boost::asio::buffer(reqText));
  EXPECT_EQ(reqHandler.getLastResponse(),BAD_RESPONSE);
}
// Handler should take well formed HTTP/1.1 request buffer and return a buffer sequence including a response buffer and the request buffer itself.
TEST_F(RequestHandlerTest, GoodEchoRequest) {
  std::string reqText = "GET / HTTP/1.1\r\nHost: 127.0.0.1)\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";
  reqHandler.handleEchoRequest(boost::asio::buffer(reqText));
  EXPECT_EQ(reqHandler.getLastResponse(),reqHandler.getLastResponseHeader()+reqText);
}