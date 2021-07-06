#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/dup_filter_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Logger
{
    std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_rot;
    std::shared_ptr<spdlog::sinks::dup_filter_sink_mt> m_dup_rot;
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_console;
    std::shared_ptr<spdlog::sinks::dup_filter_sink_mt> m_dup_console;
    std::shared_ptr<spdlog::sinks::dist_sink_mt> m_dist_sink;
    std::shared_ptr<spdlog::logger> m_loggerPtr;

public:
    Logger(const std::string &loggername, const std::string &fname);
    template <typename... Args>
    void info(Args &&...args);
    inline std::shared_ptr<spdlog::logger> get_loggerPtr() { return m_loggerPtr; };
};

Logger::Logger(const std::string &loggername, const std::string &fname)
{
    m_rot = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fname, 1024 * 1024, 5, false);
    m_dup_rot = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
    m_dup_rot->add_sink(m_rot);
    m_console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    m_dup_console = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
    m_dup_console->add_sink(m_console);

    m_dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
    m_dist_sink->add_sink(m_dup_rot);
    m_dist_sink->add_sink(m_dup_console);
    m_loggerPtr = std::make_shared<spdlog::logger>(loggername, m_dist_sink);
}
template <typename... Args>
void Logger::info(Args &&...args)
{
    m_loggerPtr->info(std::forward<Args>(args)...);
}

class Wrapper
{

public:
    static std::shared_ptr<spdlog::logger> spdlogptr;
    static std::shared_ptr<Logger> logptr;
    static void Init(const std::string &loggername, const std::string &fname);

    template <typename... Args>
    static void info(Args &&...args)
    {
        spdlogptr->info(args...);
    }
};

std::shared_ptr<Logger> Wrapper::logptr;
std::shared_ptr<spdlog::logger> Wrapper::spdlogptr;
void Wrapper::Init(const std::string &loggername, const std::string &fname)
{
    Wrapper::logptr = std::make_shared<Logger>(loggername, fname);
    Wrapper::spdlogptr = Wrapper::logptr->get_loggerPtr();
}
int main()
{
    Wrapper::Init("toplogger", "flog.log");
    Wrapper::spdlogptr->info("sss{}", 1);
    Wrapper::logptr->info("HI{}\n", 15);
    Wrapper::info("test{}", 1);
    return 0;
}
