# Contributor Documentation

## Source Code Layout

### Dependency Graph

![Dependency Graph Image](https://lh7-us.googleusercontent.com/docsz/AD_4nXcxzx1V8w1aazewRRXnD16AHFiWX-_khnrIq7j4eMCGhh3tGPSXS2ReK5LRhg55v5j5mEPKUuhhyzRs-yBl32YmAUGdIj2LANMHqA8u5EKq0oAIupxpYjaUgZiBenrmbhKEhBeWjhE8ZZYY2dUDj5xjS1yT?key=dXuRdM3tReV7ipJFJYEsrg)


### Code Flow

The general flow is as follows: a server will begin in the main, and start running a session. The session listens for requests and passes them to the request manager. The request manager then determines what request handler to call and receive a response from. It will follow the route in reverse and send out the response from the session to the client.

    server_main -> server -> session -> request_manager -> request_handler


## How to Build, Test, and Run the code

### Building:

**From the base directory** of the repository, **in the dev environment**, run the following for a clean build:

    rm -rf build
    mkdir build
    cd build
    cmake ..
    make


### Testing

After building, **from the build directory**, **in the dev env**, run:

    make test

This, together with the build step above, can be run **from the base directory**, **in the dev env**, with:

    ./clean_build.sh


### Coverage

To check test coverage, go to the **base directory of the repository** **in the dev env**, and run:

    ./clean_coverage.sh

The coverage results can be found in "build\_coverage", and you can view results in your browser via "build\_coverage/index.html"


### Run

To simulate running code on Cloud Build, navigate to fortnite-gamers and run start\_docker.sh **outside of the cs130 development environment.** Alternatively, follow the instructions in DockerREADME.md.


## Adding a Request Handler

1. Add your newHandler.h file

   a. You need to include "registry.h" along with "request\_handler.h"

   b. Your handler must contain a static function of the form:

        static RequestHandler* Init(std::string path, std::unordered_map<std::string, std::string> args);
    
    c.  Your handler must also contain this static function:
            
        static inline ArgSet expectedArgs = {};

    d. With `NewHandler` being the name of your handler class, declare a constructor for your class of the form:

        NewHandler(std::string path, std::unordered_map<std::string, std::string> args);

    e. Define your handle_request function like below:
    
        http_response handle_request(const http_request& request);
    
    f. Declare all of the handler arguments (if any) as private variables

    g. With `NewHandler` being the name of your handler class, you must also call the following macro after you declare your class:

        REGISTER_HANDLER(NewHandler);


StaticHandler.h example:

    #ifndef STATIC_HANDLER_H
    #define STATIC_HANDLER_H


    #include "registry.h"
    #include "request_handler.h"


    const std::string STATIC_HANDLER_ROOT_ARG = "root";


    class StaticHandler : public RequestHandler {
    public:
        StaticHandler(std::string path, std::unordered_map<std::string, std::string> args);
        http_response handle_request(const http_request& request);
        static RequestHandler* Init(std::string path, std::unordered_map<std::string, std::string> args);
        static inline ArgSet expectedArgs = {STATIC_HANDLER_ROOT_ARG};
    private:
        std::string path_;
        std::string root_;
    };

    REGISTER_HANDLER(StaticHandler);

    #endif // STATIC_HANDLER_H

2. Add your newHandler.cc file

    a. You only need to include the header file for your handler along with any other classes or libraries you may have used

    b. Define the constructor you declared in step 1c and have it initialize all of your handler argument variables by using the string map passed in (key is the argument name and the mapped value is the value assigned to it in the config)

    c. Define the static Init function you declared in step 1b and have it simply return a new object of your handler type

StaticHandler.cc example:

    #include "handlers/static_handler.h"
    #include "file_reader.h"


    StaticHandler::StaticHandler(std::string path, std::unordered_map<std::string,std::string> args)
    :path_(path), root_(args[STATIC_HANDLER_ROOT_ARG]){}


    http_response StaticHandler::handle_request(const http_request& request) {
        ...
    }

    RequestHandler* StaticHandler::Init(std::string path, std::unordered_map<std::string, std::string> args){
        return new StaticHandler(path, args);
    }


3. Modify CMakeLists.txt

    a. Add handler library with add\_library under the comment "#Add server, session, handler, and helper libraries"

        add_library(static_handler_lib OBJECT src/handlers/static_handler.cc)

    b. Use target\_link\_libraries to add any needed includes under the comment "# Add necessary links for server, session, handlers, and helpers"

        target_link_libraries(static_handler_lib handler_lib file_reader_lib Boost::filesystem Boost::log_setup Boost::log)

    c. Link handler library to server under "# add server executable"
    
        add_executable(server src/server_main.cc)
        target_link_libraries(
            server 
            config_parser_lib 
            error_handler_lib
            echo_handler_lib
            static_handler_lib
            session_lib 
            server_lib 
            manager_lib 
            logging_lib 
            Boost::system
        )

    d. Create an executable for the test file with add\_executable and target\_link\_libraries under the comment "# Update test executable name, srcs, and deps"

        add_executable(static_handler_test tests/static_handler_test.cc)
        target_link_libraries(static_handler_test static_handler_lib gtest_main)
