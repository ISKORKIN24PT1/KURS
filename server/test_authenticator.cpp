#include "test_authenticator.h"
#include "authenticator.h"
#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <cstdio>

using namespace std;

namespace TestAuthenticatorHelpers {
    const string TEST_USERS_FILE = "test_users.txt";
    const string TEST_EMPTY_FILE = "test_empty.txt";
    const string TEST_CORRUPTED_FILE = "test_corrupted.txt";

    void createTestUsersFile() {
        ofstream file(TEST_USERS_FILE);
        file << "testuser TestPassword123\n";
        file << "admin Admin@123\n";
        file << "user1 Password1\n";
        file.close();
    }

    void createCorruptedUsersFile() {
        ofstream file(TEST_CORRUPTED_FILE);
        file << "user_without_separator\n";
        file << ":password_only\n";
        file << "only_login:\n";
        file << "user password extra\n";
        file.close();
    }

    void cleanupTestFiles() {
        remove(TEST_USERS_FILE.c_str());
        remove(TEST_EMPTY_FILE.c_str());
        remove(TEST_CORRUPTED_FILE.c_str());
    }
}

// 1.1: Загрузка пользователей из файла
TEST(Authenticator_LoadUsersFromFile) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    bool result = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    
    CHECK_EQUAL(true, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

// 1.2: Загрузка пользователей из несуществующего файла
TEST(Authenticator_LoadUsersFromNonExistentFile) {
    Authenticator auth;
    bool result = auth.loadUsersFromFile("non_existent_file.txt");
    
    CHECK_EQUAL(false, result);
}

// 1.3: Загрузка пользователей из поврежденного файла
TEST(Authenticator_LoadUsersFromCorruptedFile) {
    TestAuthenticatorHelpers::createCorruptedUsersFile();
    
    Authenticator auth;
    bool result = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_CORRUPTED_FILE);
    
    CHECK_EQUAL(false, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

// 1.4: Вычисление хэша
TEST(Authenticator_ComputeHash) {
    Authenticator auth;
    
    string salt = "4F9C429F5C6884DB";
    string password = "TestPassword123";
    
    string hash = auth.computeHash(salt, password);
    
    CHECK_EQUAL(64, hash.length());
    
    for (char c : hash) {
        bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        CHECK(isHex);
    }
}

// 1.5: Успешная аутентификация
TEST(Authenticator_AuthenticateSuccess) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    bool loaded = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    CHECK(loaded);
    
    string login = "testuser";
    string salt = "4F9C429F5C6884DB";
    string password = "TestPassword123";
    
    string clientHash = auth.computeHash(salt, password);
    bool result = auth.authenticate(login, salt, clientHash);
    
    CHECK_EQUAL(true, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

// 1.6: Аутентификация с неправильным паролем
TEST(Authenticator_AuthenticateWrongPassword) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    bool loaded = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    CHECK(loaded);
    
    string login = "testuser";
    string salt = "4F9C429F5C6884DB";
    string wrongPassword = "WrongPassword";
    
    string clientHash = auth.computeHash(salt, wrongPassword);
    bool result = auth.authenticate(login, salt, clientHash);
    
    CHECK_EQUAL(false, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

// 1.7: Аутентификация с неправильным логином
TEST(Authenticator_AuthenticateWrongLogin) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    bool loaded = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    CHECK(loaded);
    
    string wrongLogin = "nonexistentuser";
    string salt = "4F9C429F5C6884DB";
    string password = "TestPassword123";
    
    string clientHash = auth.computeHash(salt, password);
    bool result = auth.authenticate(wrongLogin, salt, clientHash);
    
    CHECK_EQUAL(false, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

// 1.8: Работа с пустым файлом
TEST(Authenticator_EmptyFile) {
    ofstream file(TestAuthenticatorHelpers::TEST_EMPTY_FILE);
    file.close();
    
    Authenticator auth;
    bool result = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_EMPTY_FILE);
    
    CHECK_EQUAL(false, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}
