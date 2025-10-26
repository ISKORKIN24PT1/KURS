#ifndef SERVER_H
#define SERVER_H

#include "logger.h"
#include "authenticator.h"
#include "calculator.h"
#include "network_helper.h"
#include <atomic>
#include <thread>
#include <string>

// Предварительное объявление чтобы избежать циклических зависимостей
class ClientHandler;

// Объявляем extern для глобальных переменных
extern class Server* serverInstance;
void signalHandler(int signal);

class Server {
private:
    int port;
    std::string usersFile;
    std::string logFile;
    std::atomic<bool> running;
    
    Logger logger;
    Authenticator authenticator;
    Calculator calculator;
    NetworkHelper network;
    
public:
    Server(int port, const std::string& usersFile, const std::string& logFile);
    ~Server();
    
    bool initialize();
    void run();
    void stop();
    void showHelp();
    
private:
    void handleCommandLineArgs(int argc, char* argv[]);
};

#endif
