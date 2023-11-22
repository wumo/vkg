#include "syntactic_sugar.hpp"

#include <utility>

namespace vkg {

std::time_t currentTime() { return std::time(nullptr); }

double measure(const std::function<void()> &block) {
    auto start = std::chrono::high_resolution_clock::now();
    block();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    return ms.count();
}

void printMeasure(const std::function<void()> &block, std::string msg) {
    auto duration = measure(block);
    println(msg, "duration: ", duration, "ms");
}

//void signal_handler(int signum) {
//  ::signal(signum, SIG_DFL);
//  using namespace std::literals::chrono_literals;
//  auto a = 42s;
//  auto time = std::chrono::system_clock::now();
//  auto s = fmt::format("./dump-{}.txt", time.time_since_epoch().count());
//  boost::stacktrace::safe_dump_to(s.c_str());
//  ::raise(SIGABRT);
//}

bool endWith(std::string const &fullString, std::string const &ending) {
    if(fullString.length() >= ending.length())
        return 0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending);
    else
        return false;
}
}
