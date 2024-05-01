#include <boost/log/trivial.hpp>
#include "signal_handler.h"

// Signal handler function
void signalHandler(int signal) {
    // Log the signal
    BOOST_LOG_TRIVIAL(info) << "Received termination signal (SIGINT)";
    
    // Terminate the program
    exit(signal);
}
