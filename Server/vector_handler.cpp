#include "vector_handler.h"
#include "network_utils.h"
#include <stdexcept>
#include <cstring>

/**
 * @brief Создает обработчик векторных запросов
 * @param logger Логгер для записи событий обработки векторов
 */
VectorHandler::VectorHandler(Logger& logger) : logger_(logger) {}

/**
 * @brief Основной метод обработки векторов для аутентифицированного клиента
 * @details Процесс обработки:
 *          1. Чтение количества векторов (uint32_t, сетевой порядок байт)
 *          2. Для каждого вектора:
 *             a. Чтение размера вектора
 *             b. Чтение данных вектора
 *             c. Обработка вектора (суммирование с ограничением)
 *             d. Отправка результата клиенту
 *          3. Логирование прогресса и статистики
 * @param client_fd Файловый дескриптор клиентского сокета
 * @param login Логин аутентифицированного пользователя (для логирования)
 * @throw std::runtime_error при ошибках чтения/записи или неверных данных
 * @note Максимальное количество векторов: 100,000
 * @note Максимальный размер одного вектора: 10,000,000 элементов
 * @note Каждые 10 векторов логируется прогресс обработки
 */
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

/**
 * @brief Читает количество векторов для обработки
 * @param client_fd Файловый дескриптор клиентского сокета
 * @return Количество векторов для обработки
 * @throw std::runtime_error при ошибке чтения или неверном значении
 * @note Значение читается в сетевом порядке байт и проверяется на корректность
 */
uint32_t VectorHandler::readVectorCount(int client_fd) {
    uint32_t count = NetworkUtils::readNetworkUint32(client_fd);
    if(!validateVectorCount(count)) {
        throw std::runtime_error("Invalid vector count: " + std::to_string(count));
    }
    return count;
}

/**
 * @brief Проверяет корректность количества векторов
 * @param count Проверяемое количество векторов
 * @return true если количество допустимо, false в противном случае
 * @note Допустимый диапазон: 1..100,000
 */
bool VectorHandler::validateVectorCount(uint32_t count) {
    return count > 0 && count <= 100000;
}

/**
 * @brief Проверяет корректность размера вектора
 * @param size Проверяемый размер вектора
 * @return true если размер допустим, false в противном случае
 * @note Допустимый диапазон: 1..10,000,000
 */
bool VectorHandler::validateVectorSize(uint32_t size) {
    return size > 0 && size <= 10000000;
}

/**
 * @brief Читает один вектор из сокета
 * @details Читает размер вектора (uint32_t), затем читает данные вектора.
 *          Размер вектора проверяется на корректность.
 * @param client_fd Файловый дескриптор клиентского сокета
 * @param vector Ссылка на вектор для записи данных
 * @return true если чтение успешно, false в противном случае
 * @note Использует recvAll() для гарантированного чтения всех данных
 * @post Если возвращено true, vector содержит прочитанные данные
 */
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

/**
 * @brief Обрабатывает один вектор (суммирование с ограничением)
 * @param vector Вектор для обработки
 * @return Результат обработки (сумма элементов с ограничением)
 * @note Использует VectorProcessor::sumClamp() для вычисления
 */
int32_t VectorHandler::processVector(const std::vector<uint32_t>& vector) {
    return VectorProcessor::sumClamp(vector);
}

/**
 * @brief Отправляет результат обработки вектора клиенту
 * @param client_fd Файловый дескриптор клиентского сокета
 * @param result Результат обработки вектора
 * @return true если отправка успешна, false в противном случае
 * @note Результат отправляется в сетевом порядке байт
 */
bool VectorHandler::sendResult(int client_fd, int32_t result) {
    return NetworkUtils::sendNetworkUint32(client_fd, result);
}