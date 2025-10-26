#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <string>
#include <unordered_map>
#include <vector>

class Authenticator {
private:
    std::unordered_map<std::string, std::string> users; // login -> password
    
public:
    bool loadUsersFromFile(const std::string& filename);
    bool authenticate(const std::string& login, const std::string& salt, const std::string& clientHash);
    std::string computeHash(const std::string& salt, const std::string& password);
};

#endif
