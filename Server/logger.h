#pragma once
#include <string>
#include <mutex>
#include <fstream>

class Logger {
public:
    explicit Logger(const std::string& filename);
    ~Logger();

    void info(const std::string& msg);
    void error(const std::string& msg);
    void warning(const std::string& msg);

private:
    std::mutex mtx;
    std::ofstream ofs;
    void write(const std::string& level, const std::string& msg);
};
