#include "authenticator.h"
#include <fstream>
#include <sstream>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>  // Используем Crypto++ вместо OpenSSL
#include <iomanip>
#include <iostream>

using namespace CryptoPP;

bool Authenticator::loadUsersFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open users file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string login = line.substr(0, pos);
            std::string password = line.substr(pos + 1);
            users[login] = password;
            count++;
            std::cout << "DEBUG: Loaded user: " << login << std::endl;
        }
    }
    
    file.close();
    std::cout << "DEBUG: Total users loaded: " << count << std::endl;
    return true;
}

std::string Authenticator::computeHash(const std::string& salt, const std::string& password) {
    std::string data = salt + password;
    
    SHA256 sha256;
    std::string hash;
    
    StringSource(data, true,
        new HashFilter(sha256,
            new HexEncoder(
                new StringSink(hash)
            )
        )
    );
    
    return hash;
}

bool Authenticator::authenticate(const std::string& login, const std::string& salt, 
                                const std::string& clientHash) {
    // Отладочный вывод
    std::cout << "=== DEBUG AUTHENTICATION ===" << std::endl;
    std::cout << "DEBUG AUTH: Login='" << login << "'" << std::endl;
    std::cout << "DEBUG AUTH: Salt='" << salt << "'" << std::endl;
    std::cout << "DEBUG AUTH: ClientHash='" << clientHash << "'" << std::endl;
    std::cout << "DEBUG AUTH: ClientHash length=" << clientHash.length() << std::endl;
    
    auto it = users.find(login);
    if (it == users.end()) {
        std::cout << "DEBUG AUTH: User '" << login << "' not found in database" << std::endl;
        std::cout << "DEBUG AUTH: Available users: ";
        for (const auto& user : users) {
            std::cout << user.first << " ";
        }
        std::cout << std::endl;
        return false;
    }
    
    std::cout << "DEBUG AUTH: User '" << login << "' found, password='" << it->second << "'" << std::endl;
    
    std::string serverHash = computeHash(salt, it->second);
    std::cout << "DEBUG AUTH: ServerHash='" << serverHash << "'" << std::endl;
    std::cout << "DEBUG AUTH: ServerHash length=" << serverHash.length() << std::endl;
    
    bool match = (serverHash == clientHash);
    std::cout << "DEBUG AUTH: Match=" << (match ? "YES" : "NO") << std::endl;
    std::cout << "=== END DEBUG AUTHENTICATION ===" << std::endl;
    
    return match;
}
