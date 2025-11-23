#ifndef TEST_AUTHENTICATOR_H
#define TEST_AUTHENTICATOR_H

#include "authenticator.h"
#include <string>
#include <fstream>

// Вспомогательные функции для тестов
namespace TestAuthenticatorHelpers {
    void createTestUsersFile();
    void createCorruptedUsersFile();
    void cleanupTestFiles();
    bool fileContainsString(const std::string& filename, const std::string& searchString);
    
    extern const std::string TEST_USERS_FILE;
    extern const std::string TEST_EMPTY_FILE;
    extern const std::string TEST_CORRUPTED_FILE;
}

#endif
