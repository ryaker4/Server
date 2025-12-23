#ifndef AUTH_HANDLER_H
#define AUTH_HANDLER_H

#include <string>
#include "logger.h"
#include "authdb.h"

/**
 * @class AuthHandler
 * @brief Класс для обработки аутентификации клиентов
 */
class AuthHandler {
public:
    /**
     * @brief Конструктор обработчика аутентификации
     * @param logger Логгер для записи событий
     * @param authDb База данных аутентификации
     */
    AuthHandler(Logger& logger, AuthDB& authDb);
    
    /**
     * @brief Основной метод аутентификации
     * @param client_fd Файловый дескриптор клиентского сокета
     * @param out_login Ссылка на строку для записи аутентифицированного логина
     * @return true если аутентификация успешна, false в противном случае
     */
    bool authenticate(int client_fd, std::string& out_login);
    
    /**
     * @brief Парсинг данных аутентификации
     * @param data Сырые данные от клиента
     * @param login Ссылка на строку для записи логина
     * @param salt_hex Ссылка на строку для записи соли в hex
     * @param hash_hex Ссылка на строку для записи хэша в hex
     * @return true если парсинг успешен, false в противном случае
     */
    bool parseAuthData(const std::string& data, std::string& login, 
                      std::string& salt_hex, std::string& hash_hex);
    
    /**
     * @brief Проверка хэша пароля
     * @param login Логин пользователя
     * @param password Пароль из базы данных
     * @param salt_hex Соль в hex формате
     * @param client_hash_hex Хэш от клиента в hex формате
     * @return true если хэши совпадают, false в противном случае
     */
    bool verifyHash(const std::string& login, const std::string& password,
                   const std::string& salt_hex, const std::string& client_hash_hex);
    
private:
    Logger& logger_;   ///< Ссылка на объект логгера
    AuthDB& authDb_;   ///< Ссылка на базу данных аутентификации
    
    /**
     * @brief Отправка ответа клиенту
     * @param client_fd Файловый дескриптор клиентского сокета
     * @param success Результат аутентификации
     * @return true если отправка успешна, false в противном случае
     */
    bool sendResponse(int client_fd, bool success);
    
    /**
     * @brief Вычисление SHA224 хэша
     * @param data Данные для хэширования
     * @return Хэш в виде hex строки
     */
    std::string computeSHA224(const std::string& data);
};

#endif