#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H
#include "server_params.h"

#include <string>
#include <atomic>
#include <cstdint>
#include <vector>

class Logger;
class AuthDB;



class NetworkServer {
public:
    NetworkServer(const ServerParams& p, Logger& lg, AuthDB& a);
    ~NetworkServer();

    void run();

    // Остановка сервера извне (например, из обработчика SIGINT)
    void requestStop();
    bool isRunning() const;

private:
    void createSocket();
    void serveClient(int client_fd);

    int listen_fd = -1;

    ServerParams params;
    Logger& logger;
    AuthDB& auth;

    std::atomic<bool> running { true };
};

#endif
