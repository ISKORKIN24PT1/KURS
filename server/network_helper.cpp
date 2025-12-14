/**
 * @file network_helper.cpp
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Реализация класса NetworkHelper для сетевого взаимодействия.
 */

#include "network_helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>
#include <iostream>

/**
 * @brief Конструктор класса NetworkHelper.
 */
NetworkHelper::NetworkHelper() : serverSocketfd(-1), clientSocketfd(-1) {}

/**
 * @brief Деструктор класса NetworkHelper.
 */
NetworkHelper::~NetworkHelper() {
    closeClient();
    closeServer();
}

/**
 * @brief Создает сокет для сервера.
 * @return true, если сокет успешно создан, false в случае ошибки.
 */
bool NetworkHelper::createSocket() {
    serverSocketfd = socket(AF_INET, SOCK_STREAM, 0);
    return serverSocketfd != -1;
}

/**
 * @brief Привязывает сокет к указанному порту.
 * @param[in] port Номер порта для привязки.
 * @return true, если привязка успешна, false в случае ошибки.
 */
bool NetworkHelper::bindSocket(int port) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    return bind(serverSocketfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0;
}

/**
 * @brief Начинает прослушивание порта.
 * @return true, если операция успешна, false в случае ошибки.
 */
bool NetworkHelper::startListening() {
    return listen(serverSocketfd, 5) == 0; // backlog = 5
}

/**
 * @brief Принимает входящее соединение.
 * @return Дескриптор клиентского сокета, или -1 в случае ошибки.
 */
int NetworkHelper::acceptConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    return accept(serverSocketfd, (struct sockaddr*)&clientAddr, &clientLen);
}

/**
 * @brief Закрывает серверный сокет.
 * @return true, если операция успешна, false в случае ошибки.
 */
bool NetworkHelper::closeServer() {
    if (serverSocketfd != -1) {
        close(serverSocketfd);
        serverSocketfd = -1;
        return true;
    }
    return false;
}

/**
 * @brief Устанавливает клиентский сокет для сетевых операций.
 * @param[in] socket Дескриптор клиентского сокета.
 * @return true, если операция успешна, false в случае ошибки.
 */
bool NetworkHelper::setClientSocket(int socket) {
    clientSocketfd = socket;
    return true;
}

/**
 * @brief Закрывает клиентский сокет.
 * @return true, если операция успешна, false в случае ошибки.
 */
bool NetworkHelper::closeClient() {
    if (clientSocketfd != -1) {
        close(clientSocketfd);
        clientSocketfd = -1;
        return true;
    }
    return false;
}

/**
 * @brief Принимает произвольные данные через сокет.
 * @param[out] buffer Буфер для приема данных.
 * @param[in] size Ожидаемый размер данных в байтах.
 * @return true, если прием успешен, false в случае ошибки.
 */
bool NetworkHelper::receiveData(void* buffer, size_t size) {
    if (clientSocketfd == -1) return false;
    
    char* buf = static_cast<char*>(buffer);
    size_t totalReceived = 0;
    
    while (totalReceived < size) {
        ssize_t received = recv(clientSocketfd, buf + totalReceived, size - totalReceived, 0);
        if (received <= 0) {
            if (received == 0) {
                std::cerr << "DEBUG: Connection closed by client" << std::endl;
            } else {
                std::cerr << "DEBUG: recv error: " << strerror(errno) << std::endl;
            }
            return false;
        }
        totalReceived += received;
    }
    
    return true;
}

/**
 * @brief Отправляет произвольные данные через сокет.
 * @param[in] data Указатель на данные для отправки.
 * @param[in] size Размер данных в байтах.
 * @return true, если отправка успешна, false в случае ошибки.
 */
bool NetworkHelper::sendData(const void* data, size_t size) {
    if (clientSocketfd == -1) return false;
    
    const char* buf = static_cast<const char*>(data);
    size_t totalSent = 0;
    
    while (totalSent < size) {
        ssize_t sent = send(clientSocketfd, buf + totalSent, size - totalSent, 0);
        if (sent <= 0) {
            std::cerr << "DEBUG: send error: " << strerror(errno) << std::endl;
            return false;
        }
        totalSent += sent;
    }
    
    return true;
}

/**
 * @brief Отправляет строку через сокет.
 * @param[in] str Строка для отправки.
 * @return true, если отправка успешна, false в случае ошибки.
 */
bool NetworkHelper::sendString(const std::string& str) {
    return sendData(str.c_str(), str.length());
}

/**
 * @brief Принимает строку через сокет.
 * @param[in] maxLength Максимальная длина принимаемой строки.
 * @return Принятая строка.
 */
std::string NetworkHelper::receiveString(size_t maxLength) {
    if (clientSocketfd == -1) return "";
    
    std::vector<char> buffer(maxLength);
    ssize_t received = recv(clientSocketfd, buffer.data(), maxLength, 0);
    
    if (received > 0) {
        return std::string(buffer.data(), received);
    }
    return "";
}

/**
 * @brief Принимает 32-битное целое число.
 * @param[out] value Переменная для сохранения принятого числа.
 * @return true, если прием успешен, false в случае ошибки.
 */
bool NetworkHelper::receiveUint32(uint32_t& value) {
    // Просто читаем как есть
    if (!receiveData(&value, sizeof(value))) {
        return false;
    }
    
    // Отладочный вывод
    std::cout << "DEBUG receiveUint32: value = " << value 
              << " (hex: 0x" << std::hex << value << std::dec << ")" << std::endl;
    
    return true;
}

/**
 * @brief Отправляет 32-битное целое число.
 * @param[in] value Число для отправки.
 * @return true, если отправка успешна, false в случае ошибки.
 */
bool NetworkHelper::sendUint32(uint32_t value) {
    // Отправляем как есть
    std::cout << "DEBUG sendUint32: sending value = " << value << std::endl;
    return sendData(&value, sizeof(value));
}

/**
 * @brief Принимает 64-битное целое число.
 * @param[out] value Переменная для сохранения принятого числа.
 * @return true, если прием успешен, false в случае ошибки.
 */
bool NetworkHelper::receiveUint64(uint64_t& value) {
    if (!receiveData(&value, sizeof(value))) {
        return false;
    }
    
    // Отладочный вывод
    std::cout << "DEBUG receiveUint64: value = " << value << std::endl;
    
    return true;
}

/**
 * @brief Отправляет 64-битное целое число.
 * @param[in] value Число для отправки.
 * @return true, если отправка успешна, false в случае ошибки.
 */
bool NetworkHelper::sendUint64(uint64_t value) {
    std::cout << "DEBUG sendUint64: sending value = " << value << std::endl;
    return sendData(&value, sizeof(value));
}

/**
 * @brief Отправляет вектор 64-битных целых чисел.
 * @param[in] vector Вектор для отправки.
 * @return true, если отправка успешна, false в случае ошибки.
 */
bool NetworkHelper::sendVector(const std::vector<uint64_t>& vector) {
    for (uint64_t value : vector) {
        if (!sendUint64(value)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Принимает вектор 64-битных целых чисел.
 * @param[out] vector Вектор для сохранения принятых данных.
 * @param[in] size Ожидаемое количество элементов в векторе.
 * @return true, если прием успешен, false в случае ошибки.
 */
bool NetworkHelper::receiveVector(std::vector<uint64_t>& vector, uint32_t size) {
    vector.resize(size);
    for (uint32_t i = 0; i < size; i++) {
        if (!receiveUint64(vector[i])) {
            return false;
        }
        // Отладочный вывод первых нескольких элементов
        if (i < 3 && size > 1) {
            std::cout << "DEBUG receiveVector: element[" << i << "] = " << vector[i] << std::endl;
        }
    }
    if (size > 0) {
        std::cout << "DEBUG: Received vector with " << size << " elements" << std::endl;
    }
    return true;
}
