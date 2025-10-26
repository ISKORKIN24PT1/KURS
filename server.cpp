#include "server.h"
#include "client_handler.h"
#include <iostream>
#include <csignal>
#include <cstring>
#include <unistd.h>

// УБИРАЕМ глобальные переменные отсюда - они теперь только в main.cpp
// Server* serverInstance = nullptr;
// void signalHandler(int signal) { ... }

Server::Server(int port, const std::string& usersFile, const std::string& logFile)
    : port(port), usersFile(usersFile), logFile(logFile), 
      running(false), logger(logFile) {
}

Server::~Server() {
    stop();
}

bool Server::initialize() {
    // Загружаем базу пользователей
    if (!authenticator.loadUsersFromFile(usersFile)) {
        std::cerr << "ERROR: Cannot load users file: " << usersFile << std::endl;
        return false;
    }
    
    // Создаем сокет
    if (!network.createSocket()) {
        logger.logError("Cannot create socket");
        return false;
    }
    
    // Биндим сокет
    if (!network.bindSocket(port)) {
        logger.logError("Cannot bind socket to port " + std::to_string(port));
        return false;
    }
    
    // Начинаем слушать
    if (!network.startListening()) {
        logger.logError("Cannot start listening on port " + std::to_string(port));
        return false;
    }
    
    logger.logInfo("Server initialized successfully on port " + std::to_string(port));
    return true;
}

void Server::run() {
    running = true;
    logger.logInfo("Server started");
    
    while (running) {
        // Принимаем соединение
        int clientSocket = network.acceptConnection();
        if (clientSocket == -1) {
            if (running) {
                logger.logError("Error accepting connection");
            }
            continue;
        }
        
        logger.logInfo("New client connected, socket: " + std::to_string(clientSocket));
        
        // Обрабатываем клиента (в этом потоке, т.к. однопоточный режим)
        ClientHandler handler(clientSocket, authenticator, calculator, logger);
        handler.handleClient();
        
        close(clientSocket);
        logger.logInfo("Client connection closed");
    }
}

void Server::stop() {
    running = false;
    network.closeServer();
    logger.logInfo("Server stopped");
}

void Server::showHelp() {
    std::cout << "Usage: ./server --users <users_file> --log <log_file> --port <port_number>" << std::endl;
    std::cout << "Example: ./server --users users.txt --log server.log --port 33333" << std::endl;
    std::cout << "Help: ./server -h or --help" << std::endl;
}
