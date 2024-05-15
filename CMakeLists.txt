cmake_minimum_required(VERSION 3.10.0)
project(fortnite-gamers)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Add server, session and helper libraries
add_library(session_lib src/session.cc)
add_library(server_lib src/server.cc)
add_library(handler_lib src/handlers/request_handler.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(location_data_lib src/location_data.cc)
target_compile_features(config_parser_lib PUBLIC cxx_std_20)
add_library(logging_lib src/logging.cc)
add_library(file_reader_lib src/file_reader.cc)
add_library(signal_lib src/signal_handler.cc)

# Add handler libraries
add_library(static_handler_lib OBJECT src/handlers/static_handler.cc)
add_library(echo_handler_lib OBJECT src/handlers/echo_handler.cc)
add_library(error_handler_lib OBJECT src/handlers/error_handler.cc)

# Add necessary links for server, session and helpers
add_library(manager_lib src/request_manager.cc)
target_compile_features(manager_lib PUBLIC cxx_std_20)
target_link_libraries(server_lib session_lib signal_lib)
target_link_libraries(manager_lib handler_lib location_data_lib Boost::log_setup Boost::log)
target_link_libraries(session_lib manager_lib)
target_link_libraries(file_reader_lib Boost::system Boost::filesystem)
target_link_libraries(
    config_parser_lib 
    location_data_lib 
    static_handler_lib
    echo_handler_lib
    Boost::log_setup Boost::log
)

# Add necessary links for handlers
target_link_libraries(echo_handler_lib handler_lib Boost::log)
target_link_libraries(static_handler_lib handler_lib file_reader_lib Boost::filesystem Boost::log_setup Boost::log)
target_link_libraries(error_handler_lib handler_lib Boost::log_setup Boost::log)

# add server executable
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

# Update test executable name, srcs, and deps
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(
    config_parser_test 
    config_parser_lib 
    static_handler_lib
    echo_handler_lib
    gtest_main
)
add_executable(echo_handler_test tests/echo_handler_test.cc)
target_link_libraries(echo_handler_test echo_handler_lib gtest_main)
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_lib file_reader_lib gtest_main)
add_executable(error_handler_test tests/error_handler_test.cc)
target_link_libraries(error_handler_test error_handler_lib gtest_main)
add_executable(manager_test tests/request_manager_test.cc)
target_link_libraries(
    manager_test 
    manager_lib 
    config_parser_lib
    error_handler_lib
    echo_handler_lib
    static_handler_lib
    handler_lib
    file_reader_lib 
    gtest_main
)
add_executable(static_handler_test tests/static_handler_test.cc)
target_link_libraries(static_handler_test static_handler_lib gtest_main)

# Update with test binary
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(manager_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(static_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(error_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)


# Add integration test
find_package(Python REQUIRED)
add_test(NAME integ_test 
        COMMAND python3 ../tests/integration_tests.py ../server_config ../build/bin/server)

# Update with target/test targets
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
    TARGETS 
        handler_lib 
        config_parser_lib 
        session_lib server_lib 
        static_handler_lib
        echo_handler_lib 
        error_handler_lib
        file_reader_lib
        manager_lib  
        server_lib  
        location_data_lib
    TESTS 
        config_parser_test 
        echo_handler_test 
        manager_test
        static_handler_test
        server_test
        error_handler_test
)
