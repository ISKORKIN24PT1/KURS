#include "test_calculator.h"
#include "calculator.h"
#include <UnitTest++/UnitTest++.h>
#include <vector>
#include <limits>

using namespace std;

TEST(Calculator_ComputeSumOfSquares_NormalCase) {
    Calculator calc;
    
    vector<uint64_t> input = {1, 2, 3, 4, 5};
    uint64_t result = calc.computeSumOfSquares(input);
    
    CHECK_EQUAL(55, result);
}

TEST(Calculator_ComputeSumOfSquares_EmptyVector) {
    Calculator calc;
    
    vector<uint64_t> input;
    uint64_t result = calc.computeSumOfSquares(input);
    
    CHECK_EQUAL(0, result);
}

TEST(Calculator_ComputeSumOfSquares_SingleElement) {
    Calculator calc;
    
    vector<uint64_t> input = {10};
    uint64_t result = calc.computeSumOfSquares(input);
    
    CHECK_EQUAL(100, result);
}

TEST(Calculator_ComputeSumOfSquares_ZeroValues) {
    Calculator calc;
    
    vector<uint64_t> input = {0, 0, 0, 0};
    uint64_t result = calc.computeSumOfSquares(input);
    
    CHECK_EQUAL(0, result);
}

TEST(Calculator_ComputeSumOfSquares_OverflowMultiplication) {
    Calculator calc;
    
    vector<uint64_t> input = {4294967296ULL};
    
    uint64_t result = calc.computeSumOfSquares(input);
    
    CHECK_EQUAL(1, result);
}

TEST(Calculator_ComputeSumOfSquares_OverflowAddition) {
    Calculator calc;
    
    vector<uint64_t> input = {
        std::numeric_limits<uint64_t>::max() / 2,
        std::numeric_limits<uint64_t>::max() / 2
    };
    
    uint64_t result = calc.computeSumOfSquares(input);
    
    CHECK_EQUAL(1, result);
}

TEST(Calculator_ComputeSumOfSquares_LargeNumbers) {
    Calculator calc;
    
    vector<uint64_t> input = {1000000, 2000000, 3000000};
    uint64_t result = calc.computeSumOfSquares(input);
    
    uint64_t expected = 14000000000000ULL;
    CHECK_EQUAL(expected, result);
}

TEST(Calculator_ComputeSumOfSquares_BoundaryCase) {
    Calculator calc;
    
    uint64_t max_safe_value = 4294967295ULL;
    
    vector<uint64_t> input = {max_safe_value, 1};
    uint64_t result = calc.computeSumOfSquares(input);
    
    uint64_t expected = max_safe_value * max_safe_value + 1;
    CHECK_EQUAL(expected, result);
}
