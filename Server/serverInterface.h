#pragma once
#include <string>
#include "server_params.h"



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
