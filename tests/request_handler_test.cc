#include "gtest/gtest.h"
#include "request_handler.h"

class RequestHandlerTest : public testing::Test {
  protected:
    void SetUp() override{}

  request_handler reqHandler;
};

// Handler should take request buffer and return a buffer sequence including a response buffer and the request buffer itself.
TEST_F(RequestHandlerTest, EchoRequest) {
  std::string standard = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nthis is a test\n";
  std::string reqText = "this is a test\n";
  reqHandler.handleEchoRequest(boost::asio::buffer(reqText));
  EXPECT_EQ(reqHandler.getEchoResponse(),standard);
}

// Handler should take empty request buffer and return a buffer sequence including a response buffer and the request buffer itself.
TEST_F(RequestHandlerTest, EmptyEchoRequest) {
  std::string standard = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";
  std::string reqText = "";
  reqHandler.handleEchoRequest(boost::asio::buffer(reqText));
  EXPECT_EQ(reqHandler.getEchoResponse(),standard);
}