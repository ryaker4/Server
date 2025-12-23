#ifndef SERVER_PARAMS_H
#define SERVER_PARAMS_H

#include <string>

/**
 * @struct ServerParams
 * @brief Структура параметров конфигурации сервера
 */
struct ServerParams {
    int port = 33333;                     ///< Порт для прослушивания
    std::string address = "127.0.0.1";    ///< IP-адрес для привязки
    std::string logFile = "server.log";   ///< Путь к файлу логов
    std::string clientsDbFile = "clients.db"; ///< Путь к файлу базы данных клиентов
    bool help = false;                    ///< Флаг запроса справки
};

#endif