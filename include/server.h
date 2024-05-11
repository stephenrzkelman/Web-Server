#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include "session.h"

class server {
public:
    // Constructor for the server class.
    // Initializes a new server object with the given IO service and port.
    // The server starts accepting incoming connections upon construction.
    // Parameters:
    //   io_service: Reference to the Boost.Asio IO service to be used for asynchronous operations.
    //   request_manager: The object in charge of determining which handler to use and returning the response
    //   port: The port number on which the server will listen for incoming connections.
    server(boost::asio::io_service& io_service, 
        RequestManager& request_manager,
        unsigned short port);
    
    // Public member function to start accepting connections.
    // Creates a new session object for each incoming connection and initiates an asynchronous accept operation.
    void start_accept();

private:
    // Private member function to handle the completion of an asynchronous accept operation.
    // This function is called when an incoming connection is accepted.
    // If no error occurs, it starts the session by calling 'start()' on the session object.
    // If an error occurs, it cleans up the session object and continues accepting connections.
    // Parameters:
    //   new_session: Pointer to the session object representing the newly accepted connection.
    //   error: The error code associated with the completion of the accept operation.
    void handle_accept(session* new_session, const boost::system::error_code& error);

    // Member variables
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<RequestManager> request_manager_;
};

#endif // SERVER_H