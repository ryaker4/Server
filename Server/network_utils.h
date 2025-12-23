#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>

namespace NetworkUtils {
    /**
     * @brief Гарантированное чтение всех запрошенных данных из сокета
     * @param fd Файловый дескриптор сокета
     * @param buf Указатель на буфер для приема данных
     * @param len Количество байт для чтения
     * @return Количество прочитанных байт, или -1 при ошибке, или 0 при закрытии соединения
     */
    ssize_t recvAll(int fd, void* buf, size_t len);
    
    /**
     * @brief Преобразование массива байт в шестнадцатеричную строку
     * @param data Указатель на массив байт
     * @param length Количество байт для преобразования
     * @return Строка в шестнадцатеричном формате в верхнем регистре
     */
    std::string bytesToHex(const unsigned char* data, size_t length);
    
    /**
     * @brief Преобразование шестнадцатеричной строки в массив байт
     * @param hex Строка в шестнадцатеричном формате
     * @param output Указатель на буфер для записи результата
     * @param output_len Ожидаемая длина выходного буфера в байтах
     * @return true если преобразование успешно, false в противном случае
     */
    bool hexToBytes(const std::string& hex, unsigned char* output, size_t output_len);
    
    /**
     * @brief Преобразование структуры sockaddr_in в строку формата "IP:PORT"
     * @param addr Структура адреса сокета
     * @return Строковое представление адреса
     */
    std::string sockaddrToString(const sockaddr_in& addr);
    
    /**
     * @brief Проверка строки на корректность шестнадцатеричного формата
     * @param str Проверяемая строка
     * @return true если строка содержит только шестнадцатеричные символы, false в противном случае
     */
    bool isValidHex(const std::string& str);
    
    /**
     * @brief Чтение 32-битного беззнакового целого в сетевом порядке байт
     * @param fd Файловый дескриптор сокета
     * @return Прочитанное значение
     * @throw std::runtime_error при ошибке чтения
     */
    uint32_t readNetworkUint32(int fd);
    
    /**
     * @brief Отправка 32-битного беззнакового целого в сетевом порядке байт
     * @param fd Файловый дескриптор сокета
     * @param value Значение для отправки
     * @return true если отправлено успешно, false в противном случае
     */
    bool sendNetworkUint32(int fd, uint32_t value);
}

#endif