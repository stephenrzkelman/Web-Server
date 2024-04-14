#include "gtest/gtest.h"
#include "session.h"

class SessionTest : public testing::Test {
  protected:
    boost::asio::io_service io_service;
    session* new_session = new session(io_service);
    const boost::system::error_code error;
    size_t bytes;
};

//Test basic config file for success
// TEST_F(SessionTest, Read) {
//   std::string test = "read test";
//   new_session->socket_ = fakeSocket;
//   new_session->handle_read(error,bytes);
//   EXPECT_EQ(new_session->getResponse(),"bah");
// }