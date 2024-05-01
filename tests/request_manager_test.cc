#include "gtest/gtest.h"
#include "request_manager.h"
#include "config_parser.h"
#include "file_reader.h"
#include "echo_request_handler.h"
#include "content_request_handler.h"
#include "error_request_handler.h"
#include <fstream>

class RequestManagerTest : public testing::Test {
    protected:
      void SetUp(const char* config_file) {
        full_parsed_config = NginxConfig();
        parser.Parse(config_file, &full_parsed_config);
      }
      void manage_request_success(
        boost::asio::mutable_buffer request,
        std::string expected_response
      ){
        std::shared_ptr<request_handler> echo_handler;
        echo_handler.reset(new echo_request_handler());
        std::shared_ptr<request_handler> content_handler;
        std::ifstream file_handler;
        FileReader file_reader = FileReader(file_handler);
        content_handler.reset(new content_request_handler(file_reader));
        std::shared_ptr<error_request_handler> error_handler;
        error_handler.reset(new error_request_handler());
        std::unordered_map<std::string, std::shared_ptr<request_handler>> request_handlers = {
        {ECHO_REQUEST, echo_handler},
        {SERVE_CONTENT, content_handler}
        };
        std::vector<std::shared_ptr<Servlet>> servlets = full_parsed_config.findPaths();
        RequestManager request_manager = RequestManager(
            request_handlers,
            error_handler,
            servlets
        );
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
    SetUp("configs/two_paths_config");
    std::string success_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 84\r\n\r\n";
    manage_request_success(request, success_header + request_string);
}

TEST_F(RequestManagerTest, ManageNonexistentRequestTest){
    std::string request_string = "GET / HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/two_paths_config");
    std::string not_found_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
    manage_request_success(request, not_found_header);
}

TEST_F(RequestManagerTest, ManageEmptyRequestTest){
    std::string request_string = "";
    boost::asio::mutable_buffer request = boost::asio::buffer(request_string);
    SetUp("configs/two_paths_config");
    std::string not_found_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
    manage_request_success(request, not_found_header);
}