#include "gtest/gtest.h"
#include "request_manager.h"
#include "config_parser.h"
#include "handlers/handler_registration.h"
#include <fstream>

class RequestManagerTest : public testing::Test {
    protected:
      void SetUp(const char* config_file) {
        full_parsed_config = NginxConfig();
        parser.Parse(config_file, &full_parsed_config);
        HandlerRegistration::register_handlers();
      }
      void manage_request_success(
        boost::asio::mutable_buffer request,
        std::string expected_response
      ){
        std::unordered_map<std::string, LocationData> locations = full_parsed_config.findLocations().value();
        RequestManager request_manager = RequestManager(locations);
        std::string result = request_manager.manageRequest(request);
        EXPECT_EQ(result, expected_response);
      }
    NginxConfigParser parser;
    NginxConfig full_parsed_config;
};

// 
TEST_F(RequestManagerTest, ManageEchoRequestTest){
    std::string request_string = "GET /echo HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/all_items_config");
    std::string success_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 84\r\n\r\n";
    manage_request_success(request, success_header + request_string);
}

TEST_F(RequestManagerTest, ManageBadPathRequestTest){
    std::string request_string = "GET echo HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/all_items_config");
    std::string not_found_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
    manage_request_success(request, not_found_header);
}

TEST_F(RequestManagerTest, ManageBadEchoRequestTest){
    std::string request_string = "GET /echoP HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/all_items_config");
    std::string not_found_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
    manage_request_success(request, not_found_header);
}

TEST_F(RequestManagerTest, ManageBadEchoRequestTest2){
    std::string request_string = "GET /ech HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/all_items_config");
    std::string not_found_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
    manage_request_success(request, not_found_header);
}

TEST_F(RequestManagerTest, ManageBadEchoRequestTest3){
    std::string request_string = "GET /echo/extra HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/all_items_config");
    std::string success_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 90\r\n\r\n";
    manage_request_success(request, success_header + request_string);
}


TEST_F(RequestManagerTest, ManageNonexistentRequestTest){
    std::string request_string = "GET / HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/all_items_config");
    std::string not_found_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
    manage_request_success(request, not_found_header);
}

TEST_F(RequestManagerTest, ManageEmptyRequestTest){
    std::string request_string = "";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/all_items_config");
    std::string not_found_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
    manage_request_success(request, not_found_header);
}