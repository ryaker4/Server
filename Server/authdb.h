#pragma once
#include <string>
#include <unordered_map>

/**
 * @class AuthDB
 * @brief Класс для работы с базой данных аутентификации
 */
class AuthDB {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    AuthDB() = default;
    
    /**
     * @brief Загрузка базы данных из файла
     * @param filename Имя файла в формате "login:password" по строкам
     * @throw std::runtime_error если файл не может быть открыт
     */
    void loadFromFile(const std::string& filename);
    
    /**
     * @brief Поиск пароля по логину
     * @param login Логин пользователя
     * @param outPassword Ссылка на строку для записи найденного пароля
     * @return true если логин найден, false в противном случае
     */
    bool findPassword(const std::string& login, std::string& outPassword) const;

private:
    std::unordered_map<std::string,std::string> db; ///< Хэш-таблица для хранения пар логин-пароль
};