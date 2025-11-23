#ifndef TEST_LOGGER_H
#define TEST_LOGGER_H

#include "logger.h"
#include <string>

namespace TestLoggerHelpers {
    bool fileContainsString(const std::string& filename, const std::string& searchString);
    void cleanupTestFiles();
    extern const std::string TEST_LOG_FILE;
}

#endif
