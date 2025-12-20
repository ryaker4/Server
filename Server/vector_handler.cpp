#include "vector_handler.h"
#include "network_utils.h"
#include <stdexcept>
#include <cstring>

VectorHandler::VectorHandler(Logger& logger) : logger_(logger) {}

void VectorHandler::process(int client_fd, const std::string& login) {
    logger_.info("=== VECTOR PROCESSING START ===");
    logger_.info("Processing vectors for: '" + login + "'");
    
    // Чтение количества векторов
    uint32_t vec_count = readVectorCount(client_fd);
    logger_.info("Vector count: " + std::to_string(vec_count));
    
    // Обработка каждого вектора
    size_t total_vectors = 0;
    size_t total_numbers = 0;
    
    for(uint32_t i = 0; i < vec_count; ++i) {
        std::vector<uint32_t> vec;
        if(!readVector(client_fd, vec)) {
            throw std::runtime_error("Failed to read vector " + std::to_string(i));
        }
        
        int32_t result = processVector(vec);
        if(!sendResult(client_fd, result)) {
            throw std::runtime_error("Failed to send result for vector " + std::to_string(i));
        }
        
        total_vectors++;
        total_numbers += vec.size();
        
        // Логирование прогресса
        if((i + 1) % 10 == 0 || (i + 1) == vec_count) {
            logger_.info("Processed " + std::to_string(i + 1) + 
                        "/" + std::to_string(vec_count) + " vectors");
        }
    }
    
    logger_.info("=== VECTOR PROCESSING COMPLETE ===");
    logger_.info("Total: " + std::to_string(total_vectors) + " vectors, " +
                std::to_string(total_numbers) + " numbers for '" + login + "'");
}

uint32_t VectorHandler::readVectorCount(int client_fd) {
    uint32_t count = NetworkUtils::readNetworkUint32(client_fd);
    if(!validateVectorCount(count)) {
        throw std::runtime_error("Invalid vector count: " + std::to_string(count));
    }
    return count;
}

bool VectorHandler::validateVectorCount(uint32_t count) {
    return count > 0 && count <= 100000;
}

bool VectorHandler::validateVectorSize(uint32_t size) {
    return size > 0 && size <= 10000000;
}

bool VectorHandler::readVector(int client_fd, std::vector<uint32_t>& vector) {
    // Чтение размера вектора
    uint32_t size = NetworkUtils::readNetworkUint32(client_fd);
    if(!validateVectorSize(size)) {
        logger_.error("Invalid vector size: " + std::to_string(size));
        return false;
    }
    
    // Чтение данных вектора
    vector.resize(size);
    size_t bytes = size * sizeof(uint32_t);
    
    if(NetworkUtils::recvAll(client_fd, vector.data(), bytes) != (ssize_t)bytes) {
        logger_.error("Failed to read vector data");
        return false;
    }
    
    return true;
}

int32_t VectorHandler::processVector(const std::vector<uint32_t>& vector) {
    return VectorProcessor::sumClamp(vector);
}

bool VectorHandler::sendResult(int client_fd, int32_t result) {
    return NetworkUtils::sendNetworkUint32(client_fd, result);
}