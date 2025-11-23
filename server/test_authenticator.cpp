#include "test_authenticator.h"
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
        file << "testuser:TestPassword123\n";
        file << "admin:Admin@123\n";
        file << "user1:Password1\n";
        file.close();
    }

    void createCorruptedUsersFile() {
        ofstream file(TEST_CORRUPTED_FILE);
        file << "user_without_colon\n";
        file << ":password_only\n";
        file << "only_login:\n";
        file.close();
    }

    void cleanupTestFiles() {
        remove(TEST_USERS_FILE.c_str());
        remove(TEST_EMPTY_FILE.c_str());
        remove(TEST_CORRUPTED_FILE.c_str());
    }
}

TEST(Authenticator_LoadUsersFromFile) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    bool result = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    
    CHECK_EQUAL(true, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

TEST(Authenticator_LoadUsersFromNonExistentFile) {
    Authenticator auth;
    bool result = auth.loadUsersFromFile("non_existent_file.txt");
    
    CHECK_EQUAL(false, result);
}

TEST(Authenticator_LoadUsersFromCorruptedFile) {
    TestAuthenticatorHelpers::createCorruptedUsersFile();
    
    Authenticator auth;
    bool result = auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_CORRUPTED_FILE);
    
    CHECK_EQUAL(true, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

TEST(Authenticator_ComputeHash) {
    Authenticator auth;
    
    string salt = "4F9C429F5C6884DB";
    string password = "TestPassword123";
    
    string hash = auth.computeHash(salt, password);
    
    CHECK_EQUAL(64, hash.length());
    
    for (char c : hash) {
        bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        CHECK_EQUAL(true, isHex);
    }
}

TEST(Authenticator_AuthenticateSuccess) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    
    string login = "testuser";
    string salt = "4F9C429F5C6884DB";
    string password = "TestPassword123";
    
    string clientHash = auth.computeHash(salt, password);
    
    bool result = auth.authenticate(login, salt, clientHash);
    
    CHECK_EQUAL(true, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

TEST(Authenticator_AuthenticateWrongPassword) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    
    string login = "testuser";
    string salt = "4F9C429F5C6884DB";
    string wrongPassword = "WrongPassword";
    
    string clientHash = auth.computeHash(salt, wrongPassword);
    bool result = auth.authenticate(login, salt, clientHash);
    
    CHECK_EQUAL(false, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}

TEST(Authenticator_AuthenticateWrongLogin) {
    TestAuthenticatorHelpers::createTestUsersFile();
    
    Authenticator auth;
    auth.loadUsersFromFile(TestAuthenticatorHelpers::TEST_USERS_FILE);
    
    string wrongLogin = "nonexistentuser";
    string salt = "4F9C429F5C6884DB";
    string password = "TestPassword123";
    
    string clientHash = auth.computeHash(salt, password);
    bool result = auth.authenticate(wrongLogin, salt, clientHash);
    
    CHECK_EQUAL(false, result);
    
    TestAuthenticatorHelpers::cleanupTestFiles();
}
