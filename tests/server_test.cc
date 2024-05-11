#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include "server.h"
#include "location_data.h"

#include <string>

class ServerTest : public testing::Test {
    protected:
        void SetUp() {}
};

// Basic initialization of server and calling of start_accept method
TEST_F(ServerTest, StartAndAccept) {

    //Setup request manager
    boost::asio::io_service io_service;
    std::unordered_map<std::string, LocationData> locations;
    RequestManager request_manager = RequestManager(locations);

    //Setup server
    server s(io_service, request_manager, 8080);
    s.start_accept();
}
