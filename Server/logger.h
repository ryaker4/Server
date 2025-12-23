#pragma once
#include <string>
#include <mutex>
#include <fstream>

/**
 * @class Logger
 * @brief Класс для потокобезопасного логирования в файл
 */
class Logger {
public:
    /**
     * @brief Конструктор логгера
     * @param filename Имя файла для записи логов
     * @throw std::runtime_error если файл не может быть открыт
     */
    explicit Logger(const std::string& filename);
    
    /**
     * @brief Деструктор логгера
     */
    ~Logger();

    /**
     * @brief Запись информационного сообщения
     * @param msg Текст сообщения
     */
    void info(const std::string& msg);
    
    /**
     * @brief Запись сообщения об ошибке
     * @param msg Текст сообщения
     */
    void error(const std::string& msg);
    
    /**
     * @brief Запись предупреждающего сообщения
     * @param msg Текст сообщения
     */
    void warning(const std::string& msg);

private:
    std::mutex mtx;               ///< Мьютекс для синхронизации доступа к файлу
    std::ofstream ofs;            ///< Поток для записи в файл
    
    /**
     * @brief Основной метод записи в лог
     * @param level Уровень логирования
     * @param msg Текст сообщения
     */
    void write(const std::string& level, const std::string& msg);
};