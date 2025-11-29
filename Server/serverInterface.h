#pragma once
#include <string>

struct ServerParams {
    int port = 46913;
    std::string address = "0.0.0.0";
    std::string logFile = "server.log";
    std::string clientsDbFile = "clients.db";
    bool help = false;
};

class ServerInterface {
public:
    ServerInterface();
    bool parse(int argc, char** argv);
    ServerParams getParams() const;
    std::string getDescription() const;
private:
    struct Impl;
    Impl* pimpl;
};
