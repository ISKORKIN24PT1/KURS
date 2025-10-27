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
    : serverSocket(-1), port(port), userFile(userFile), logFile(logFile) {
    serverInstance = this; // Сохраняем указатель на текущий экземпляр
}

// Деструктор сервера - останавливаем сервер и закрываем сокет
Server::~Server() {
    stop();
    if (serverSocket != -1) {
        close(serverSocket);
    }
}

// Инициализация сервера
bool Server::initialize() {
    std::cout << "Инициализация сервера..." << std::endl;
    return createSocket(); // Создаем и настраиваем сокет
}

// Создание и настройка серверного сокета
bool Server::createSocket() {
    // Создаем TCP сокет
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return false;
    }

    // Настраиваем опцию повторного использования адреса
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Ошибка настройки сокета" << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    // Настраиваем адрес сервера
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;           // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;   // Принимаем соединения с любого интерфейса
    serverAddr.sin_port = htons(port);         // Порт в сетевом порядке байт

    // Привязываем сокет к адресу
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Ошибка привязки сокета к порту " << port << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    return true;
}

// Основной цикл работы сервера
void Server::run() {
    // Переводим сокет в режим прослушивания (максимум 10 соединений в очереди)
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Ошибка прослушивания" << std::endl;
        return;
    }

    setupSignalHandlers(); // Настраиваем обработчики сигналов
    running = true;        // Устанавливаем флаг работы
    
    std::cout << "Сервер запущен на порту " << port << std::endl;
    std::cout << "Для остановки нажмите Ctrl+C" << std::endl;

    handleConnections();   // Начинаем обработку входящих соединений
}

// Остановка сервера
void Server::stop() {
    if (running) {
        running = false; // Сбрасываем флаг работы
        std::cout << "Остановка сервера..." << std::endl;
        
        if (serverSocket != -1) {
            close(serverSocket); // Закрываем серверный сокет
            serverSocket = -1;
        }
    }
}

// Проверка состояния сервера
bool Server::isRunning() const {
    return running;
}

// Обработка входящих соединений
void Server::handleConnections() {
    while (running) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        
        // Принимаем новое соединение
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == -1) {
            if (running) {
                std::cerr << "Ошибка принятия соединения" << std::endl;
            }
            continue;
        }

        // Получаем информацию о клиенте
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cout << "Принято соединение от " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // Обрабатываем клиента
        handleClient(clientSocket);
    }
}

// Обработка отдельного клиента
void Server::handleClient(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;
    bool authenticated = false;
    
    // Фаза аутентификации
    bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Получено " << bytesRead << " байт от клиента: " << buffer << std::endl;
        
        std::string request(buffer, bytesRead);
        std::string response = processAuthentication(request); // Обрабатываем аутентификацию
        
        send(clientSocket, response.c_str(), response.length(), 0);
        std::cout << "Отправлено клиенту: " << response << std::endl;
        
        if (response == "OK") {
            authenticated = true;
            std::cout << "Клиент аутентифицирован, ожидаем данные для вычислений..." << std::endl;
        } else {
            close(clientSocket);
            return;
        }
    }
    
    // Если аутентификация успешна, обрабатываем данные для вычислений
    if (authenticated) {
        try {
            // Читаем количество векторов (4 байта)
            uint32_t numVectors = readUint32(clientSocket);
            std::cout << "Количество векторов: " << numVectors << std::endl;
            
            // Обрабатываем каждый вектор
            for (uint32_t i = 0; i < numVectors; i++) {
                // Читаем размер вектора
                uint32_t vectorSize = readUint32(clientSocket);
                std::cout << "Размер вектора " << i << ": " << vectorSize << std::endl;
                
                // Читаем данные вектора
                std::vector<uint64_t> vectorData;
                for (uint32_t j = 0; j < vectorSize; j++) {
                    uint64_t value = readUint64(clientSocket);
                    vectorData.push_back(value);
                }
                
                // Логируем первые 5 элементов вектора для отладки
                std::cout << "Вектор " << i << " прочитан: ";
                for (size_t j = 0; j < std::min(vectorData.size(), size_t(5)); j++) {
                    std::cout << vectorData[j] << " ";
                }
                if (vectorData.size() > 5) std::cout << "...";
                std::cout << std::endl;
                
                // Вычисляем сумму квадратов элементов вектора
                uint64_t result = processVector(vectorData);
                
                std::cout << "Отправляем результат " << result << " для вектора " << i << std::endl;
                
                // Отправляем результат клиенту
                sendUint64(clientSocket, result);
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Ошибка при обработке данных: " << e.what() << std::endl;
        }
    }
    
    close(clientSocket);
    std::cout << "Соединение с клиентом закрыто" << std::endl;
}

// Чтение 32-битного беззнакового целого из сокета
uint32_t Server::readUint32(int clientSocket) {
    uint32_t value;
    ssize_t bytesRead = recv(clientSocket, &value, sizeof(value), MSG_WAITALL);
    if (bytesRead != sizeof(value)) {
        throw std::runtime_error("Не удалось прочитать uint32");
    }
    // БЕЗ ПРЕОБРАЗОВАНИЙ - используем как есть (данные уже в нужном формате)
    return value;
}

// Чтение 64-битного беззнакового целого из сокета
uint64_t Server::readUint64(int clientSocket) {
    uint64_t value;
    ssize_t bytesRead = recv(clientSocket, &value, sizeof(value), MSG_WAITALL);
    if (bytesRead != sizeof(value)) {
        throw std::runtime_error("Не удалось прочитать uint64");
    }
    // БЕЗ ПРЕОБРАЗОВАНИЙ - используем как есть (данные уже в нужном формате)
    return value;
}

// Отправка 32-битного беззнакового целого в сокет
void Server::sendUint32(int clientSocket, uint32_t value) {
    // БЕЗ ПРЕОБРАЗОВАНИЙ - отправляем как есть
    send(clientSocket, &value, sizeof(value), 0);
}

// Отправка 64-битного беззнакового целого в сокет
void Server::sendUint64(int clientSocket, uint64_t value) {
    // БЕЗ ПРЕОБРАЗОВАНИЙ - отправляем как есть
    send(clientSocket, &value, sizeof(value), 0);
}

// Обработка аутентификации клиента
std::string Server::processAuthentication(const std::string& request) {
    // Упрощенная аутентификация - проверяем наличие строки "user" в запросе
    if (request.find("user") != std::string::npos) {
        std::cout << "Аутентификация успешна" << std::endl;
        return "OK";
    } else {
        std::cout << "Аутентификация failed" << std::endl;
        return "ERROR";
    }
}

// Обработка вектора данных - вычисление суммы квадратов
uint64_t Server::processVector(const std::vector<uint64_t>& data) {
    // Вычисляем сумму квадратов элементов
    uint64_t sum_of_squares = 0;
    for (uint64_t value : data) {
        sum_of_squares += value * value;
    }
    
    std::cout << "Обработка вектора: сумма квадратов=" << sum_of_squares << ", элементов=" << data.size() << std::endl;
    
    return sum_of_squares;
}

// Настройка обработчиков сигналов для graceful shutdown
void Server::setupSignalHandlers() {
    struct sigaction sa{};
    sa.sa_handler = signalHandler;  // Устанавливаем обработчик
    sigemptyset(&sa.sa_mask);       // Очищаем маску сигналов
    sa.sa_flags = 0;                // Без специальных флагов

    // Устанавливаем обработчики для SIGINT (Ctrl+C) и SIGTERM
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

// Обработчик сигналов для graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nПолучен сигнал " << signal << ", остановка сервера..." << std::endl;
    if (serverInstance != nullptr) {
        serverInstance->stop(); // Останавливаем сервер через экземпляр
    }
}
