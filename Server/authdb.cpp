#include "authdb.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

void AuthDB::loadFromFile(const std::string& filename) {
    db.clear();
    std::ifstream ifs(filename);
    if (!ifs.is_open()) throw std::runtime_error("Cannot open clients DB: " + filename);
    std::string line;
    while (std::getline(ifs, line)) {
        if(line.empty()) continue;
        std::istringstream ss(line);
        std::string login, pass;
        if (std::getline(ss, login, ':') && std::getline(ss, pass)) {
            db[login] = pass;
        }
    }
}

bool AuthDB::findPassword(const std::string& login, std::string& outPassword) const {
    auto it = db.find(login);
    if (it == db.end()) return false;
    outPassword = it->second;
    return true;
}
