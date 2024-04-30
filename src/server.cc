#include "server.h"
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

ServerConfig::ServerConfig(
    RequestManager request_manager,
    unsigned short port
): request_manager(request_manager), 
    port(port){}

server::server(boost::asio::io_service& io_service, ServerConfig& config_data)
: io_service_(io_service),
acceptor_(io_service, tcp::endpoint(tcp::v4(), config_data.port))
{
    request_manager_.reset(&config_data.request_manager);
    start_accept();
}

void server::start_accept() {
    BOOST_LOG_TRIVIAL(info) << "Starting Accept";
    session* new_session = new session(io_service_, request_manager_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->start();
    } else {
        delete new_session;
    }

    start_accept();
}
