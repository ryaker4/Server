#include "logger.h"
#include <chrono>
#include <ctime>

/**
 * @brief Создает логгер с привязкой к файлу
 * @details Открывает файл для записи в режиме добавления (append).
 *          Если файл не существует, он будет создан.
 *          Если файл существует, новые записи будут добавляться в конец.
 * @param filename Путь к файлу лога
 * @throw std::runtime_error если файл не может быть открыт для записи
 * @note Формат открытия файла: std::ios::app (добавление в конец)
 */
Logger::Logger(const std::string& filename) {
    ofs.open(filename, std::ios::app);
    if(!ofs.is_open()) {
        throw std::runtime_error("Cannot open log file: " + filename);
    }
}

/**
 * @brief Деструктор логгера
 * @details Закрывает файловый поток, если он был открыт.
 *          Гарантирует корректное освобождение ресурсов.
 */
Logger::~Logger() {
    if(ofs.is_open()) ofs.close();
}

/**
 * @brief Основной метод записи сообщения в лог
 * @details Формат записи: [YYYY-MM-DD HH:MM:SS] LEVEL: сообщение
 *          Время берется с точностью до секунды.
 *          Метод потокобезопасен благодаря использованию мьютекса.
 * @param level Уровень логирования (INFO, ERROR, WARNING)
 * @param msg Текст сообщения для записи
 * @note Использует локальную блокировку мьютекса для предотвращения
 *       пересечения записей от разных потоков
 */
void Logger::write(const std::string& level, const std::string& msg) {
    std::lock_guard<std::mutex> g(mtx);
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    ofs << "[" << std::string(std::ctime(&t)).substr(0,24) << "] "
        << level << ": " << msg << std::endl;
}

/**
 * @brief Записывает информационное сообщение в лог
 * @param msg Текст информационного сообщения
 * @note Уровень: INFO
 */
void Logger::info(const std::string& msg) { write("INFO", msg); }

/**
 * @brief Записывает сообщение об ошибке в лог
 * @param msg Текст сообщения об ошибке
 * @note Уровень: ERROR
 */
void Logger::error(const std::string& msg) { write("ERROR", msg); }

/**
 * @brief Записывает предупреждающее сообщение в лог
 * @param msg Текст предупреждающего сообщения
 * @note Уровень: WARNING
 */
void Logger::warning(const std::string& msg) { write("WARNING", msg); }