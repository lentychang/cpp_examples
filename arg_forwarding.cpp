#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/dup_filter_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// template<typename ...Args>
// void info(Args &&...args){
//     spdlog::info(args...);
// }

class Wrapper
{
    static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> rot;
    static std::shared_ptr<spdlog::sinks::dup_filter_sink_mt> dup_rot;
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console;
    static std::shared_ptr<spdlog::sinks::dup_filter_sink_mt> dup_console;
    static std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink;

public:
    static std::shared_ptr<spdlog::logger> logptr;
    static void Init();

    static std::unordered_map<const char *, std::shared_ptr<spdlog::sinks::base_sink<std::mutex>>> sinks;
    template <typename... Args>
    static void info(Args &&...args)
    {
        logptr->info(args...);
    }
};
std::shared_ptr<spdlog::logger> Wrapper::logptr;
void Wrapper::Init()
{
    rot = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("log_filename", 1024 * 1024, 5, false);
    dup_rot = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
    dup_rot->add_sink(rot);
    console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    dup_console = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
    dup_console->add_sink(console);

    dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
    dist_sink->add_sink(dup_rot);
    dist_sink->add_sink(dup_console);

    Wrapper::logptr = std::make_shared<spdlog::logger>("loggername", dist_sink);
}

std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> Wrapper::rot;
std::shared_ptr<spdlog::sinks::dup_filter_sink_mt> Wrapper::dup_rot;
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Wrapper::console;
std::shared_ptr<spdlog::sinks::dup_filter_sink_mt> Wrapper::dup_console;
std::shared_ptr<spdlog::sinks::dist_sink_mt> Wrapper::dist_sink;

int main()
{
    Wrapper::Init();
    Wrapper::logptr->info("sss{}", 1);
    Wrapper::info("test{}", 1);
    return 0;
}
