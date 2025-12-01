#include "network_server.h"
#include "logger.h"
#include "authdb.h"
#include "vector_processor.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <vector>

// Crypto++
#include <cryptopp/sha.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/secblock.h>

// ====================================================================
// Вспомогательная функция: гарантированно считать len байт
// ====================================================================
static ssize_t recv_all(int fd, void* buf, size_t len) {
    char* p = static_cast<char*>(buf);
    size_t rem = len;

    while (rem > 0) {
        ssize_t r = recv(fd, p, rem, 0);
        if (r <= 0) return r;
        p += r;
        rem -= r;
    }
    return static_cast<ssize_t>(len);
}

// ====================================================================
// Конструктор / деструктор
// ====================================================================
NetworkServer::NetworkServer(const ServerParams& p, Logger& lg, AuthDB& a)
    : params(p), logger(lg), auth(a) {}

NetworkServer::~NetworkServer() {
    if (listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
}

// ====================================================================
// Остановка сервера
// ====================================================================
void NetworkServer::requestStop() {
    running = false;
    if (listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
}

bool NetworkServer::isRunning() const {
    return running.load();
}

// ====================================================================
// Создание сокета
// ====================================================================
void NetworkServer::createSocket() {
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
        throw std::system_error(errno, std::generic_category(), "socket");

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(params.port));
    addr.sin_addr.s_addr = inet_addr(params.address.c_str());

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::system_error(errno, std::generic_category(), "bind");

    if (listen(listen_fd, 5) == -1)
        throw std::system_error(errno, std::generic_category(), "listen");

    logger.info("Listening on " + params.address + ":" + std::to_string(params.port));
}

// ====================================================================
// Главный цикл работы сервера
// ====================================================================
void NetworkServer::run() {
    createSocket();

    while (running) {
        logger.info("Waiting for client...");

        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);

        int client_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);

        // Если мы получили SIGINT → listen_fd закрыт → accept возвращает -1
        if (!running) break;

        if (client_fd == -1) {
            logger.error("accept failed");
            continue;
        }

        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ipbuf, sizeof(ipbuf));
        logger.info(std::string("Accepted connection from ") +
                    ipbuf + ":" + std::to_string(ntohs(cli_addr.sin_port)));

        try {
            serveClient(client_fd);
        } catch (const std::exception& e) {
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
void NetworkServer::serveClient(int client_fd) {
    // Буфер для приема (с запасом, например 2 КБ)
    std::vector<char> packetBuffer(2048);
    
    // Читаем данные от клиента (обычный recv, не recv_all, т.к. мы не знаем точный размер)
    ssize_t bytes_read = recv(client_fd, packetBuffer.data(), packetBuffer.size(), 0);
    
    // 1. Проверка: получили ли мы хоть что-то
    if (bytes_read <= 0) {
        throw std::runtime_error("Client disconnected or read error");
    }

    // 2. Проверка: минимальный размер пакета
    // Логин (минимум 1 байт) + Соль (16) + Хэш (28) = 45 байт
    if (bytes_read < 45) {
        throw std::runtime_error("Data too short to contain auth info");
    }

    // 3. Вычисляем длину логина
    // Логин — это всё, что идет ПЕРЕД последними 72 байтами
    const size_t SUFFIX_TEXT_SIZE = 16 + 56;
    size_t login_len = bytes_read - SUFFIX_TEXT_SIZE; 
    
    // Лишняя защита от переполнения логина (на всякий случай)
    if (login_len > 1024) {
         throw std::runtime_error("Login too long");
    }

    // 4. Извлекаем данные
    
    // -- Логин --
    std::string login(packetBuffer.data(), login_len);
    logger.info("Login: " + login);

    // -- Соль (16 байт) --
    // Находится сразу после логина
    unsigned char salt[16];
    std::memcpy(salt, packetBuffer.data() + login_len, 16);

    // -- Хэш клиента (56 байт) --
    // Находится после логина и соли
    const size_t SIZE_HASH = 56;
    unsigned char client_hash[SIZE_HASH];
    std::memcpy(client_hash, packetBuffer.data() + login_len + 16, SIZE_HASH);


    // -------- STEP 4: lookup password --------
    std::string password;
    if (!auth.findPassword(login, password)) {
        uint8_t err = 1;
        send(client_fd, &err, 1, 0);
        logger.info("Auth failed: unknown login");
        return;
    }

    // -------- STEP 5: compute SHA224(salt + password) --------
    CryptoPP::SHA224 sha224;
    CryptoPP::SecByteBlock server_hash(SIZE_HASH);

    std::vector<unsigned char> buf(16 + password.size());
    memcpy(buf.data(), salt, 16);
    memcpy(buf.data() + 16, password.data(), password.size());

    sha224.Update(buf.data(), buf.size());
    sha224.Final(server_hash.data());

    // -------- STEP 6: compare hashes --------
    if (memcmp(server_hash.data(), client_hash, SIZE_HASH) != 0) {
        uint8_t err = 1;
        send(client_fd, &err, 1, 0);
        logger.info("Auth failed: wrong password/hash");
        return;
    }

    uint8_t ok = 0;
    send(client_fd, &ok, 1, 0);
    logger.info("Authentication OK");

    // ============================
    // === VECTOR PROCESSING ======
    // ============================

    // A: read number of vectors (uint32)
    uint32_t vec_count_net;
    if (recv_all(client_fd, &vec_count_net, 4) != 4)
        throw std::runtime_error("Failed to read vec_count");

    uint32_t vec_count = ntohl(vec_count_net);
    if (vec_count == 0 || vec_count > 100000)
        throw std::runtime_error("Invalid vec_count");

    logger.info("Client will send " + std::to_string(vec_count) + " vectors");

    // B: process each vector
    for (uint32_t i = 0; i < vec_count; ++i) {

        uint32_t vec_len_net;
        if (recv_all(client_fd, &vec_len_net, 4) != 4)
            throw std::runtime_error("Failed to read vec_len");

        uint32_t vec_len = ntohl(vec_len_net);
        if (vec_len == 0 || vec_len > 10000000)
            throw std::runtime_error("Invalid vec_len");

        std::vector<float> data(vec_len);
        size_t bytes = vec_len * sizeof(float);

        if (recv_all(client_fd, data.data(), bytes) != (ssize_t)bytes)
            throw std::runtime_error("Failed to read vector data");

        int32_t result = VectorProcessor::sumClamp(data);
        int32_t result_net = htonl(result);

        if (send(client_fd, &result_net, sizeof(result_net), 0) != sizeof(result_net))
            throw std::runtime_error("Failed to send result");

        logger.info("Vector " + std::to_string(i+1) + "/" +
                    std::to_string(vec_count) + " sum = " + std::to_string(result));
    }

    logger.info("All vectors processed.");
}
