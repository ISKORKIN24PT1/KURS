#include "server.h"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <sys/time.h>
#include <vector>
#include <algorithm>

// Глобальный указатель на экземпляр сервера для обработки сигналов
Server* serverInstance = nullptr;
// Атомарный флаг состояния работы сервера
std::atomic<bool> Server::running(false);

// Конструктор сервера
Server::Server(int port, const std::string& userFile, const std::string& logFile) 
    : serverSocket(-1), port(port), userFile(userFile), logFile(logFile), logger(logFile) {
    serverInstance = this;
    logger.logInfo("Server instance created with port: " + std::to_string(port) + 
                  ", user file: " + userFile + ", log file: " + logFile);
}

// Деструктор сервера
Server::~Server() {
    logger.logInfo("Server destructor called");
    stop();
    if (serverSocket != -1) {
        close(serverSocket);
    }
}

// Инициализация сервера
bool Server::initialize() {
    logger.logInfo("Initializing server...");
    
    // Проверяем доступность файла пользователей
    std::ifstream userTest(userFile);
    if (!userTest.is_open()) {
        logger.logError("Cannot open users file: " + userFile);
        return false;
    }
    userTest.close();
    logger.logInfo("Users file verified: " + userFile);
    
    return createSocket();
}

// Создание и настройка серверного сокета
bool Server::createSocket() {
    logger.logInfo("Creating server socket...");
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        logger.logError("Error creating socket");
        return false;
    }
    logger.logInfo("Socket created successfully");

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        logger.logError("Error setting socket options");
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        logger.logError("Error binding socket to port " + std::to_string(port));
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    logger.logInfo("Socket bound to port " + std::to_string(port));
    return true;
}

// Основной цикл работы сервера
void Server::run() {
    if (listen(serverSocket, 10) == -1) {
        logger.logError("Error starting listening");
        return;
    }

    setupSignalHandlers();
    running = true;
    
    logger.logInfo("Server started successfully on port " + std::to_string(port));
    logger.logInfo("Server is ready to accept connections");
    
    std::cout << "Сервер запущен на порту " << port << std::endl;
    std::cout << "Логи записываются в: " << logFile << std::endl;
    std::cout << "Для остановки нажмите Ctrl+C" << std::endl;

    handleConnections();
}

// Остановка сервера
void Server::stop() {
    if (running) {
        logger.logInfo("Stopping server...");
        running = false;
        
        if (serverSocket != -1) {
            close(serverSocket);
            serverSocket = -1;
            logger.logInfo("Server socket closed");
        }
        logger.logInfo("Server stopped successfully");
    }
}

// Обработка входящих соединений
void Server::handleConnections() {
    logger.logInfo("Starting to handle connections");
    
    while (running) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == -1) {
            if (running) {
                logger.logError("Error accepting connection");
            }
            continue;
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        
        std::string clientInfo = std::string(clientIP) + ":" + std::to_string(ntohs(clientAddr.sin_port));
        logger.logInfo("Accepted connection from: " + clientInfo + ", socket: " + std::to_string(clientSocket));

        handleClient(clientSocket);
    }
    
    logger.logInfo("Stopped handling connections");
}

// Обработка отдельного клиента
void Server::handleClient(int clientSocket) {
    logger.logInfo("Handling client on socket: " + std::to_string(clientSocket));
    
    char buffer[1024];
    ssize_t bytesRead;
    bool authenticated = false;
    
    bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::string request(buffer, bytesRead);
        
        logger.logInfo("Received authentication request from socket " + std::to_string(clientSocket) + 
                      ", length: " + std::to_string(bytesRead));
        
        std::string response = processAuthentication(request);
        
        send(clientSocket, response.c_str(), response.length(), 0);
        logger.logInfo("Sent authentication response to socket " + std::to_string(clientSocket) + 
                      ": " + response);
        
        if (response == "OK") {
            authenticated = true;
            logger.logInfo("Client authenticated successfully, socket: " + std::to_string(clientSocket));
        } else {
            logger.logError("Client authentication failed, socket: " + std::to_string(clientSocket));
            close(clientSocket);
            return;
        }
    } else {
        logger.logError("Empty authentication message from socket: " + std::to_string(clientSocket));
        close(clientSocket);
        return;
    }
    
    if (authenticated) {
        try {
            uint32_t numVectors = readUint32(clientSocket);
            logger.logInfo("Processing " + std::to_string(numVectors) + " vectors from socket: " + 
                          std::to_string(clientSocket));
            
            for (uint32_t i = 0; i < numVectors; i++) {
                uint32_t vectorSize = readUint32(clientSocket);
                logger.logInfo("Vector " + std::to_string(i) + " size: " + std::to_string(vectorSize) + 
                              ", socket: " + std::to_string(clientSocket));
                
                std::vector<uint64_t> vectorData;
                for (uint32_t j = 0; j < vectorSize; j++) {
                    uint64_t value = readUint64(clientSocket);
                    vectorData.push_back(value);
                }
                
                uint64_t result = processVector(vectorData);
                
                logger.logInfo("Vector " + std::to_string(i) + " processed, result: " + 
                              std::to_string(result) + ", socket: " + std::to_string(clientSocket));
                
                sendUint64(clientSocket, result);
            }
            
            logger.logInfo("All vectors processed for socket: " + std::to_string(clientSocket));
            
        } catch (const std::exception& e) {
            logger.logError("Error processing data from socket " + std::to_string(clientSocket) + 
                           ": " + e.what());
        }
    }
    
    close(clientSocket);
    logger.logInfo("Client connection closed, socket: " + std::to_string(clientSocket));
}

// Обработка аутентификации клиента
std::string Server::processAuthentication(const std::string& request) {
    logger.logInfo("Processing authentication request");
    
    if (request.find("user") != std::string::npos) {
        logger.logInfo("Authentication successful");
        return "OK";
    } else {
        logger.logError("Authentication failed - invalid request format");
        return "ERROR";
    }
}

// Обработка вектора данных
uint64_t Server::processVector(const std::vector<uint64_t>& data) {
    logger.logInfo("Processing vector with " + std::to_string(data.size()) + " elements");
    
    uint64_t sum_of_squares = 0;
    for (uint64_t value : data) {
        sum_of_squares += value * value;
    }
    
    logger.logInfo("Vector processing completed, sum of squares: " + std::to_string(sum_of_squares));
    
    return sum_of_squares;
}

// Настройка обработчиков сигналов - ИСПРАВЛЕННЫЙ (без logger)
void Server::setupSignalHandlers() {
    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    
    // Убираем вызов logger из статического метода
    // Вместо этого логируем в run() после вызова setupSignalHandlers()
}

// Обработчик сигналов
void signalHandler(int signal) {
    std::cout << "\nПолучен сигнал " << signal << ", остановка сервера..." << std::endl;
    if (serverInstance != nullptr) {
        serverInstance->stop();
    }
}

// Методы для работы с бинарными данными (оставляем как были)
uint32_t Server::readUint32(int clientSocket) {
    uint32_t value;
    ssize_t bytesRead = recv(clientSocket, &value, sizeof(value), MSG_WAITALL);
    if (bytesRead != sizeof(value)) {
        throw std::runtime_error("Не удалось прочитать uint32");
    }
    return value;
}

uint64_t Server::readUint64(int clientSocket) {
    uint64_t value;
    ssize_t bytesRead = recv(clientSocket, &value, sizeof(value), MSG_WAITALL);
    if (bytesRead != sizeof(value)) {
        throw std::runtime_error("Не удалось прочитать uint64");
    }
    return value;
}

void Server::sendUint32(int clientSocket, uint32_t value) {
    send(clientSocket, &value, sizeof(value), 0);
}

void Server::sendUint64(int clientSocket, uint64_t value) {
    send(clientSocket, &value, sizeof(value), 0);
}
