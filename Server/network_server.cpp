#include "network_server.h"
#include "auth_handler.h"
#include "vector_handler.h"
#include "network_utils.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

// ====================================================================
// Конструктор / деструктор
// ====================================================================

/**
 * @brief Создает сетевой сервер с заданными параметрами
 * @param p Параметры конфигурации сервера
 * @param lg Логгер для записи событий
 * @param a База данных аутентификации
 * @note Дескриптор сокета инициализируется значением -1 (невалидный)
 */
NetworkServer::NetworkServer(const ServerParams& p, Logger& lg, AuthDB& a)
    : params(p)
    , logger(lg)
    , auth(a)
{
}

/**
 * @brief Деструктор сервера
 * @details Закрывает слушающий сокет, если он был открыт.
 *          Гарантирует освобождение системных ресурсов.
 */
NetworkServer::~NetworkServer()
{
    if(listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
}

// ====================================================================
// Остановка сервера
// ====================================================================

/**
 * @brief Запрашивает остановку сервера
 * @details Устанавливает флаг running в false и закрывает слушающий сокет.
 *          Это приводит к выходу из основного цикла accept().
 * @note Потокобезопасен благодаря использованию std::atomic<bool>
 */
void NetworkServer::requestStop()
{
    running = false;
    if(listen_fd != -1) {
        close(listen_fd);
        listen_fd = -1;
    }
}

/**
 * @brief Проверяет, работает ли сервер
 * @return true если сервер работает, false в противном случае
 */
bool NetworkServer::isRunning() const { return running.load(); }

// ====================================================================
// Создание сокета
// ====================================================================

/**
 * @brief Создает и настраивает слушающий сокет
 * @details Выполняет последовательность действий:
 *          1. Создание TCP сокета (AF_INET, SOCK_STREAM)
 *          2. Установка опции SO_REUSEADDR для быстрого переиспользования порта
 *          3. Привязка сокета к указанному адресу и порту
 *          4. Перевод сокета в режим прослушивания
 * @throw std::system_error при ошибках создания/настройки сокета
 * @post listen_fd содержит валидный дескриптор слушающего сокета
 * @note Очередь ожидающих соединений установлена в 5 (стандартное значение)
 */
void NetworkServer::createSocket()
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1)
        throw std::system_error(errno, std::generic_category(), "socket");

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(params.port));
    addr.sin_addr.s_addr = inet_addr(params.address.c_str());

    if(bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::system_error(errno, std::generic_category(), "bind");

    if(listen(listen_fd, 5) == -1)
        throw std::system_error(errno, std::generic_category(), "listen");

    logger.info("Listening on " + params.address + ":" + std::to_string(params.port));
    std::cout<< "Слушаем " << params.address << ":" << std::to_string(params.port) << std::endl;
}

// ====================================================================
// Главный цикл работы сервера
// ====================================================================

/**
 * @brief Запускает основной цикл работы сервера
 * @details Алгоритм работы:
 *          1. Создание слушающего сокета
 *          2. Цикл while(running):
 *             a. Ожидание подключения клиента (accept)
 *             b. Принятие соединения
 *             c. Обработка клиента в serveClient()
 *             d. Закрытие клиентского сокета
 *          3. Логирование завершения работы
 * @note Сервер работает в однопоточном (последовательном) режиме
 * @note Цикл прерывается при установке флага running в false
 * @see serveClient()
 */
void NetworkServer::run()
{
    createSocket();

    while(running) {
        logger.info("Waiting for client...");
        std::cout << "Ожидание клиента.." << std::endl;

        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);

        int client_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);

        if(!running)
            break;

        if(client_fd == -1) {
            logger.error("accept failed");
            continue;
        }

        std::string client_info = NetworkUtils::sockaddrToString(cli_addr);
        logger.info("Accepted connection from " + client_info);
        std::cout << "Принято соединение от: " << client_info << std::endl;

        try {
            serveClient(client_fd);
        } catch(const std::exception& e) {
            logger.error(std::string("Session error: ") + e.what());
        }

        close(client_fd);
        logger.info("Client disconnected: " + client_info);
        std::cout << "Клиент отключен: " << client_info << std::endl;
    }

    logger.info("Server loop exited.");
}

// ====================================================================
// Обслуживание одного клиента
// ====================================================================

/**
 * @brief Обслуживает подключенного клиента
 * @details Процесс обслуживания состоит из двух этапов:
 *          1. Аутентификация:
 *             - Создание AuthHandler
 *             - Проверка учетных данных клиента
 *             - Если аутентификация не пройдена - закрытие соединения
 *          2. Обработка векторов:
 *             - Создание VectorHandler
 *             - Чтение и обработка векторных данных
 *             - Отправка результатов клиенту
 * @param client_fd Файловый дескриптор клиентского сокета
 * @throw Может генерировать исключения из AuthHandler и VectorHandler
 * @note Оба этапа выполняются в одном потоке последовательно
 * @note При ошибке на любом этапе соединение закрывается
 */
void NetworkServer::serveClient(int client_fd)
{
    // Этап 1: Аутентификация
    AuthHandler authHandler(logger, auth);
    std::string login;
    
    if(!authHandler.authenticate(client_fd, login)) {
        logger.warning("Authentication failed, closing connection");
        return;
    }
    
    // Этап 2: Обработка векторов
    VectorHandler vectorHandler(logger);
    vectorHandler.process(client_fd, login);
}