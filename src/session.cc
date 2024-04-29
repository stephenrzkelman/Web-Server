#include "session.h"
#include "echo_request_handler.h"
#include <boost/bind/bind.hpp>

session::session(boost::asio::io_service& io_service) : socket_(io_service) {
    // TODO: reqHandler will be assigned echo or static handler based on url
    reqHandler = std::make_unique<echo_request_handler>();
}

tcp::socket& session::socket() {
    return socket_;
}

// Start reading from socket into buffer data_
// Completion handler is handle_read
void session::start() {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

// Creates HTTP response from a vector of buffers holding responseHeader and data_
// This vector of buffers is converted to a stirng then written to socket_ 
// Completion handler of this write to socket_ is handle_write  
void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        //Collect string of partial or full request
        partialRequest += boost::beast::buffers_to_string(boost::asio::buffer(data_, bytes_transferred));
        //See if the data fits in the buffer, if large, try to read in more data
        if (bytes_transferred == max_length) {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
                return;
        }
        boost::asio::async_write(socket_,
            reqHandler->handleRequest(boost::asio::buffer(partialRequest)),
            boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
        partialRequest = "";
    } else {
        delete this;
    }
}

// Read from socket into data_
// Completion handler is handle_read
// These two handle functions give us a loop allowing user to send another request after response from first is
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