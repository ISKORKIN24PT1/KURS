#include "network_helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

NetworkHelper::NetworkHelper() : serverSocketfd(-1), clientSocketfd(-1) {}

NetworkHelper::~NetworkHelper() {
    closeClient();
    closeServer();
}

bool NetworkHelper::createSocket() {
    serverSocketfd = socket(AF_INET, SOCK_STREAM, 0);
    return serverSocketfd != -1;
}

bool NetworkHelper::bindSocket(int port) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    return bind(serverSocketfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0;
}

bool NetworkHelper::startListening() {
    return listen(serverSocketfd, 5) == 0; // backlog = 5
}

int NetworkHelper::acceptConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    return accept(serverSocketfd, (struct sockaddr*)&clientAddr, &clientLen);
}

bool NetworkHelper::closeServer() {
    if (serverSocketfd != -1) {
        close(serverSocketfd);
        serverSocketfd = -1;
        return true;
    }
    return false;
}

bool NetworkHelper::setClientSocket(int socket) {
    clientSocketfd = socket;
    return true;
}

bool NetworkHelper::closeClient() {
    if (clientSocketfd != -1) {
        close(clientSocketfd);
        clientSocketfd = -1;
        return true;
    }
    return false;
}

bool NetworkHelper::sendData(const void* data, size_t size) {
    if (clientSocketfd == -1) return false;
    return send(clientSocketfd, data, size, 0) == static_cast<ssize_t>(size);
}

bool NetworkHelper::receiveData(void* buffer, size_t size) {
    if (clientSocketfd == -1) return false;
    return recv(clientSocketfd, buffer, size, 0) == static_cast<ssize_t>(size);
}

bool NetworkHelper::sendString(const std::string& str) {
    return sendData(str.c_str(), str.length());
}

std::string NetworkHelper::receiveString(size_t maxLength) {
    if (clientSocketfd == -1) return "";
    
    std::vector<char> buffer(maxLength);
    ssize_t received = recv(clientSocketfd, buffer.data(), maxLength, 0);
    
    if (received > 0) {
        return std::string(buffer.data(), received);
    }
    return "";
}

bool NetworkHelper::sendUint32(uint32_t value) {
    uint32_t networkValue = htonl(value);
    return sendData(&networkValue, sizeof(networkValue));
}

bool NetworkHelper::receiveUint32(uint32_t& value) {
    uint32_t networkValue;
    if (receiveData(&networkValue, sizeof(networkValue))) {
        value = ntohl(networkValue);
        return true;
    }
    return false;
}

bool NetworkHelper::sendUint64(uint64_t value) {
    // Простая реализация для little-endian систем
    uint64_t networkValue = value;
    return sendData(&networkValue, sizeof(networkValue));
}

bool NetworkHelper::receiveUint64(uint64_t& value) {
    uint64_t networkValue;
    if (receiveData(&networkValue, sizeof(networkValue))) {
        value = networkValue;
        return true;
    }
    return false;
}

bool NetworkHelper::sendVector(const std::vector<uint64_t>& vector) {
    for (uint64_t value : vector) {
        if (!sendUint64(value)) {
            return false;
        }
    }
    return true;
}

bool NetworkHelper::receiveVector(std::vector<uint64_t>& vector, uint32_t size) {
    vector.resize(size);
    for (uint32_t i = 0; i < size; i++) {
        if (!receiveUint64(vector[i])) {
            return false;
        }
    }
    return true;
}
