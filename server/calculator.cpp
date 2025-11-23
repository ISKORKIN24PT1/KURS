#include "calculator.h"
#include <limits>
#include <iostream>

uint64_t Calculator::computeSumOfSquares(const std::vector<uint64_t>& vector) {
    uint64_t sum = 0;
    
    for (uint64_t value : vector) {
        // Проверка на переполнение при возведении в квадрат
        if (value > 0) {
            // Проверяем, не превысит ли value * value максимальное значение uint64_t
            if (value > std::numeric_limits<uint64_t>::max() / value) {
                // Переполнение при умножении - возвращаем 1 согласно ТЗ
                std::cout << "DEBUG: Overflow detected in multiplication for value: " << value << std::endl;
                return 1;
            }
        }
        
        uint64_t square = value * value;
        
        // Проверка на переполнение при сложении
        if (sum > std::numeric_limits<uint64_t>::max() - square) {
            // Переполнение при сложении - возвращаем 1 согласно ТЗ
            std::cout << "DEBUG: Overflow detected in addition. sum: " << sum << ", square: " << square << std::endl;
            return 1;
        }
        
        sum += square;
    }
    
    return sum;
}
