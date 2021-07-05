#include <spdlog/spdlog.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/dup_filter_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include <thread>

// useful sink from spdplog
// dist_sink that distribute log to different logger
// dup_filter_sink that filter out same log
// rotating_file_sink

int main(int argc, char **argv)
{
    // create a thread safe sink which will keep its file size to a maximum of 5MB and a maximum of 3 rotated files.
    // auto file_logger = spdlog::rotating_logger_mt("file_logger", "logs/mylogfile", 1048576 * 5, 3);
    
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt> ("rot_file.log", 1000*5, 3, false);
    auto dup_filter_rotating_sink = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
    dup_filter_rotating_sink->add_sink(rotating_sink);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto dup_filter_console_sink = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
    dup_filter_console_sink->add_sink(console_sink);
    auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();    
    dist_sink->add_sink(dup_filter_rotating_sink);
    dist_sink->add_sink(dup_filter_console_sink);
    auto logger = std::make_shared<spdlog::logger>("logger",dist_sink);

    auto rotating_err_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt> ("rot_err_file.log", 1000*8, 3, false);
    auto console_err_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto dist_err_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();    
    dist_err_sink->add_sink(rotating_err_sink);
    dist_err_sink->add_sink(console_err_sink);
    auto err_logger = std::make_shared<spdlog::logger>("err_logger",dist_err_sink);

    logger->info("{} - {}",1,2);
    logger->info("{} - {}",1,2);
    logger->info("{} - {}",1,2);
    logger->info("{} - {}",1,2);
    logger->info("{} - {}",1,2);
    logger->info("{} - {}",5,2);
    for(size_t i =0; i<100;++i) logger->info("{} - {}",i,2);
    err_logger->info("{} - {}",1,2);

    return 0;
}