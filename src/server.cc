#include "server.h"
#include "signal_handler.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/trivial.hpp>

namespace asio = boost::asio;

server::server(boost::asio::io_service &io_service,
               RequestManager &request_manager, unsigned short port)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
  request_manager_.reset(&request_manager);
  BOOST_LOG_TRIVIAL(info) << "Server starting up";
}

void server::start_accept() {
  // Log that the connection is started, add signal handler
  BOOST_LOG_NAMED_SCOPE("Start Accept")
  BOOST_LOG_TRIVIAL(info) << "Ready to accept a new connection";
  std::signal(SIGINT, signalHandler);

  // Create new session for server
  session *new_session = new session(io_service_, request_manager_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&server::handle_accept, this, new_session,
                                     boost::asio::placeholders::error));
}

void server::handle_accept(session *new_session,
                           const boost::system::error_code &error) {
  BOOST_LOG_NAMED_SCOPE("Handle Accept")
  BOOST_LOG_TRIVIAL(info)
      << "New connection request received, starting to accept it";
  if (!error) {
    new_session->start();

    // Get the IP address of the client when accepting
    tcp::socket &socket = new_session->socket();
    asio::ip::tcp::endpoint remoteEndpoint = socket.remote_endpoint();
    asio::ip::address ipAddress = remoteEndpoint.address();
    BOOST_LOG_TRIVIAL(info)
        << "Request is coming from ip: " << ipAddress.to_string();

  } else {
    delete new_session;
  }

  // Get ready to accept a new connection
  start_accept();
}
