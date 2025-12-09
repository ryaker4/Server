#include "network_server.h"

#include "authdb.h"
#include "logger.h"
#include "vector_processor.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// Crypto++
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/misc.h>
#include <cryptopp/secblock.h>
#include <cryptopp/sha.h>

// ====================================================================
// Вспомогательные функции
// ====================================================================

// Гарантированно считать len байт
static ssize_t recv_all(int fd, void* buf, size_t len)
{
    char* p = static_cast<char*>(buf);
    size_t rem = len;

    while(rem > 0) {
        ssize_t r = recv(fd, p, rem, 0);
        if(r <= 0)
            return r;
        p += r;
        rem -= r;
    }
    return static_cast<ssize_t>(len);
}

// Функция для конвертации байт в hex строку
static std::string bytesToHex(const unsigned char* data, size_t length)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::uppercase;
    for(size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

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
}

// ====================================================================
// Главный цикл работы сервера
// ====================================================================
void NetworkServer::run()
{
    createSocket();

    while(running) {
        logger.info("Waiting for client...");

        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);

        int client_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);

        if(!running)
            break;

        if(client_fd == -1) {
            logger.error("accept failed");
            continue;
        }

        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ipbuf, sizeof(ipbuf));
        logger.info(std::string("Accepted connection from ") + ipbuf + ":" + std::to_string(ntohs(cli_addr.sin_port)));

        try {
            serveClient(client_fd);
        } catch(const std::exception& e) {
            logger.error(std::string("Session error: ") + e.what());
        }

        close(client_fd);
        logger.info("Client disconnected");
    }

    logger.info("Server loop exited.");
}

// ====================================================================
// Обслуживание одного клиента
// ====================================================================
void NetworkServer::serveClient(int client_fd)
{
    // Читаем все данные аутентификации
    const size_t BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];

    ssize_t total_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if(total_read <= 0) {
        throw std::runtime_error("Failed to read authentication data");
    }

    buffer[total_read] = '\0';
    std::string auth_data(buffer, total_read);

    logger.info("=== НАЧАЛО АУТЕНТИФИКАЦИИ ===");
    logger.info("Получено данных: " + std::to_string(total_read) + " байт");
    logger.info("Сырые данные: " + auth_data);

    // Проверяем, что данных достаточно
    if(auth_data.length() < 72) {
        logger.error("Данных недостаточно: " + std::to_string(auth_data.length()) + " символов (нужно минимум 72)");
        throw std::runtime_error("Auth data too short");
    }

    // Проверяем, что последние 72 символа - это hex
    std::string hex_part = auth_data.substr(auth_data.length() - 72);

    // Валидация hex символов
    bool is_hex = true;
    for(char c : hex_part) {
        if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            is_hex = false;
            break;
        }
    }

    if(!is_hex) {
        logger.error("Последние 72 символа не являются hex: " + hex_part);
        throw std::runtime_error("Last 72 characters are not valid hex");
    }

    // Получение логина сообщения
    std::string login = auth_data.substr(0, auth_data.length() - 72);

    // Разделяем hex часть на соль и хэш
    std::string salt_hex = hex_part.substr(0, 16);  //< 16 hex = 8 байт
    std::string hash_hex = hex_part.substr(16, 56); //< 56 hex = 28 байт

    logger.info("Извлечено:");
    logger.info("  Логин: '" + login + "' (длина: " + std::to_string(login.length()) + ")");
    logger.info("  Соль (hex): " + salt_hex + " (ожидается 8 байт)");
    logger.info("  Хэш клиента (hex): " + hash_hex + " (ожидается 28 байт)");

    // -------- lookup password --------
    std::string password;
    if(!auth.findPassword(login, password)) {
        logger.error("Логин не найден в базе данных: '" + login + "'");
        uint8_t err = 1;
        send(client_fd, &err, 1, 0);
        return;
    }

    // -------- compute SHA224(salt_hex + password) для сравнения --------
    logger.info("=== ОСНОВНОЕ ВЫЧИСЛЕНИЕ ===");

    CryptoPP::SHA224 sha224;
    CryptoPP::SecByteBlock server_hash(28);

    // Создаем строку: salt_hex + password
    std::string data_to_hash = salt_hex + password;

    logger.info("Строка для хэширования (SALT_16||PASSWORD): " + data_to_hash);
    logger.info("Длина строки: " + std::to_string(data_to_hash.length()) + " символов");

    // Вычисляем хэш от текстовой строки
    sha224.Update((const unsigned char*)data_to_hash.data(), data_to_hash.size());
    sha224.Final(server_hash.data());

    std::string server_hash_hex = bytesToHex(server_hash.data(), 28);
    logger.info("Вычисленный хэш сервера (hex): " + server_hash_hex);
    logger.info("Хэш от клиента (hex): " + hash_hex);

    unsigned char client_hash[28]; // 28 байт
    // функция для конвертации двух hex символов в байт
    auto hexCharToByte = [](char c) -> unsigned char {
        if(c >= '0' && c <= '9')
            return c - '0';
        if(c >= 'a' && c <= 'f')
            return 10 + (c - 'a');
        if(c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
        return 0;
    };

    // Конвертируем хэш клиента
    for(int i = 0; i < 28; i++) {
        unsigned char high = hexCharToByte(hash_hex[i * 2]);
        unsigned char low = hexCharToByte(hash_hex[i * 2 + 1]);
        client_hash[i] = (high << 4) | low;
    }

    bool match = CryptoPP::VerifyBufsEqual(server_hash.data(), client_hash, 28); 

    if(!match) {
        logger.error("Хэши не совпадают! Аутентификация не пройдена.");

        // Отправляем ERR и разрываем соединение
        const char* err_msg = "ERR";
        if(send(client_fd, err_msg, strlen(err_msg), 0) != (ssize_t)strlen(err_msg)) {
            logger.error("Не удалось отправить ERR клиенту");
        } else {
            logger.info("Отправлен 'ERR' клиенту");
        }
        close(client_fd);
        return; //< Разрыв соединения
    }

    logger.info("Хэши совпадают! Аутентификация успешна.");

    // Отправляем OK
    const char* ok_msg = "OK";
    if(send(client_fd, ok_msg, strlen(ok_msg), 0) != (ssize_t)strlen(ok_msg)) {
        logger.error("Не удалось отправить OK клиенту");
        close(client_fd);
        return;
    }
    logger.info("Отправлен 'OK' клиенту");

    // ============================
    // === VECTOR PROCESSING ======
    // ============================

    logger.info("=== НАЧАЛО ОБРАБОТКИ ВЕКТОРОВ ===");

    // A: read number of vectors (uint32)
    uint32_t vec_count_net;
    if(recv_all(client_fd, &vec_count_net, 4) != 4)
        throw std::runtime_error("Failed to read vector count");
    logger.info("Количество векторов:" + std::to_string(vec_count_net));

    // uint32_t vec_count = ntohl(vec_count_net);
    uint32_t vec_count = vec_count_net;
    if(vec_count == 0 || vec_count > 100000)
        throw std::runtime_error("Invalid vector count: " + std::to_string(vec_count));

    logger.info("Будет обработано векторов: " + std::to_string(vec_count));

    // B: process each vector
    size_t total_vectors_processed = 0;
    size_t total_floats_processed = 0;

    for(uint32_t i = 0; i < vec_count; ++i) {
        uint32_t vec_len_net;
        if(recv_all(client_fd, &vec_len_net, 4) != 4)
            throw std::runtime_error("Failed to read vector length");

        // uint32_t vec_len = ntohl(vec_len_net);
        uint32_t vec_len = vec_len_net;
        if(vec_len == 0 || vec_len > 10000000)
            throw std::runtime_error("Invalid vector length: " + std::to_string(vec_len));

        // std::vector<float> data(vec_len);
        std::vector<uint32_t> data(vec_len);
        size_t bytes = vec_len * sizeof(uint32_t);

        if(recv_all(client_fd, data.data(), bytes) != (ssize_t)bytes)
            throw std::runtime_error("Failed to read vector data");

        int32_t result = VectorProcessor::sumClamp(data);
        // int32_t result_net = htonl(result);
        int32_t result_net = result;

        if(send(client_fd, &result_net, sizeof(result_net), 0) != sizeof(result_net))
            throw std::runtime_error("Failed to send result");

        total_vectors_processed++;
        total_floats_processed += vec_len;

        // Логируем только каждые 10 векторов или последний
        if((i + 1) % 10 == 0 || (i + 1) == vec_count) {
            logger.info("Обработано " + std::to_string(i + 1) + "/" + std::to_string(vec_count) + " векторов");
        }
    }

    logger.info("=== ЗАВЕРШЕНИЕ ОБРАБОТКИ ===");
    logger.info("Итого для '" + login + "': " + std::to_string(total_vectors_processed) + " векторов, " +
                std::to_string(total_floats_processed) + " чисел");
}
