#include "calculator.h"
#include <limits>

uint64_t Calculator::computeSumOfSquares(const std::vector<uint64_t>& vector) {
    uint64_t sum = 0;
    
    for (uint64_t value : vector) {
        // Проверка на переполнение
        if (value > 0 && sum > std::numeric_limits<uint64_t>::max() / value) {
            // Переполнение вверх - возвращаем 1
            return 1;
        }
        
        uint64_t square = value * value;
        
        // Проверка на переполнение при сложении
        if (sum > std::numeric_limits<uint64_t>::max() - square) {
            // Переполнение вверх - возвращаем 1
            return 1;
        }
        
        sum += square;
    }
    
    return sum;
}
