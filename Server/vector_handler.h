#ifndef VECTOR_HANDLER_H
#define VECTOR_HANDLER_H

#include <string>
#include <cstdint>
#include "logger.h"
#include "vector_processor.h"

class VectorHandler {
public:
    explicit VectorHandler(Logger& logger);
    
    // Основной метод обработки векторов
    void process(int client_fd, const std::string& login);
    
    // Вспомогательные методы
    bool readVector(int client_fd, std::vector<uint32_t>& vector);
    int32_t processVector(const std::vector<uint32_t>& vector);
    bool sendResult(int client_fd, int32_t result);
    
private:
    Logger& logger_;
    
    // Чтение данных протокола
    uint32_t readVectorCount(int client_fd);
    bool validateVectorCount(uint32_t count);
    bool validateVectorSize(uint32_t size);
};

#endif