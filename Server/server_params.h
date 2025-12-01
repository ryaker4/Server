#ifndef SERVER_PARAMS_H
#define SERVER_PARAMS_H

#include <string>

struct ServerParams {
    int port = 46913;
    std::string address = "0.0.0.0";
    std::string logFile = "server.log";
    std::string clientsDbFile = "clients.db";
    bool help = false;
};

#endif
