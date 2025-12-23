#include "authdb.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

/**
 * @brief Загружает базу данных клиентов из текстового файла
 * @details Формат файла: каждая строка содержит пару "логин:пароль"
 *          Пустые строки игнорируются. Если формат строки некорректен
 *          (нет символа ':'), она пропускается.
 * @param filename Путь к файлу базы данных
 * @throw std::runtime_error если файл не может быть открыт
 * @note Предыдущее содержимое базы данных очищается перед загрузкой
 * @warning Файл должен быть в формате UTF-8 или ASCII
 * @post База данных содержит все корректные пары логин-пароль из файла
 */
void AuthDB::loadFromFile(const std::string& filename) {
    db.clear();
    std::ifstream ifs(filename);
    if (!ifs.is_open()) throw std::runtime_error("Cannot open clients DB: " + filename);
    std::string line;
    while (std::getline(ifs, line)) {
        if(line.empty()) continue;
        std::istringstream ss(line);
        std::string login, pass;
        if (std::getline(ss, login, ':') && std::getline(ss, pass)) {
            db[login] = pass;
        }
    }
}

/**
 * @brief Ищет пароль по логину в базе данных
 * @details Выполняет поиск в хэш-таблице по ключу (логину).
 *          Если логин найден, пароль записывается в outPassword.
 * @param login Логин пользователя для поиска
 * @param outPassword Ссылка на строку для записи найденного пароля
 * @return true если логин найден в базе данных,
 *         false если логин отсутствует
 * @note Время поиска: в среднем O(1), в худшем случае O(n)
 * @post Если возвращено true, outPassword содержит пароль пользователя
 *       Если возвращено false, outPassword остается неизменным
 */
bool AuthDB::findPassword(const std::string& login, std::string& outPassword) const {
    auto it = db.find(login);
    if (it == db.end()) return false;
    outPassword = it->second;
    return true;
}