#include "vector_processor.h"
#include <limits>

int32_t VectorProcessor::sumClamp(const std::vector<float>& v) {
    double acc = 0.0;
    for (float x : v) {
        acc += x;
        if (acc < 0) acc = 0; // защита от отрицательных / переполнения вниз
        if (acc > static_cast<double>(std::numeric_limits<int32_t>::max()))
            return std::numeric_limits<int32_t>::max();
    }
    if (acc < 0) return 0;
    if (acc > static_cast<double>(std::numeric_limits<int32_t>::max()))
        return std::numeric_limits<int32_t>::max();
    return static_cast<int32_t>(acc);
}
