#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>

namespace NetworkUtils {
    // Гарантированное чтение всех данных
    ssize_t recvAll(int fd, void* buf, size_t len);
    
    // Преобразование байт в hex строку
    std::string bytesToHex(const unsigned char* data, size_t length);
    
    // Преобразование hex строки в байты
    bool hexToBytes(const std::string& hex, unsigned char* output, size_t output_len);
    
    // Преобразование sockaddr_in в строку IP:PORT
    std::string sockaddrToString(const sockaddr_in& addr);
    
    // Валидация hex строки
    bool isValidHex(const std::string& str);
    
    // Функции для работы с сетевым порядком байт
    uint32_t readNetworkUint32(int fd);
    bool sendNetworkUint32(int fd, uint32_t value);
}

#endif