#include "test_client_handler.h"
#include "client_handler.h"
#include "authenticator.h"
#include "calculator.h"
#include "logger.h"
#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <vector>

using namespace std;

namespace TestClientHandlerHelpers {
    const string TEST_USERS_FILE = "test_client_users.txt";
    const string TEST_LOG_FILE = "test_client.log";

    void createTestUsersFile() {
        ofstream file(TEST_USERS_FILE);
        file << "testuser TestPassword123\n";
        file.close();
    }

    void cleanupTestFiles() {
        remove(TEST_USERS_FILE.c_str());
        remove(TEST_LOG_FILE.c_str());
    }
}

// 4.1: Парсинг сообщения аутентификации
TEST(ClientHandler_AuthenticationMessageParsing) {
    TestClientHandlerHelpers::createTestUsersFile();
    
    Authenticator auth;
    Logger logger(TestClientHandlerHelpers::TEST_LOG_FILE);
    
    bool loaded = auth.loadUsersFromFile(TestClientHandlerHelpers::TEST_USERS_FILE);
    CHECK(loaded);
    
    string login = "testuser";
    string salt = "4F9C429F5C6884DB";
    string password = "TestPassword123";
    
    string clientHash = auth.computeHash(salt, password);
    
    CHECK_EQUAL(64, clientHash.length());
    
    TestClientHandlerHelpers::cleanupTestFiles();
}

// 4.2: Обработка вектора
TEST(ClientHandler_VectorProcessing) {
    Calculator calc;
    
    vector<uint64_t> testVector = {1, 2, 3, 4, 5};
    uint64_t result = calc.computeSumOfSquares(testVector);
    
    CHECK_EQUAL(55, result);
}

// 4.3: Обработка пустого вектора
TEST(ClientHandler_EmptyVectorProcessing) {
    Calculator calc;
    
    vector<uint64_t> testVector;
    uint64_t result = calc.computeSumOfSquares(testVector);
    
    CHECK_EQUAL(0, result);
}
