#include "session.h"
#include <boost/bind/bind.hpp>

session::session(boost::asio::io_service &io_service,
                 std::shared_ptr<RequestManager> request_manager)
    : socket_(io_service), request_manager_(request_manager) {}

tcp::socket &session::socket() { return socket_; }

// Start reading from socket into buffer data_
// Completion handler is handle_read
void session::start() {
  boost::beast::http::async_read(
      socket_, request_buf_, request_,
      boost::beast::bind_front_handler(&session::handle_read, this));
}

// Creates HTTP response from a vector of buffers holding responseHeader and
// data_ This vector of buffers is converted to a stirng then written to socket_
// Completion handler of this write to socket_ is handle_write
void session::handle_read(const boost::system::error_code &error,
                          size_t bytes_transferred) {
  if (error) {
    BOOST_LOG_TRIVIAL(error) << "Problem parsing the http request: "  << error.message();
    response_.result(boost::beast::http::status::bad_request);
    response_.set(boost::beast::http::field::content_type, "text/plain");
    boost::beast::http::write(socket_, response_);
    delete this;
    return;
  }
  response_ = request_manager_->manageRequest(request_);
  boost::beast::http::async_write(
      socket_, response_,
      boost::beast::bind_front_handler(&session::handle_write, this));
}

// Read from socket into data_
// Completion handler is handle_read
// These two handle functions give us a loop allowing user to send another
// request after response from first is
void session::handle_write(const boost::system::error_code &error,
                           size_t bytes_transferred) {
  if (error) {
    delete this;
    return;
  }
  request_.body() = "";
  boost::beast::http::async_read(
      socket_, request_buf_, request_,
      boost::beast::bind_front_handler(&session::handle_read, this));
}
