#pragma once
#include <string>
#include "server_params.h"

/**
 * @class ServerInterface
 * @brief Класс для обработки параметров командной строки сервера
 * @details Использует паттерн PImpl для сокрытия реализации
 */
class ServerInterface {
public:
    /**
     * @brief Конструктор интерфейса командной строки
     */
    ServerInterface();
    
    /**
     * @brief Парсинг аргументов командной строки
     * @param argc Количество аргументов
     * @param argv Массив аргументов
     * @return true если парсинг успешен, false если требуется показать справку
     */
    bool parse(int argc, char** argv);
    
    /**
     * @brief Получение параметров сервера
     * @return Структура с параметрами конфигурации
     */
    ServerParams getParams() const;
    
    /**
     * @brief Получение описания параметров командной строки
     * @return Строка с описанием всех доступных опций
     */
    std::string getDescription() const;
    
private:
    struct Impl;
    Impl* pimpl; ///< Указатель на реализацию (PImpl)
};