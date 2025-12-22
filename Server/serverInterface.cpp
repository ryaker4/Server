#include "serverInterface.h"
#include <boost/program_options.hpp>
#include <sstream>
#include <iostream>
#include <memory>

namespace po = boost::program_options;

struct ServerInterface::Impl {
    po::options_description desc{"Allowed options"};
    ServerParams params;
    po::variables_map vm;

    Impl() {
        desc.add_options()
            ("help,h", "Show help")
            ("port,p", po::value<int>(&params.port)->default_value(33333), "Server port to listen")
            ("address,a", po::value<std::string>(&params.address)->default_value("127.0.0.1"), "Bind address")
            ("log,l", po::value<std::string>(&params.logFile)->default_value("server.log"), "Log file path")
            ("clients-db,d", po::value<std::string>(&params.clientsDbFile)->default_value("clients.db"),
                 "Clients DB file (format: login:password per line)");
    }
};

ServerInterface::ServerInterface() : pimpl(new Impl()) {}
bool ServerInterface::parse(int argc, char** argv) {
    try {
        po::store(po::parse_command_line(argc, argv, pimpl->desc), pimpl->vm);
        if (pimpl->vm.count("help")) {
            pimpl->params.help = true;
            return false;
        }
        po::notify(pimpl->vm);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[CLI] parse error: " << e.what() << std::endl;
        return false;
    }
}
ServerParams ServerInterface::getParams() const { return pimpl->params; }
std::string ServerInterface::getDescription() const {
    std::ostringstream ss; ss << pimpl->desc;
    return ss.str();
}
