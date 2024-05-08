#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include "server.h"

#include <string>

class ServerTest : public testing::Test {
    protected:
        void SetUp() {}
};

// Basic initialization of server and calling of start_accept method
TEST_F(ServerTest, StartAndAccept) {
    // Dummy request manager setup
    std::shared_ptr<error_request_handler> error_handler;
    std::unordered_map<std::string, std::shared_ptr<request_handler>> request_handlers;
    std::vector<std::shared_ptr<Servlet>> servlets;
    RequestManager request_manager = RequestManager(
        request_handlers,
        error_handler,
        servlets
    );

    boost::asio::io_service io_service;
    ServerConfig config_data(request_manager, 8080);
    server s(io_service, config_data);
    s.start_accept();
}
