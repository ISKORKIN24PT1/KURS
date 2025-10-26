#include "authenticator.h"
#include <fstream>
#include <sstream>
#include <openssl/sha.h>
#include <iomanip>

bool Authenticator::loadUsersFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string login = line.substr(0, pos);
            std::string password = line.substr(pos + 1);
            users[login] = password;
        }
    }
    
    file.close();
    return true;
}

std::string Authenticator::computeHash(const std::string& salt, const std::string& password) {
    std::string data = salt + password;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

bool Authenticator::authenticate(const std::string& login, const std::string& salt, 
                                const std::string& clientHash) {
    auto it = users.find(login);
    if (it == users.end()) {
        return false;
    }
    
    std::string serverHash = computeHash(salt, it->second);
    return serverHash == clientHash;
}
