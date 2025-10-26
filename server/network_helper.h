#ifndef NETWORK_HELPER_H
#define NETWORK_HELPER_H

#include <string>
#include <vector>
#include <cstdint>

class NetworkHelper {
private:
    int serverSocketfd;
    int clientSocketfd;
    
public:
    NetworkHelper();
    ~NetworkHelper();
    
    // Серверные методы
    bool createSocket();
    bool bindSocket(int port);
    bool startListening();
    int acceptConnection();
    bool closeServer();
    
    // Клиентские методы
    bool setClientSocket(int socket);
    bool closeClient();
    
    // Базовые функции отправки/приема
    bool sendData(const void* data, size_t size);
    bool receiveData(void* buffer, size_t size);
    
    // Специализированные функции для протокола
    bool sendString(const std::string& str);
    std::string receiveString(size_t maxLength = 1024);
    
    bool sendUint32(uint32_t value);
    bool receiveUint32(uint32_t& value);
    
    bool sendUint64(uint64_t value);
    bool receiveUint64(uint64_t& value);
    
    bool sendVector(const std::vector<uint64_t>& vector);
    bool receiveVector(std::vector<uint64_t>& vector, uint32_t size);
};

#endif
