#ifndef AUTH_HANDLER_H
#define AUTH_HANDLER_H

#include <string>
#include "logger.h"
#include "authdb.h"

class AuthHandler {
public:
    AuthHandler(Logger& logger, AuthDB& authDb);
    
    // Основной метод аутентификации
    bool authenticate(int client_fd, std::string& out_login);
    
    // Вспомогательные методы, которые можно тестировать отдельно
    bool parseAuthData(const std::string& data, std::string& login, 
                      std::string& salt_hex, std::string& hash_hex);
    bool verifyHash(const std::string& login, const std::string& password,
                   const std::string& salt_hex, const std::string& client_hash_hex);
    
private:
    Logger& logger_;
    AuthDB& authDb_;
    
    // Отправка ответа клиенту
    bool sendResponse(int client_fd, bool success);
    
    // Вычисление SHA224
    std::string computeSHA224(const std::string& data);
};

#endif