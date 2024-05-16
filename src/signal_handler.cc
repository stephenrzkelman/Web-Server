#include "signal_handler.h"
#include <boost/log/trivial.hpp>

// Signal handler function
void signalHandler(int signal) {
  // Log the signal
  BOOST_LOG_TRIVIAL(info) << "Received termination signal (SIGINT)";

  // Terminate the program
  exit(signal);
}
