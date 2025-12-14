/**
 * @file calculator.cpp
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Реализация класса Calculator для вычисления суммы квадратов элементов вектора.
 * @details Содержит метод computeSumOfSquares с контролем переполнения.
 * @warning При переполнении возвращает 1 согласно техническому заданию.
 */

#include "calculator.h"
#include <limits>
#include <iostream>

/**
 * @brief Вычисляет сумму квадратов элементов вектора с контролем переполнения.
 * 
 * @param[in] vector Вектор целых чисел типа uint64_t.
 * @return Сумма квадратов элементов вектора, либо 1 в случае переполнения.
 * @details Алгоритм последовательно обрабатывает каждый элемент вектора:
 *          1. Проверяет возможность переполнения при умножении value * value.
 *          2. Если переполнение при умножении - возвращает 1.
 *          3. Вычисляет квадрат значения.
 *          4. Проверяет возможность переполнения при сложении с текущей суммой.
 *          5. Если переполнение при сложении - возвращает 1.
 *          6. Добавляет квадрат к общей сумме.
 *          В отладочном режиме выводится информация о детектировании переполнения.
 * @note Согласно техническому заданию, при переполнении возвращается значение 1.
 * @see Курсовая работа, раздел 2.1.2 "Класс Calculator"
 */
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
