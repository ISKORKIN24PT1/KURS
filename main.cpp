#include "server.h"
#include <iostream>
#include <csignal>
#include <cstring>

// Глобальные переменные ТОЛЬКО здесь
Server* serverInstance = nullptr;

void signalHandler(int signal) {
    if (serverInstance && signal == SIGINT) {
        std::cout << "\nReceived SIGINT, shutting down..." << std::endl;
        serverInstance->stop();
    }
}

int main(int argc, char* argv[]) {
    // Обработка аргументов командной строки
    std::string usersFile = "users.txt";
    std::string logFile = "server.log";
    int port = 33333;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cout << "Network Server Help:" << std::endl;
            std::cout << "  --users <file>    Users database file (required)" << std::endl;
            std::cout << "  --log <file>      Log file (required)" << std::endl;
            std::cout << "  --port <number>   Port number (required)" << std::endl;
            std::cout << "  -h, --help        Show this help" << std::endl;
            return 0;
        }
        else if (strcmp(argv[i], "--users") == 0 && i + 1 < argc) {
            usersFile = argv[++i];
        }
        else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc) {
            logFile = argv[++i];
        }
        else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }
    
    // Создаем и инициализируем сервер
    Server server(port, usersFile, logFile);
    serverInstance = &server;
    
    // Устанавливаем обработчик сигналов
    signal(SIGINT, signalHandler);
    
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    std::cout << "Server starting on port " << port << std::endl;
    std::cout << "Users file: " << usersFile << std::endl;
    std::cout << "Log file: " << logFile << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    // Запускаем сервер
    server.run();
    
    std::cout << "Server stopped" << std::endl;
    return 0;
}
