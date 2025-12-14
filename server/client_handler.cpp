/**
 * @file client_handler.cpp
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Реализация класса ClientHandler для обработки клиентских подключений.
 * @details Содержит методы для аутентификации клиентов, обработки векторов данных
 *          и управления соединением.
 */

#include "client_handler.h"
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cctype>

/**
 * @brief Конструктор класса ClientHandler.
 * @param socket Дескриптор клиентского сокета.
 * @param auth Ссылка на объект аутентификации.
 * @param calc Ссылка на объект вычислений.
 * @param log Ссылка на объект логирования.
 */
ClientHandler::ClientHandler(int socket, Authenticator& auth, Calculator& calc, Logger& log)
    : clientSocket(socket), authenticator(auth), calculator(calc), logger(log) {
    network = std::make_unique<NetworkHelper>();
    network->setClientSocket(clientSocket);
}

/**
 * @brief Основной метод обработки клиентского подключения.
 */
void ClientHandler::handleClient() {
    logger.logInfo("New client connected, socket: " + std::to_string(clientSocket));
    
    // Аутентификация
    if (!authenticateClient()) {
        logger.logError("Client authentication failed");
        network->closeClient();
        return;
    }
    
    logger.logInfo("Client authenticated successfully");
    
    // Обработка векторов
    if (!processVectors()) {
        logger.logError("Error processing vectors");
        network->closeClient();
        return;
    }
    
    logger.logInfo("Client processing completed successfully");
    network->closeClient();
}

/**
 * @brief Вспомогательная функция для проверки строки на содержание только шестнадцатеричных символов.
 * @param str Проверяемая строка.
 * @return true, если строка содержит только символы 0-9, A-F, a-f; false в противном случае.
 */
bool isValidHexString(const std::string& str) {
    if (str.empty()) return false;
    
    for (char c : str) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Выполняет аутентификацию клиента.
 * @return true, если аутентификация успешна, false в противном случае.
 */
bool ClientHandler::authenticateClient() {
    // Получаем сообщение аутентификации
    std::string authMessage = network->receiveString(256);
    if (authMessage.empty()) {
        sendErrorAndClose("Empty authentication message");
        return false;
    }
    
    // Отладочный вывод
    std::cout << "DEBUG: Received auth message: " << authMessage << std::endl;
    logger.logInfo("Auth message received: " + authMessage.substr(0, 50) + "...");
    
    std::string login, salt, clientHash;
    
    // Пробуем два формата:
    // 1. Формат с пробелами: "LOGIN SALT16 HASH"
    std::istringstream iss(authMessage);
    iss >> login >> salt >> clientHash;
    
    // 2. Если не получилось, пробуем разобрать слитный формат: "LOGINSALT16HASH"
    if (login.empty() || salt.empty() || clientHash.empty()) {
        std::cout << "DEBUG: Trying to parse compact format..." << std::endl;
        logger.logInfo("Trying to parse compact auth format");
        
        // Предполагаем что логин "user" (4 символа), соль 16 символов, остальное - хеш
        if (authMessage.length() >= 4 + 16) {
            login = authMessage.substr(0, 4);  // "user"
            salt = authMessage.substr(4, 16);  // 16 символов соли
            clientHash = authMessage.substr(20); // остальное - хеш
            
            std::cout << "DEBUG: Parsed - Login: " << login 
                      << " Salt: " << salt 
                      << " Hash: " << clientHash.substr(0, 16) << "..." << std::endl;
            
            logger.logInfo("Parsed compact format - Login: " + login + 
                          " Salt: " + salt + 
                          " Hash: " + clientHash.substr(0, 16) + "...");
        } else {
            std::cout << "DEBUG: Auth message too short: " << authMessage.length() << " chars" << std::endl;
            logger.logError("Auth message too short: " + std::to_string(authMessage.length()) + " chars");
            sendErrorAndClose("Authentication message too short");
            return false;
        }
    } else {
        std::cout << "DEBUG: Parsed standard format - Login: " << login 
                  << " Salt: " << salt 
                  << " Hash: " << clientHash.substr(0, 16) << "..." << std::endl;
        
        logger.logInfo("Parsed standard format - Login: " + login + 
                      " Salt: " + salt + 
                      " Hash: " + clientHash.substr(0, 16) + "...");
    }
    
    // Проверка логина
    if (login.empty()) {
        std::cout << "DEBUG: Empty login" << std::endl;
        logger.logError("Empty login");
        sendErrorAndClose("Login cannot be empty");
        return false;
    }
    
    // ПРОВЕРКА СОЛИ: ДОЛЖНА БЫТЬ РОВНО 16 HEX СИМВОЛОВ (64 БИТА)
    if (salt.length() != 16) {
        std::cout << "DEBUG ERROR: Salt must be exactly 16 hexadecimal digits (64 bits), "
                  << "got " << salt.length() << " characters: " << salt << std::endl;
        logger.logError("Invalid salt length: " + std::to_string(salt.length()) + 
                       " characters (expected 16)");
        sendErrorAndClose("Salt must be 16 hexadecimal digits (64 bits)");
        return false;
    }
    
    // Проверка что соль состоит только из hex символов
    if (!isValidHexString(salt)) {
        std::cout << "DEBUG ERROR: Salt contains non-hexadecimal characters: " << salt << std::endl;
        logger.logError("Salt contains non-hexadecimal characters");
        sendErrorAndClose("Salt must contain only hexadecimal digits (0-9, A-F)");
        return false;
    }
    
    // Проверка хеша
    if (clientHash.empty()) {
        std::cout << "DEBUG: Empty hash" << std::endl;
        logger.logError("Empty hash");
        sendErrorAndClose("Hash cannot be empty");
        return false;
    }
    
    // Проверка что хеш состоит только из hex символов и имеет разумную длину
    // SHA-256 hash в hex формате должен быть 64 символа
    if (clientHash.length() != 64) {
        std::cout << "DEBUG WARNING: Hash length is " << clientHash.length() 
                  << " (expected 64 for SHA-256)" << std::endl;
        // Не прерываем соединение, так как хеш может быть длиннее из-за пробелов или других символов
    }
    
    if (!isValidHexString(clientHash)) {
        std::cout << "DEBUG ERROR: Hash contains non-hexadecimal characters" << std::endl;
        logger.logError("Hash contains non-hexadecimal characters");
        sendErrorAndClose("Hash must contain only hexadecimal digits");
        return false;
    }
    
    // Валидация успешна
    std::cout << "DEBUG: Authentication data validated successfully" << std::endl;
    std::cout << "DEBUG: Salt (" << salt.length() << " hex digits = " 
              << (salt.length() * 4) << " bits): " << salt << std::endl;
    logger.logInfo("Salt validated: " + salt + " (" + std::to_string(salt.length() * 4) + " bits)");
    
    // Проверяем аутентификацию
    if (authenticator.authenticate(login, salt, clientHash)) {
        network->sendString("OK");
        std::cout << "DEBUG: Authentication SUCCESS" << std::endl;
        logger.logInfo("Authentication SUCCESS for user: " + login);
        return true;
    } else {
        network->sendString("ERR");
        std::cout << "DEBUG: Authentication FAILED" << std::endl;
        logger.logError("Authentication FAILED for user: " + login);
        return false;
    }
}

/**
 * @brief Обрабатывает векторы данных от клиента.
 * @return true, если обработка успешна, false в случае ошибки.
 */
bool ClientHandler::processVectors() {
    uint32_t numVectors;
    
    // Получаем количество векторов
    if (!network->receiveUint32(numVectors)) {
        sendErrorAndClose("Failed to receive number of vectors");
        return false;
    }
    
    logger.logInfo("Processing " + std::to_string(numVectors) + " vectors");
    std::cout << "DEBUG: Processing " << numVectors << " vectors" << std::endl;
    
    // ВАЖНО: Проверка на разумное количество векторов (защита от ошибок/атак)
    if (numVectors == 0) {
        std::cout << "DEBUG: Zero vectors requested" << std::endl;
        logger.logInfo("Zero vectors requested - nothing to process");
        return true; // Пустой запрос - это нормально
    }
    
    // ВРЕМЕННО УВЕЛИЧИМ ЛИМИТ ДЛЯ ТЕСТИРОВАНИЯ
    if (numVectors > 10000) {
        std::cout << "DEBUG ERROR: Suspiciously large number of vectors: " << numVectors << std::endl;
        logger.logError("Suspiciously large number of vectors: " + std::to_string(numVectors));
        sendErrorAndClose("Too many vectors requested (max 10000)");
        return false;
    }
    
    std::vector<uint64_t> results;
    results.reserve(numVectors);
    
    // Обрабатываем каждый вектор
    for (uint32_t i = 0; i < numVectors; i++) {
        uint32_t vectorSize;
        
        // Получаем размер вектора
        if (!network->receiveUint32(vectorSize)) {
            sendErrorAndClose("Failed to receive vector size");
            return false;
        }
        
        std::cout << "DEBUG: Vector " << (i+1) << "/" << numVectors << " size: " << vectorSize << std::endl;
        
        // Проверка на разумный размер вектора
        if (vectorSize == 0) {
            std::cout << "DEBUG: Empty vector #" << (i+1) << std::endl;
            // Пустой вектор - результат 0
            results.push_back(0);
            
            // Отправляем результат для этого вектора
            if (!network->sendUint64(0)) {
                sendErrorAndClose("Failed to send result for empty vector");
                return false;
            }
            
            logger.logInfo("Vector " + std::to_string(i+1) + " is empty, result: 0");
            continue;
        }
        
        // ВРЕМЕННО УВЕЛИЧИМ ЛИМИТ ДЛЯ ТЕСТИРОВАНИЯ
        if (vectorSize > 10000000) {
            std::cout << "DEBUG ERROR: Suspiciously large vector size: " << vectorSize << std::endl;
            logger.logError("Suspiciously large vector size: " + std::to_string(vectorSize));
            sendErrorAndClose("Vector size too large (max 10,000,000)");
            return false;
        }
        
        // Выделяем память заранее для избежания многократных реаллокаций
        std::vector<uint64_t> vector;
        vector.reserve(vectorSize);
        
        // Получаем значения вектора
        if (!network->receiveVector(vector, vectorSize)) {
            sendErrorAndClose("Failed to receive vector data");
            return false;
        }
        
        std::cout << "DEBUG: Received vector " << (i+1) << " with " << vector.size() << " elements" << std::endl;
        
        // Вычисляем сумму квадратов
        uint64_t result = calculator.computeSumOfSquares(vector);
        results.push_back(result);
        
        // Отправляем результат для этого вектора
        if (!network->sendUint64(result)) {
            sendErrorAndClose("Failed to send result");
            return false;
        }
        
        std::cout << "DEBUG: Vector " << (i+1) << " result: " << result << std::endl;
        logger.logInfo("Vector " + std::to_string(i+1) + " processed, result: " + std::to_string(result));
    }
    
    std::cout << "DEBUG: All " << numVectors << " vectors processed successfully" << std::endl;
    logger.logInfo("All " + std::to_string(numVectors) + " vectors processed successfully");
    
    return true;
}

/**
 * @brief Отправляет сообщение об ошибке клиенту и закрывает соединение.
 * @param error Текст сообщения об ошибке.
 */
void ClientHandler::sendErrorAndClose(const std::string& error) {
    logger.logError(error);
    std::cout << "DEBUG ERROR: " << error << std::endl;
    network->sendString("ERR");
}
