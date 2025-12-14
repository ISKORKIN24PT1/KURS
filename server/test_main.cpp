#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <chrono>

// Определяем макрос для отключения отладочного вывода в тестах
#define DISABLE_DEBUG_OUTPUT

using namespace std;
using namespace std::chrono;

// Минималистичный репортер
class MinimalReporter : public UnitTest::TestReporter
{
private:
    int totalTests;
    int failedTests;
    high_resolution_clock::time_point startTime;
    
public:
    MinimalReporter() : totalTests(0), failedTests(0) 
    {
        startTime = high_resolution_clock::now();
    }
    
    virtual void ReportTestStart(UnitTest::TestDetails const& /*test*/) override
    {
        totalTests++;
    }
    
    virtual void ReportFailure(UnitTest::TestDetails const& test, char const* failure) override
    {
        failedTests++;
        // Только при ошибках показываем детали
        cout << "FAILED: " << test.suiteName << "." << test.testName << endl;
        cout << "  " << failure << endl;
        cout << "  " << test.filename << ":" << test.lineNumber << endl;
    }
    
    virtual void ReportTestFinish(UnitTest::TestDetails const& /*test*/, float /*secondsElapsed*/) override
    {
        // Ничего не выводим для успешных тестов
    }
    
    virtual void ReportSummary(int totalTestCount, 
                              int failedTestCount, 
                              int /*failureCount*/,
                              float /*secondsElapsed*/) override
    {
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime);
        
        cout << "\n========================================" << endl;
        cout << "TESTS COMPLETED" << endl;
        cout << "========================================" << endl;
        cout << "Total tests: " << totalTestCount << endl;
        cout << "Passed: " << (totalTestCount - failedTestCount) << endl;
        cout << "Failed: " << failedTestCount << endl;
        cout << "Time: " << duration.count() << " ms" << endl;
        cout << "========================================" << endl;
    }
};

int main() {
    // Перенаправляем stderr в /dev/null для подавления вывода ошибок
    freopen("/dev/null", "w", stderr);
    
    MinimalReporter reporter;
    UnitTest::TestRunner runner(reporter);
    
    int result = runner.RunTestsIf(UnitTest::Test::GetTestList(), 
                                  NULL, 
                                  UnitTest::True(), 
                                  0);
    
    // Восстанавливаем stderr
    freopen("/dev/stderr", "w", stderr);
    
    return result;
}
