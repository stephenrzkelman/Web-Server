#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/beast.hpp>
#include <gtest/gtest_prod.h>
#include <iostream>
using boost::asio::ip::tcp;

class session {

friend class SessionTest;
FRIEND_TEST(SessionTest, Read);

public:
    // Constructor for the session class.
    // Initializes a new session object with the given IO service for asynchronous operations.
    // Parameters:
    //   io_service: Reference to the Boost.Asio IO service to be used for asynchronous operations.
    session(boost::asio::io_service& io_service);

    // Member function to access the TCP socket associated with the session.
    // Returns:
    //   A reference to the TCP socket used by this session.
    boost::asio::ip::tcp::socket& socket();

    // Member function to start an asynchronous read operation on the socket.
    // This function initiates an asynchronous read operation on the socket to read data into the 'data_' buffer.
    // When the read operation completes, the handle_read function is called.
    void start();

private:
    // Private member function to handle the completion of an asynchronous read operation.
    // This function is called when an asynchronous read operation initiated by 'start()' completes.
    // Parameters:
    //   error: The error code associated with the completion of the read operation.
    //   bytes_transferred: The number of bytes transferred during the read operation.
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

    // Private member function to handle the completion of an asynchronous write operation.
    // This function is called when an asynchronous write operation initiated by 'handle_read()' completes.
    // Parameters:
    //   error: The error code associated with the completion of the write operation.
    void handle_write(const boost::system::error_code& error);

    std::string getResponse();

    // Member variables
    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    std::string responseHeader;
    std::string response;
};

#endif // SESSION_H