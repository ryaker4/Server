#include "network_utils.h"
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <algorithm>

namespace NetworkUtils {

ssize_t recvAll(int fd, void* buf, size_t len) {
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

std::string bytesToHex(const unsigned char* data, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::uppercase;
    for(size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

bool hexToBytes(const std::string& hex, unsigned char* output, size_t output_len) {
    if(hex.length() != output_len * 2) {
        return false;
    }
    
    auto hexCharToByte = [](char c) -> unsigned char {
        if(c >= '0' && c <= '9')
            return c - '0';
        if(c >= 'a' && c <= 'f')
            return 10 + (c - 'a');
        if(c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
        return 0xFF;
    };
    
    for(size_t i = 0; i < output_len; i++) {
        unsigned char high = hexCharToByte(hex[i * 2]);
        unsigned char low = hexCharToByte(hex[i * 2 + 1]);
        if(high == 0xFF || low == 0xFF) {
            return false;
        }
        output[i] = (high << 4) | low;
    }
    
    return true;
}

std::string sockaddrToString(const sockaddr_in& addr) {
    char ipbuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipbuf, sizeof(ipbuf));
    return std::string(ipbuf) + ":" + std::to_string(ntohs(addr.sin_port));
}

bool isValidHex(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](char c) {
        return (c >= '0' && c <= '9') || 
               (c >= 'a' && c <= 'f') || 
               (c >= 'A' && c <= 'F');
    });
}

uint32_t readNetworkUint32(int fd) {
    uint32_t value;
    if(recvAll(fd, &value, 4) != 4) {
        throw std::runtime_error("Failed to read uint32");
    }
    return value;
}

bool sendNetworkUint32(int fd, uint32_t value) {
    return send(fd, &value, 4, 0) == 4;
}

} // namespace NetworkUtils