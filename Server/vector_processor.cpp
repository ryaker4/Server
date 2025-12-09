#include "vector_processor.h"
#include <limits>
#include <cstdint>

int32_t VectorProcessor::sumClamp(const std::vector<uint32_t>& v) {
    int64_t acc = 0;  // Используем 64-бит для избежания переполнения
    
    for (uint32_t x : v) {
        acc += x;
        if (acc < 0) acc = 0;
        if (acc > static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
            return std::numeric_limits<int32_t>::max();
    }
    
    if (acc < 0) return 0;
    if (acc > static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
        return std::numeric_limits<int32_t>::max();
    
    return static_cast<int32_t>(acc);
}