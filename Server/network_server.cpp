#include "network_server.h"
#include "auth_handler.h"
#include "vector_handler.h"
#include "network_utils.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

// ====================================================================
// Конструктор / деструктор
// ====================================================================
NetworkServer::NetworkServer(const ServerParams& p, Logger& lg, AuthDB& a)
    : params(p)
    , logger(lg)
    , auth(a)
{
}

NetworkServer::~NetworkServer()
{
    if(listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
}

// ====================================================================
// Остановка сервера
// ====================================================================
void NetworkServer::requestStop()
{
    running = false;
    if(listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
}

bool NetworkServer::isRunning() const { return running.load(); }

// ====================================================================
// Создание сокета
// ====================================================================
void NetworkServer::createSocket()
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1)
        throw std::system_error(errno, std::generic_category(), "socket");

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(params.port));
    addr.sin_addr.s_addr = inet_addr(params.address.c_str());

    if(bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::system_error(errno, std::generic_category(), "bind");

    if(listen(listen_fd, 5) == -1)
        throw std::system_error(errno, std::generic_category(), "listen");

    logger.info("Listening on " + params.address + ":" + std::to_string(params.port));
    std::cout<< "Слушаем " << params.address << ":" << std::to_string(params.port) << std::endl;
}

// ====================================================================
// Главный цикл работы сервера
// ====================================================================
void NetworkServer::run()
{
    createSocket();

    while(running) {
        logger.info("Waiting for client...");
        std::cout << "Ожидание клиента.." << std::endl;

        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);

        int client_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);

        if(!running)
            break;

        if(client_fd == -1) {
            logger.error("accept failed");
            continue;
        }

        std::string client_info = NetworkUtils::sockaddrToString(cli_addr);
        logger.info("Accepted connection from " + client_info);
        std::cout << "Принято соединение от: " << client_info << std::endl;

        try {
            serveClient(client_fd);
        } catch(const std::exception& e) {
            logger.error(std::string("Session error: ") + e.what());
        }

        close(client_fd);
        logger.info("Client disconnected: " + client_info);
        std::cout << "Клиент отключен: " << client_info << std::endl;
    }

    logger.info("Server loop exited.");
}

// ====================================================================
// Обслуживание одного клиента
// ====================================================================
void NetworkServer::serveClient(int client_fd)
{
    // Этап 1: Аутентификация
    AuthHandler authHandler(logger, auth);
    std::string login;
    
    if(!authHandler.authenticate(client_fd, login)) {
        logger.warning("Authentication failed, closing connection");
        return;
    }
    
    // Этап 2: Обработка векторов
    VectorHandler vectorHandler(logger);
    vectorHandler.process(client_fd, login);
}