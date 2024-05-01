#include "logging.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

// Add attribute keywords
BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)

void init_logging() {

    // Setup the common formatter for all sinks
    logging::formatter fmt = expr::stream
        << "[" << std::setw(6) << std::setfill('0') << line_id << std::setfill(' ') << "] "
        << expr::if_(expr::has_attr(tag_attr))
           [
               expr::stream << "[" << tag_attr << "] "
           ]
        << "[" << logging::expressions::attr<logging::attributes::current_thread_id::value_type>("ThreadID") << "] "
        << "[" << expr::attr<boost::posix_time::ptime>("TimeStamp") << "] "
        << "[" << logging::trivial::severity << "] " 
        << expr::smessage;

    // Setup file logging
    logging::add_file_log
    (
        keywords::file_name = "logging/fortnite-gamers_%N.log",                                 /*< TODO: make file name pattern more descriptive, say with time file is created >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                             /*< rotate files every 10 MiB... >*/
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),           /*< ...or at midnight >*/
        keywords::format = fmt,                                                                 /*< log record format >*/
        keywords::auto_flush = true
    );

    // Setup console logging 
    logging::add_console_log(
        std::cout, 
        keywords::auto_flush = true,
        keywords::format = fmt
    );

    // Only logs with a severity greater than or equal to info will show. Severity levels are as follows in order of severity: trace, debug, info, warning, error, fatal
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );

    // Add attributes to the logs
    logging::add_common_attributes();
}