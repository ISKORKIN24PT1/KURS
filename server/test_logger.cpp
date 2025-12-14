#include "test_logger.h"
#include "logger.h"
#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <thread>
#include <vector>

using namespace std;

namespace TestLoggerHelpers {
    const string TEST_LOG_FILE = "test_log.log";

    void cleanupTestFiles() {
        remove(TEST_LOG_FILE.c_str());
    }

    bool fileContainsString(const string& filename, const string& searchString) {
        ifstream file(filename);
        string line;
        
        while (getline(file, line)) {
            if (line.find(searchString) != string::npos) {
                return true;
            }
        }
        return false;
    }
}

// 3.1: Создание логгера
TEST(Logger_Creation) {
    TestLoggerHelpers::cleanupTestFiles();
    
    Logger logger(TestLoggerHelpers::TEST_LOG_FILE);
    
    ifstream file(TestLoggerHelpers::TEST_LOG_FILE);
    CHECK_EQUAL(true, file.good());
    
    TestLoggerHelpers::cleanupTestFiles();
}

// 3.2: Логирование информационного сообщения
TEST(Logger_LogInfo) {
    TestLoggerHelpers::cleanupTestFiles();
    
    Logger logger(TestLoggerHelpers::TEST_LOG_FILE);
    string testMessage = "TEST_INFO_MESSAGE_12345";
    
    logger.logInfo(testMessage);
    
    bool found = TestLoggerHelpers::fileContainsString(TestLoggerHelpers::TEST_LOG_FILE, testMessage);
    CHECK_EQUAL(true, found);
    
    found = TestLoggerHelpers::fileContainsString(TestLoggerHelpers::TEST_LOG_FILE, "INFO");
    CHECK_EQUAL(true, found);
    
    TestLoggerHelpers::cleanupTestFiles();
}

// 3.3: Логирование сообщения об ошибке
TEST(Logger_LogError) {
    TestLoggerHelpers::cleanupTestFiles();
    
    Logger logger(TestLoggerHelpers::TEST_LOG_FILE);
    string testMessage = "TEST_ERROR_MESSAGE_67890";
    
    logger.logError(testMessage);
    
    bool found = TestLoggerHelpers::fileContainsString(TestLoggerHelpers::TEST_LOG_FILE, testMessage);
    CHECK_EQUAL(true, found);
    
    found = TestLoggerHelpers::fileContainsString(TestLoggerHelpers::TEST_LOG_FILE, "ERROR");
    CHECK_EQUAL(true, found);
    
    TestLoggerHelpers::cleanupTestFiles();
}
