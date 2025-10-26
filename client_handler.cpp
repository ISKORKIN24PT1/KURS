#include "client_handler.h"
#include <sstream>
#include <unistd.h>
#include <iostream>

ClientHandler::ClientHandler(int socket, Authenticator& auth, Calculator& calc, Logger& log)
    : clientSocket(socket), authenticator(auth), calculator(calc), logger(log) {
    network = std::make_unique<NetworkHelper>();
    network->setClientSocket(clientSocket);
}

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
        }
    } else {
        std::cout << "DEBUG: Parsed standard format - Login: " << login 
                  << " Salt: " << salt 
                  << " Hash: " << clientHash.substr(0, 16) << "..." << std::endl;
        
        logger.logInfo("Parsed standard format - Login: " + login + 
                      " Salt: " + salt + 
                      " Hash: " + clientHash.substr(0, 16) + "...");
    }
    
    if (login.empty() || salt.empty() || clientHash.empty()) {
        std::cout << "DEBUG: Failed to parse auth message" << std::endl;
        logger.logError("Failed to parse auth message");
        sendErrorAndClose("Invalid authentication format");
        return false;
    }
    
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

bool ClientHandler::processVectors() {
    uint32_t numVectors;
    
    // Получаем количество векторов
    if (!network->receiveUint32(numVectors)) {
        sendErrorAndClose("Failed to receive number of vectors");
        return false;
    }
    
    logger.logInfo("Processing " + std::to_string(numVectors) + " vectors");
    std::cout << "DEBUG: Processing " << numVectors << " vectors" << std::endl;
    
    std::vector<uint64_t> results;
    
    // Обрабатываем каждый вектор
    for (uint32_t i = 0; i < numVectors; i++) {
        uint32_t vectorSize;
        
        // Получаем размер вектора
        if (!network->receiveUint32(vectorSize)) {
            sendErrorAndClose("Failed to receive vector size");
            return false;
        }
        
        std::cout << "DEBUG: Vector " << (i+1) << " size: " << vectorSize << std::endl;
        
        // Получаем значения вектора
        std::vector<uint64_t> vector;
        if (!network->receiveVector(vector, vectorSize)) {
            sendErrorAndClose("Failed to receive vector data");
            return false;
        }
        
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
    
    return true;
}

void ClientHandler::sendErrorAndClose(const std::string& error) {
    logger.logError(error);
    std::cout << "DEBUG ERROR: " << error << std::endl;
    network->sendString("ERR");
}
