#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"


// template<typename ...Args>
// void info(Args &&...args){
//     spdlog::info(args...);
// }

class Wrapper{
    public:
    static std::shared_ptr<spdlog::logger> logptr;
    static void Init();
    template<typename ...Args>
    static void info(Args &&...args){
        logptr->info(args...);
    }
};
std::shared_ptr<spdlog::logger> Wrapper::logptr;
void Wrapper::Init(){
    Wrapper::logptr = spdlog::rotating_logger_mt("logger","logfile.log",1000,3,false);
}
int main()
{
    Wrapper::Init();
    Wrapper::logptr->info("sss{}",1);
    Wrapper::info("test{}",1);
    return 0;
}
