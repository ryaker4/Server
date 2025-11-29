#include "logger.h"
#include <chrono>
#include <ctime>

Logger::Logger(const std::string& filename) {
    ofs.open(filename, std::ios::app);
    if(!ofs.is_open()) {
        throw std::runtime_error("Cannot open log file: " + filename);
    }
}

Logger::~Logger() {
    if(ofs.is_open()) ofs.close();
}

void Logger::write(const std::string& level, const std::string& msg) {
    std::lock_guard<std::mutex> g(mtx);
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    ofs << "[" << std::string(std::ctime(&t)).substr(0,24) << "] "
        << level << ": " << msg << std::endl;
}

void Logger::info(const std::string& msg) { write("INFO", msg); }
void Logger::error(const std::string& msg) { write("ERROR", msg); }
