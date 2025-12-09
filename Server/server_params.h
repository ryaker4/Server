#ifndef SERVER_PARAMS_H
#define SERVER_PARAMS_H

#include <string>

struct ServerParams {
    int port = 33333;
    std::string address = "121.0.0.1";
    std::string logFile = "server.log";
    std::string clientsDbFile = "clients.db";
    bool help = false;
};

#endif
