#ifndef VECTOR_HANDLER_H
#define VECTOR_HANDLER_H

#include <string>
#include <cstdint>
#include "logger.h"
#include "vector_processor.h"

/**
 * @class VectorHandler
 * @brief Класс для обработки векторных запросов от клиентов
 */
class VectorHandler {
public:
    /**
     * @brief Конструктор обработчика векторов
     * @param logger Логгер для записи событий
     */
    explicit VectorHandler(Logger& logger);
    
    /**
     * @brief Основной метод обработки векторов
     * @param client_fd Файловый дескриптор клиентского сокета
     * @param login Логин аутентифицированного пользователя
     * @throw std::runtime_error при ошибках обработки
     */
    void process(int client_fd, const std::string& login);
    
    /**
     * @brief Чтение вектора из сокета
     * @param client_fd Файловый дескриптор клиентского сокета
     * @param vector Ссылка на вектор для записи данных
     * @return true если чтение успешно, false в противном случае
     */
    bool readVector(int client_fd, std::vector<uint32_t>& vector);
    
    /**
     * @brief Обработка одного вектора
     * @param vector Вектор для обработки
     * @return Результат обработки (сумма с ограничением)
     */
    int32_t processVector(const std::vector<uint32_t>& vector);
    
    /**
     * @brief Отправка результата клиенту
     * @param client_fd Файловый дескриптор клиентского сокета
     * @param result Результат обработки
     * @return true если отправка успешна, false в противном случае
     */
    bool sendResult(int client_fd, int32_t result);
    
private:
    Logger& logger_; ///< Ссылка на объект логгера
    
    /**
     * @brief Чтение количества векторов
     * @param client_fd Файловый дескриптор клиентского сокета
     * @return Количество векторов для обработки
     * @throw std::runtime_error при неверном количестве
     */
    uint32_t readVectorCount(int client_fd);
    
    /**
     * @brief Валидация количества векторов
     * @param count Проверяемое количество
     * @return true если количество допустимо, false в противном случае
     */
    bool validateVectorCount(uint32_t count);
    
    /**
     * @brief Валидация размера вектора
     * @param size Проверяемый размер
     * @return true если размер допустим, false в противном случае
     */
    bool validateVectorSize(uint32_t size);
};

#endif