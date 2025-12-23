#pragma once
#include <vector>
#include <cstdint>

/**
 * @class VectorProcessor
 * @brief Класс для обработки векторов чисел с контролем переполнения
 */
class VectorProcessor {
public:
    /**
     * @brief Суммирует элементы вектора с контролем переполнения
     * @details Использует 64-битный аккумулятор для избежания переполнения,
     *          возвращает результат в диапазоне [0, 2^31-1]
     * @param v Вектор 32-битных беззнаковых целых чисел
     * @return Сумма элементов, приведенная к int32_t с учетом ограничений
     */
    static int32_t sumClamp(const std::vector<uint32_t>& v);
};