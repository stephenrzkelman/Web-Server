#include "handlers/health_handler.h"
#include "gtest/gtest.h"

class HealthHandlerTest : public testing::Test {
protected: 
    void SetUp() override {}
    http_request parseRequest(boost::asio::mutable_buffer request) {
        boost::system::error_code error;
        boost::beast::http::request_parser<boost::beast::http::string_body> parser;
        parser.put(request, error);
        if (!error && parser.is_done()) {
        return parser.get();
        } else {
        return http_request();
        }
    }
    void health_handle_request(boost::asio::mutable_buffer request,
                            std::string expected_response) {
        std::shared_ptr<RequestHandler> healthHandler;
        healthHandler.reset(new HealthHandler());

        // Defined to help parse requests
        http_request data = parseRequest(request);

        healthHandler->handle_request(data);
        EXPECT_EQ(healthHandler->getLastResponse(), expected_response);
    }
};

TEST_F(HealthHandlerTest, GoodHealthRequest){
    std::string reqText = "GET /health HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: "
                          "curl/7.81.0\r\nAccept: */*\r\n\r\n";
    std::string resText = "HTTP/1.1 200 OK\r\nContent-Type: "
                          "text/plain\r\nContent-Length: 2\r\n\r\n";
    health_handle_request(boost::asio::buffer(reqText), resText + "Ok");
}