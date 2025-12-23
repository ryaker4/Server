#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "server_params.h"
#include <atomic>
#include <cstdint>

class Logger;
class AuthDB;

/**
 * @class NetworkServer
 * @brief Основной класс сетевого сервера
 * @details Осуществляет прием подключений, аутентификацию клиентов и обработку их запросов
 */
class NetworkServer {
public:
    /**
     * @brief Конструктор сервера
     * @param p Параметры конфигурации сервера
     * @param lg Логгер для записи событий
     * @param a База данных аутентификации
     */
    NetworkServer(const ServerParams& p, Logger& lg, AuthDB& a);
    
    /**
     * @brief Деструктор сервера
     */
    ~NetworkServer();

    /**
     * @brief Запуск основного цикла сервера
     */
    void run();
    
    /**
     * @brief Запрос остановки сервера
     */
    void requestStop();
    
    /**
     * @brief Проверка состояния работы сервера
     * @return true если сервер работает, false в противном случае
     */
    bool isRunning() const;

private:
    /**
     * @brief Создание слушающего сокета
     * @throw std::system_error при ошибках создания сокета
     */
    void createSocket();
    
    /**
     * @brief Обслуживание подключенного клиента
     * @param client_fd Файловый дескриптор клиентского сокета
     */
    void serveClient(int client_fd);

    int listen_fd = -1;              ///< Файловый дескриптор слушающего сокета
    ServerParams params;             ///< Параметры конфигурации сервера
    Logger& logger;                  ///< Ссылка на объект логгера
    AuthDB& auth;                    ///< Ссылка на базу данных аутентификации
    std::atomic<bool> running{true}; ///< Флаг работы сервера
};

#endif