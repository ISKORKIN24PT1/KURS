#include "client_handler.h"
#include <sstream>
#include <unistd.h> // Добавляем для close()

ClientHandler::ClientHandler(int socket, Authenticator& auth, Calculator& calc, Logger& log)
    : clientSocket(socket), authenticator(auth), calculator(calc), logger(log) {
    network = std::make_unique<NetworkHelper>();
    network->setClientSocket(clientSocket); // Устанавливаем клиентский сокет
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
    // Получаем сообщение аутентификации: LOGIN SALT16 HASH
    std::string authMessage = network->receiveString(256);
    if (authMessage.empty()) {
        sendErrorAndClose("Empty authentication message");
        return false;
    }
    
    // Парсим сообщение (упрощенно)
    std::istringstream iss(authMessage);
    std::string login, salt, clientHash;
    
    iss >> login >> salt >> clientHash;
    
    if (login.empty() || salt.empty() || clientHash.empty()) {
        sendErrorAndClose("Invalid authentication format");
        return false;
    }
    
    // Проверяем аутентификацию
    if (authenticator.authenticate(login, salt, clientHash)) {
        network->sendString("OK");
        return true;
    } else {
        network->sendString("ERR");
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
    
    std::vector<uint64_t> results;
    
    // Обрабатываем каждый вектор
    for (uint32_t i = 0; i < numVectors; i++) {
        uint32_t vectorSize;
        
        // Получаем размер вектора
        if (!network->receiveUint32(vectorSize)) {
            sendErrorAndClose("Failed to receive vector size");
            return false;
        }
        
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
        
        logger.logInfo("Vector " + std::to_string(i+1) + " processed, result: " + std::to_string(result));
    }
    
    return true;
}

void ClientHandler::sendErrorAndClose(const std::string& error) {
    logger.logError(error);
    network->sendString("ERR");
}
