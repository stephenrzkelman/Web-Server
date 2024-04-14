#include "session.h"

session::session(boost::asio::io_service& io_service) : socket_(io_service) {
    responseHeader = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";
}

tcp::socket& session::socket() {
    return socket_;
}

void session::start() {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        std::vector<boost::asio::streambuf::mutable_buffers_type> responseBuffer;
        responseBuffer.push_back(boost::asio::buffer(responseHeader));
        responseBuffer.push_back(boost::asio::buffer(data_, max_length));
        response = boost::beast::buffers_to_string(responseBuffer);
        boost::asio::async_write(socket_,
            responseBuffer,
            boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
    } else {
        delete this;
    }
}

void session::handle_write(const boost::system::error_code& error) {
    if (!error) {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    } else {
        delete this;
    }
}

std::string session::getResponse() {
    return response;
}
