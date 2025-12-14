/**
 * @file server.cpp
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Реализация класса Server для управления серверным приложением.
 */

#include "server.h"
#include "authenticator.h"
#include "client_handler.h"
#include "calculator.h"
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
#include <fstream>

// Глобальный указатель на экземпляр сервера для обработки сигналов
Server* serverInstance = nullptr;
// Атомарный флаг состояния работы сервера
std::atomic<bool> Server::running(false);

/**
 * @brief Конструктор класса Server.
 * @param[in] port Номер порта для работы сервера.
 * @param[in] userFile Путь к файлу с базой пользователей.
 * @param[in] logFile Путь к файлу журнала работы.
 */
Server::Server(int port, const std::string& userFile, const std::string& logFile) 
    : serverSocket(-1), port(port), userFile(userFile), logFile(logFile), logger(logFile) {
    serverInstance = this;
    logger.logInfo("Server instance created with port: " + std::to_string(port) + 
                  ", user file: " + userFile + ", log file: " + logFile);
}

/**
 * @brief Деструктор класса Server.
 */
Server::~Server() {
    logger.logInfo("Server destructor called");
    stop();
    if (serverSocket != -1) {
        close(serverSocket);
    }
}

/**
 * @brief Инициализирует сервер.
 * @return true, если инициализация успешна, false в случае ошибки.
 */
bool Server::initialize() {
    logger.logInfo("Initializing server...");
    
    // Проверяем доступность файла пользователей
    std::ifstream userTest(userFile);
    if (!userTest.is_open()) {
        logger.logError("Cannot open users file: " + userFile);
        std::cerr << "ERROR: Cannot open users file: " << userFile << std::endl;
        return false;
    }
    userTest.close();
    logger.logInfo("Users file verified: " + userFile);
    
    // Загружаем пользователей и проверяем что загружены
    Authenticator auth;
    if (!auth.loadUsersFromFile(userFile)) {
        logger.logError("Failed to load valid users from file: " + userFile);
        std::cerr << "ERROR: Failed to load valid users from file: " + userFile << std::endl;
        std::cerr << "File format must be: login password (separated by space or tab)" << std::endl;
        return false;
    }
    
    return createSocket();
}

/**
 * @brief Создает серверный сокет.
 * @return true, если сокет успешно создан, false в случае ошибки.
 */
bool Server::createSocket() {
    logger.logInfo("Creating server socket...");
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        logger.logError("Error creating socket");
        std::cerr << "ERROR: Error creating socket" << std::endl;
        return false;
    }
    logger.logInfo("Socket created successfully");

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        logger.logError("Error setting socket options");
        std::cerr << "ERROR: Error setting socket options" << std::endl;
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
        std::cerr << "ERROR: Error binding socket to port " << port << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    logger.logInfo("Socket bound to port " + std::to_string(port));
    return true;
}

/**
 * @brief Запускает сервер и начинает обработку подключений.
 */
void Server::run() {
    if (listen(serverSocket, 10) == -1) {
        logger.logError("Error starting listening");
        std::cerr << "ERROR: Error starting listening" << std::endl;
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

/**
 * @brief Останавливает сервер.
 */
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

/**
 * @brief Обрабатывает входящие подключения.
 */
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

/**
 * @brief Обрабатывает отдельного клиента.
 * @param[in] clientSocket Дескриптор клиентского сокета.
 */
void Server::handleClient(int clientSocket) {
    logger.logInfo("Handling client on socket: " + std::to_string(clientSocket));
    
    // Создаём необходимые объекты для обработки клиента
    Authenticator auth;
    if (!auth.loadUsersFromFile(userFile)) {
        logger.logError("Failed to load users for client, socket: " + std::to_string(clientSocket));
        close(clientSocket);
        return;
    }
    
    Calculator calc;
    ClientHandler handler(clientSocket, auth, calc, logger);
    handler.handleClient();
    
    // ClientHandler сам закрывает сокет, но на всякий случай
    close(clientSocket);
    logger.logInfo("Client connection closed, socket: " + std::to_string(clientSocket));
}

/**
 * @brief Устанавливает обработчики сигналов.
 */
void Server::setupSignalHandlers() {
    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

/**
 * @brief Обработчик сигналов.
 * @param[in] signal Номер полученного сигнала.
 */
void signalHandler(int signal) {
    std::cout << "\nПолучен сигнал " << signal << ", остановка сервера..." << std::endl;
    if (serverInstance != nullptr) {
        serverInstance->stop();
    }
}

/**
 * @brief Проверяет состояние работы сервера.
 * @return true, если сервер работает, false если остановлен.
 */
bool Server::isRunning() const {
    return running;
}
