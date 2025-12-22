#include "serverInterface.h"
#include "logger.h"
#include "authdb.h"
#include "network_server.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ServerInterface iface;
        // Если запущено без аргументов — показать help и выйти
        if (argc == 1) {
            std::cout << iface.getDescription();
            return 0;
        }

        if (!iface.parse(argc, argv)) {
            std::cout << iface.getDescription();
            return 0;
        }
        auto params = iface.getParams();

        // logger
        Logger logger(params.logFile);
        logger.info("Server starting");
        std::cout << "Сервер запущен.." << std::endl;

        // load auth DB
        AuthDB auth;
        auth.loadFromFile(params.clientsDbFile);
        logger.info("Loaded clients DB: " + params.clientsDbFile);
        std::cout << "Загружена БД клиентов: " << params.clientsDbFile << std::endl;

        // create and run server (sequential)
        NetworkServer server(params, logger, auth);
        server.run();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
}
