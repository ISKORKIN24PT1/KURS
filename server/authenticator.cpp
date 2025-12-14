/**
 * @file authenticator.cpp
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Реализация класса Authenticator для аутентификации пользователей.
 * @details Содержит методы для загрузки пользователей из файла,
 * вычисления хеша пароля и проверки аутентификации.
 * @warning Для работы требуется библиотека Crypto++ для хеширования.
 */

#include "authenticator.h"
#include <fstream>
#include <sstream>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <iomanip>
#include <iostream>

using namespace CryptoPP;

/**
 * @brief Загружает пользователей из текстового файла.
 * @param filename Имя файла с данными пользователей.
 * @return true, если загрузка прошла успешно, false в случае ошибки.
 * @throw std::runtime_error если файл не может быть открыт.
 * @warning Формат файла: каждая строка содержит логин и пароль, разделённые пробелом или табуляцией.
 * @details Файл читается построчно, данные очищаются от лишних пробелов.
 *          Дублирование логинов и пустые значения не допускаются.
 */
bool Authenticator::loadUsersFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open users file: " << filename << std::endl;
        return false;
    }
    
    users.clear(); // Очищаем предыдущих пользователей
    std::string line;
    int count = 0;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        // Удаляем начальные и конечные пробелы
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            // Пустая строка - это нормально, просто пропускаем
            continue;
        }
        
        size_t end = line.find_last_not_of(" \t");
        std::string trimmed_line = line.substr(start, end - start + 1);
        
        // Ищем первый пробел для разделения логина и пароля
        size_t pos = trimmed_line.find(' ');
        if (pos == std::string::npos) {
            // Нет пробела - ищем табуляцию
            pos = trimmed_line.find('\t');
        }
        
        if (pos == std::string::npos || pos == 0 || pos == trimmed_line.length() - 1) {
            // Нет разделителя, или логин пустой, или пароль пустой
            std::cerr << "ERROR: Invalid line format at line " << lineNum 
                      << " in file " << filename 
                      << ": '" << trimmed_line << "'" << std::endl;
            std::cerr << "Expected format: 'login password'" << std::endl;
            return false;
        }
        
        std::string login = trimmed_line.substr(0, pos);
        std::string password = trimmed_line.substr(pos + 1);
        
        // Удаляем пробелы вокруг логина и пароля
        login = login.substr(login.find_first_not_of(" \t"), 
                           login.find_last_not_of(" \t") - login.find_first_not_of(" \t") + 1);
        password = password.substr(password.find_first_not_of(" \t"), 
                                 password.find_last_not_of(" \t") - password.find_first_not_of(" \t") + 1);
        
        if (login.empty() || password.empty()) {
            std::cerr << "ERROR: Empty login or password at line " << lineNum 
                      << " in file " << filename 
                      << ": '" << trimmed_line << "'" << std::endl;
            return false;
        }
        
        // Проверяем, нет ли уже такого логина
        if (users.find(login) != users.end()) {
            std::cerr << "ERROR: Duplicate login '" << login 
                      << "' at line " << lineNum 
                      << " in file " << filename << std::endl;
            return false;
        }
        
        users[login] = password;
        count++;
        std::cout << "DEBUG: Loaded user: '" << login << "' with password: '" << password << "'" << std::endl;
    }
    
    file.close();
    
    if (count == 0) {
        std::cerr << "ERROR: No valid users loaded from file: " << filename << std::endl;
        std::cerr << "DEBUG: Format must be: login password (separated by space or tab)" << std::endl;
        return false;
    }
    
    std::cout << "DEBUG: Total users loaded: " << count << std::endl;
    return true;
}

/**
 * @brief Вычисляет SHA-256 хеш от соли и пароля.
 * @param salt Строка с солью для хеширования.
 * @param password Пароль в открытом виде.
 * @return Строка с шестнадцатеричным представлением хеша.
 * @throw std::runtime_error если библиотека Crypto++ недоступна или произошла ошибка хеширования.
 * @details Используется алгоритм SHA-256, результат кодируется в HEX.
 */
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

/**
 * @brief Проверяет аутентификацию пользователя.
 * @param login Логин пользователя.
 * @param salt Соль, переданная клиентом.
 * @param clientHash Хеш, вычисленный клиентом.
 * @return true, если аутентификация успешна, false в противном случае.
 * @details Сравнивает хеш, вычисленный на сервере (соль + пароль из БД), с хешем от клиента.
 *          Включает подробный отладочный вывод для диагностики.
 */
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
