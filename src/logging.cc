#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void init_logging() {
    // Setup file logging
    logging::add_file_log
    (
        keywords::file_name = "logging/fortnite-gamers_%N.log",                                  /*< TODO: file name pattern >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                     /*< rotate files every 10 MiB... >*/
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),   /*< ...or at midnight >*/
        keywords::format = "[%TimeStamp%]: %Message%",                                  /*< log record format >*/
        keywords::auto_flush = true 
    );

    // Setup console logging 
    logging::add_console_log(
        std::cout, 
        keywords::auto_flush = true
    );

    // TODO: figure out what this does LOL. Was in the tutorial 
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
    logging::add_common_attributes();
}