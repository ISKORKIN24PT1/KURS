#ifndef TEST_CLIENT_HANDLER_H
#define TEST_CLIENT_HANDLER_H

#include "client_handler.h"
#include "authenticator.h"
#include "calculator.h"
#include "logger.h"
#include <memory>

class TestClientHandler {
public:
    static void runAllTests();
    
private:
    static void createTestUsersFile();
    static void cleanupTestFiles();
    
    static const std::string TEST_USERS_FILE;
    static const std::string TEST_LOG_FILE;
};

#endif
