#pragma once
#include <string>
#include <unordered_map>

class AuthDB {
public:
    // загружает файл вида "login:password" по строкам
    AuthDB() = default;
    void loadFromFile(const std::string& filename);
    // возвращает true и пароль через out если найден
    bool findPassword(const std::string& login, std::string& outPassword) const;

private:
    std::unordered_map<std::string,std::string> db;
};
