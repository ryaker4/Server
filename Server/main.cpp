#include "serverInterface.h"
#include "logger.h"
#include "authdb.h"
#include "network_server.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ServerInterface iface;
        if (!iface.parse(argc, argv)) {
            std::cout << iface.getDescription();
            return 0;
        }
        auto params = iface.getParams();

        // logger
        Logger logger(params.logFile);
        logger.info("Server starting");

        // load auth DB
        AuthDB auth;
        auth.loadFromFile(params.clientsDbFile);
        logger.info("Loaded clients DB: " + params.clientsDbFile);

        // create and run server (sequential)
        NetworkServer server(params, logger, auth);
        server.run();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
}
