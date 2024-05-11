#include "gtest/gtest.h"
#include "handlers/echo_handler.h"

const std::string BAD_RESPONSE = 
"HTTP/1.1 400 Bad Request\r\n\
Content-Type: text/plain\r\n\
Content-Length: 0\r\n\
\r\n";

class EchoHandlerTest : public testing::Test {
  protected:
    void SetUp() override{}

  EchoHandler echoHandler;
};

// Handler should take poor request buffer and return a buffer sequence of a bad response.
TEST_F(EchoHandlerTest, BadEchoRequest) {
  std::string reqText = "this is a test\n";
  request_data request;
  request.raw_request = boost::asio::buffer(reqText);
  echoHandler.handleRequest(request);
  EXPECT_EQ(echoHandler.getLastResponse(),echoHandler.getLastResponseHeader()+reqText);
}

// Handler should take empty request buffer and return a buffer sequence of a bad response.
TEST_F(EchoHandlerTest, EmptyEchoRequest) {
  std::string reqText = "";
  request_data request;
  request.raw_request = boost::asio::buffer(reqText);
  echoHandler.handleRequest(request);
  EXPECT_EQ(echoHandler.getLastResponse(),echoHandler.getLastResponseHeader()+reqText);
}
// Handler should take well formed HTTP/1.1 request buffer and return a buffer sequence including a response buffer and the request buffer itself.
TEST_F(EchoHandlerTest, GoodEchoRequest) {
  std::string reqText = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";
  request_data request;
  request.raw_request = boost::asio::buffer(reqText);
  echoHandler.handleRequest(request);
  EXPECT_EQ(echoHandler.getLastResponse(),echoHandler.getLastResponseHeader()+reqText);
}

// Handler should take well formed HTTP/1.1 large request buffer and return entire message in response body.
TEST_F(EchoHandlerTest, LargeEchoRequest) {
  std::string reqText = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nLorem-ipsum: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Sagittis aliquam malesuada bibendum arcu. Pharetra diam sit amet nisl suscipit. Sollicitudin aliquam ultrices sagittis orci a scelerisque purus. Turpis in eu mi bibendum neque egestas congue quisque egestas. Dignissim convallis aenean et tortor at. Velit aliquet sagittis id consectetur purus ut faucibus pulvinar elementum. Tristique nulla aliquet enim tortor at auctor urna nunc id. Ornare aenean euismod elementum nisi quis eleifend quam adipiscing vitae. Egestas erat imperdiet sed euismod. Vitae sapien pellentesque habitant morbi tristique senectus et netus et. Egestas pretium aenean pharetra magna ac placerat. Quis ipsum suspendisse ultrices gravida dictum. Est sit amet facilisis magna etiam. Leo urna molestie at elementum. Arcu non odio euismod lacinia at quis. Amet volutpat consequat mauris nunc congue nisi. Diam maecenas ultricies mi eget mauris pharetra et ultrices neque. Pharetra magna ac placerat vestibulum lectus mauris ultrices. Facilisis sed odio morbi quis commodo odio aenean sed adipiscing. Vulputate eu scelerisque felis imperdiet proin fermentum leo vel orci. Vitae ultricies leo integer malesuada nunc vel risus commodo. Id nibh tortor id aliquet lectus. Sit amet venenatis urna cursus eget nunc scelerisque viverra mauris. Praesent semper feugiat nibh sed pulvinar proin gravida hendrerit lectus. Congue mauris rhoncus aenean vel elit scelerisque mauris pellentesque pulvinar. Lectus arcu bibendum at varius vel pharetra vel turpis. Ut tellus elementum sagittis vitae. Urna neque viverra justo nec. Duis convallis convallis tellus id interdum velit laoreet. Neque sodales ut etiam sit amet nisl. Interdum consectetur libero id faucibus nisl tincidunt eget. Aliquam faucibus purus in massa tempor nec. Facilisi nullam vehicula ipsum a arcu cursus vitae congue mauris. Nisi est sit amet facilisis magna etiam tempor orci. Diam maecenas sed enim ut sem. Mauris pellentesque pulvinar pellentesque habitant morbi tristique senectus et netus. Lectus arcu bibendum at varius. Eu nisl nunc mi ipsum. Ridiculus mus mauris vitae ultricies leo. Interdum consectetur libero id faucibus nisl tincidunt. Auctor augue mauris augue neque gravida. Facilisi etiam dignissim diam quis enim lobortis scelerisque. Venenatis lectus magna fringilla urna. Tellus integer feugiat scelerisque varius morbi enim. Diam quam nulla porttitor massa id. Diam maecenas ultricies mi eget mauris pharetra et ultrices neque. Ipsum suspendisse ultrices gravida dictum fusce ut. Lobortis mattis aliquam faucibus purus in massa. Ultricies integer quis auctor elit sed.\r\n\r\n";
  request_data request;
  request.raw_request = boost::asio::buffer(reqText);
  echoHandler.handleRequest(request);
  EXPECT_EQ(echoHandler.getLastResponse(),echoHandler.getLastResponseHeader()+reqText);
}