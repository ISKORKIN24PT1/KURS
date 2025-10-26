#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "network_helper.h"
#include "authenticator.h"
#include "calculator.h"
#include "logger.h"
#include <memory>

class ClientHandler {
private:
    // Изменяем порядок объявления чтобы соответствовать конструктору
    int clientSocket;
    Authenticator& authenticator;
    Calculator& calculator;
    Logger& logger;
    std::unique_ptr<NetworkHelper> network;
    
public:
    ClientHandler(int socket, Authenticator& auth, Calculator& calc, Logger& log);
    void handleClient();
    
private:
    bool authenticateClient();
    bool processVectors();
    void sendErrorAndClose(const std::string& error);
};

#endif
