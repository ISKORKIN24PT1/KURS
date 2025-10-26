#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <atomic>
#include <vector>
#include <cstdint>

class Server {
public:
    Server(int port, const std::string& userFile, const std::string& logFile);
    ~Server();
    
    bool initialize();
    void run();
    void stop();
    bool isRunning() const;

private:
    static std::atomic<bool> running;
    int serverSocket;
    int port;
    std::string userFile;
    std::string logFile;
    
    void handleConnections();
    void handleClient(int clientSocket);
    std::string processAuthentication(const std::string& request);
    uint64_t processVector(const std::vector<uint64_t>& data);
    static void setupSignalHandlers();
    bool createSocket();
    
    // Функции для работы с бинарными данными
    uint32_t readUint32(int clientSocket);
    uint64_t readUint64(int clientSocket);
    void sendUint32(int clientSocket, uint32_t value);
    void sendUint64(int clientSocket, uint64_t value);
};

extern Server* serverInstance;
void signalHandler(int signal);

#endif
