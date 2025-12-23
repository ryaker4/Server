#include "auth_handler.h"
#include "network_utils.h"
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/secblock.h>
#include <cryptopp/misc.h>
#include <stdexcept>
#include <cstring>

AuthHandler::AuthHandler(Logger& logger, AuthDB& authDb) 
    : logger_(logger), authDb_(authDb) {}

bool AuthHandler::authenticate(int client_fd, std::string& out_login) {
    const size_t BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    
    ssize_t total_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if(total_read <= 0) {
        logger_.error("Failed to read authentication data");
        return false;
    }
    
    buffer[total_read] = '\0';
    std::string auth_data(buffer, total_read);
    
    logger_.info("=== AUTHENTICATION START ===");
    logger_.info("Received data: " + std::to_string(total_read) + " bytes");
    
    // Парсинг данных аутентификации
    std::string login, salt_hex, hash_hex;
    if(!parseAuthData(auth_data, login, salt_hex, hash_hex)) {
        sendResponse(client_fd, false);
        return false;
    }
    
    // Поиск пароля в базе данных
    std::string password;
    if(!authDb_.findPassword(login, password)) {
        logger_.error("Login not found: '" + login + "'");
        sendResponse(client_fd, false);
        return false;
    }
    
    // Проверка хэша
    if(!verifyHash(login, password, salt_hex, hash_hex)) {
        logger_.error("Hash verification failed for login: '" + login + "'");
        sendResponse(client_fd, false);
        return false;
    }
    
    logger_.info("Authentication successful for: '" + login + "'");
    out_login = login;
    return sendResponse(client_fd, true);
}

bool AuthHandler::parseAuthData(const std::string& data, std::string& login, 
                               std::string& salt_hex, std::string& hash_hex) {
    // Проверяем, что строка содержит хотя бы 72 символа
    if(data.length() < 72) {
        logger_.error("Auth data too short: " + std::to_string(data.length()) + " chars");
        return false;
    }
    
    // Разделяем: все кроме последних 72 символов - логин
    login = data.substr(0, data.length() - 72);
    
    // Последние 72 символа - hex данные
    std::string hex_part = data.substr(data.length() - 72);
    
    // Проверяем, что hex_part действительно hex
    if(!NetworkUtils::isValidHex(hex_part)) {
        logger_.error("Last 72 chars are not valid hex: " + hex_part);
        return false;
    }
    
    // Первые 16 символов hex_part - соль
    // Остальные 56 - хэш
    salt_hex = hex_part.substr(0, 16);
    hash_hex = hex_part.substr(16, 56);
    
    logger_.info("Parsed - Login: '" + login + 
                "', Salt: " + salt_hex + 
                ", Hash: " + hash_hex.substr(0, 16) + "...");
    
    return true;
}

std::string AuthHandler::computeSHA224(const std::string& data) {
    using namespace CryptoPP;
    
    SHA224 sha224;
    SecByteBlock hash(sha224.DigestSize());
    
    sha224.Update((const unsigned char*)data.data(), data.size());
    sha224.Final(hash.data());
    
    return NetworkUtils::bytesToHex(hash.data(), hash.size());
}

bool AuthHandler::verifyHash(const std::string& login, const std::string& password,
                           const std::string& salt_hex, const std::string& client_hash_hex) {
    logger_.info("=== HASH VERIFICATION ===");
    
    // Готовим данные для хэширования: salt_hex + password
    std::string data_to_hash = salt_hex + password;
    logger_.info("Data to hash (SALT||PASSWORD): " + data_to_hash);
    
    // Вычисляем хэш на стороне сервера
    std::string server_hash_hex = computeSHA224(data_to_hash);
    logger_.info("Server hash: " + server_hash_hex);
    logger_.info("Client hash: " + client_hash_hex);
    
    // Сравниваем хэши побайтово
    unsigned char client_hash[28];
    if(!NetworkUtils::hexToBytes(client_hash_hex, client_hash, 28)) {
        logger_.error("Failed to convert client hash from hex");
        return false;
    }
    
    unsigned char server_hash[28];
    if(!NetworkUtils::hexToBytes(server_hash_hex, server_hash, 28)) {
        logger_.error("Failed to convert server hash from hex");
        return false;
    }
    
    return CryptoPP::VerifyBufsEqual(server_hash, client_hash, 28);
}

bool AuthHandler::sendResponse(int client_fd, bool success) {
    const char* response = success ? "OK" : "ERR";
    size_t len = strlen(response);
    
    if(send(client_fd, response, len, 0) != (ssize_t)len) {
        logger_.error("Failed to send auth response");
        return false;
    }
    
    logger_.info("Sent response: " + std::string(response));
    return success;
}