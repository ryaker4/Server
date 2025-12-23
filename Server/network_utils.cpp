#include "network_utils.h"
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <algorithm>

namespace NetworkUtils {

/**
 * @brief Гарантированно читает все запрошенные данные из сокета
 * @details Выполняет циклическое чтение до тех пор, пока не будет прочитано
 *          указанное количество байт или не произойдет ошибка. Используется
 *          для чтения сообщений фиксированной длины в потоковом протоколе.
 * @param fd Файловый дескриптор сокета
 * @param buf Указатель на буфер для приема данных
 * @param len Количество байт для чтения
 * @return Фактическое количество прочитанных байт (должно быть равно len),
 *         -1 при ошибке, или 0 если соединение закрыто
 * @note В отличие от стандартного recv(), эта функция гарантирует чтение
 *       всего запрошенного объема данных за исключением случаев ошибок.
 */
ssize_t recvAll(int fd, void* buf, size_t len) {
    char* p = static_cast<char*>(buf);
    size_t rem = len;

    while(rem > 0) {
        ssize_t r = recv(fd, p, rem, 0);
        if(r <= 0)
            return r;
        p += r;
        rem -= r;
    }
    return static_cast<ssize_t>(len);
}

/**
 * @brief Преобразует массив байт в шестнадцатеричную строку в верхнем регистре
 * @details Каждый байт преобразуется в два шестнадцатеричных символа.
 *          Используется для удобного отображения бинарных данных в логах
 *          и для передачи хэшей в текстовом формате.
 * @param data Указатель на массив байт
 * @param length Количество байт для преобразования
 * @return Строка, содержащая шестнадцатеричное представление данных
 * @example
 *   bytesToHex({0xDE, 0xAD, 0xBE, 0xEF}, 4) -> "DEADBEEF"
 */
std::string bytesToHex(const unsigned char* data, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::uppercase;
    for(size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

/**
 * @brief Преобразует шестнадцатеричную строку в массив байт
 * @details Выполняет обратное преобразование bytesToHex(). Проверяет корректность
 *          входной строки и соответствие длины выходного буфера.
 * @param hex Строка в шестнадцатеричном формате (только символы 0-9, A-F, a-f)
 * @param output Указатель на буфер для записи результата
 * @param output_len Ожидаемая длина выходного буфера в байтах
 * @return true если преобразование успешно выполнено,
 *         false если строка имеет неверный формат или длину
 * @pre Длина hex должна быть равна output_len * 2
 * @post В буфере output будут записаны преобразованные байты
 */
bool hexToBytes(const std::string& hex, unsigned char* output, size_t output_len) {
    if(hex.length() != output_len * 2) {
        return false;
    }
    
    auto hexCharToByte = [](char c) -> unsigned char {
        if(c >= '0' && c <= '9')
            return c - '0';
        if(c >= 'a' && c <= 'f')
            return 10 + (c - 'a');
        if(c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
        return 0xFF;
    };
    
    for(size_t i = 0; i < output_len; i++) {
        unsigned char high = hexCharToByte(hex[i * 2]);
        unsigned char low = hexCharToByte(hex[i * 2 + 1]);
        if(high == 0xFF || low == 0xFF) {
            return false;
        }
        output[i] = (high << 4) | low;
    }
    
    return true;
}

/**
 * @brief Преобразует структуру sockaddr_in в строку формата "IP:PORT"
 * @details Используется для логирования информации о подключенных клиентах.
 *          Преобразует сетевой адрес в текстовый формат и добавляет номер порта.
 * @param addr Структура адреса сокета, содержащая IP и порт
 * @return Строка в формате "xxx.xxx.xxx.xxx:ppppp"
 * @note Порт преобразуется из сетевого порядка байт в хостовой
 */
std::string sockaddrToString(const sockaddr_in& addr) {
    char ipbuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ipbuf, sizeof(ipbuf));
    return std::string(ipbuf) + ":" + std::to_string(ntohs(addr.sin_port));
}

/**
 * @brief Проверяет, является ли строка корректной шестнадцатеричной записью
 * @details Проверяет каждый символ строки на принадлежность к набору
 *          шестнадцатеричных символов (0-9, a-f, A-F).
 * @param str Проверяемая строка
 * @return true если все символы строки являются шестнадцатеричными,
 *         false в противном случае
 * @note Не проверяет длину строки, только содержание
 */
bool isValidHex(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](char c) {
        return (c >= '0' && c <= '9') || 
               (c >= 'a' && c <= 'f') || 
               (c >= 'A' && c <= 'F');
    });
}

/**
 * @brief Читает 32-битное беззнаковое целое в сетевом порядке байт
 * @details Читает 4 байта из сокета и преобразует их из сетевого порядка
 *          байт в порядок хоста. Используется для чтения заголовков протокола.
 * @param fd Файловый дескриптор сокета
 * @return Прочитанное значение
 * @throw std::runtime_error если не удалось прочитать 4 байта
 * @note Использует recvAll() для гарантированного чтения
 */
uint32_t readNetworkUint32(int fd) {
    uint32_t value;
    if(recvAll(fd, &value, 4) != 4) {
        throw std::runtime_error("Failed to read uint32");
    }
    return value;
}

/**
 * @brief Отправляет 32-битное беззнаковое целое в сетевом порядке байт
 * @details Преобразует значение в сетевой порядок байт и отправляет
 *          его через сокет. Используется для отправки заголовков протокола.
 * @param fd Файловый дескриптор сокета
 * @param value Значение для отправки
 * @return true если отправлено успешно (ровно 4 байта),
 *         false в противном случае
 */
bool sendNetworkUint32(int fd, uint32_t value) {
    return send(fd, &value, 4, 0) == 4;
}

} 