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
    
    users.clear(); // Очищаем предыдущих пользователей
    std::string line;
    int count = 0;
    int skipped = 0;
    
    while (std::getline(file, line)) {
        // Удаляем начальные и конечные пробелы
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            // Пустая строка
            skipped++;
            continue;
        }
        
        size_t end = line.find_last_not_of(" \t");
        std::string trimmed_line = line.substr(start, end - start + 1);
        
        // Ищем разделитель
        size_t pos = trimmed_line.find(':');
        if (pos == std::string::npos || pos == 0 || pos == trimmed_line.length() - 1) {
            // Нет разделителя, или логин пустой, или пароль пустой
            std::cout << "DEBUG: Invalid line format, skipping: '" << trimmed_line << "'" << std::endl;
            skipped++;
            continue;
        }
        
        std::string login = trimmed_line.substr(0, pos);
        std::string password = trimmed_line.substr(pos + 1);
        
        // Удаляем пробелы вокруг логина и пароля
        login = login.substr(login.find_first_not_of(" \t"), login.find_last_not_of(" \t") - login.find_first_not_of(" \t") + 1);
        password = password.substr(password.find_first_not_of(" \t"), password.find_last_not_of(" \t") - password.find_first_not_of(" \t") + 1);
        
        if (login.empty() || password.empty()) {
            std::cout << "DEBUG: Empty login or password, skipping: '" << trimmed_line << "'" << std::endl;
            skipped++;
            continue;
        }
        
        users[login] = password;
        count++;
        std::cout << "DEBUG: Loaded user: '" << login << "' with password: '" << password << "'" << std::endl;
    }
    
    file.close();
    
    if (count == 0) {
        std::cerr << "ERROR: No valid users loaded from file: " << filename << std::endl;
        std::cerr << "DEBUG: Format must be: login:password" << std::endl;
        return false;
    }
    
    std::cout << "DEBUG: Total users loaded: " << count << std::endl;
    if (skipped > 0) {
        std::cout << "DEBUG: Skipped invalid lines: " << skipped << std::endl;
    }
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
    
    // Проверяем есть ли пользователи вообще
    if (users.empty()) {
        std::cout << "DEBUG AUTH: No users loaded in database" << std::endl;
        return false;
    }
    
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
