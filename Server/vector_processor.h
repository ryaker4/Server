#pragma once
#include <vector>
#include <cstdint>

class VectorProcessor {
public:
    // суммирует элементы (float/double) и контролирует переполнение.
    // Возвращает приводимое к int32 значение в диапазоне [0, 2^31-1].
    static int32_t sumClamp(const std::vector<float>& v);
};
