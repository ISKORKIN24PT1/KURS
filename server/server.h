/**
 * @file server.h
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Заголовочный файл класса Server для управления серверным приложением.
 * @details Класс является центральным компонентом приложения, отвечающим за
 *          запуск и управление работой серверной части системы.
 */

#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <atomic>
#include <vector>
#include <cstdint>
#include "logger.h"  

/**
 * @brief Класс для управления серверным приложением.
 * 
 * @details Класс Server реализует логику сетевого взаимодействия, обработки
 *          клиентских подключений и управления жизненным циклом сервера.
 */
class Server {
public:
    /**
     * @brief Конструктор класса Server.
     * @param[in] port Номер порта для работы сервера.
     * @param[in] userFile Путь к файлу с базой пользователей.
     * @param[in] logFile Путь к файлу журнала работы.
     */
    Server(int port, const std::string& userFile, const std::string& logFile);
    
    /**
     * @brief Деструктор класса Server.
     */
    ~Server();
    
    /**
     * @brief Инициализирует сервер.
     * @return true, если инициализация успешна, false в случае ошибки.
     */
    bool initialize();
    
    /**
     * @brief Запускает сервер и начинает обработку подключений.
     */
    void run();
    
    /**
     * @brief Останавливает сервер.
     */
    void stop();
    
    /**
     * @brief Проверяет состояние работы сервера.
     * @return true, если сервер работает, false если остановлен.
     */
    bool isRunning() const;

private:
    /**
     * @brief Атомарный флаг состояния работы сервера.
     */
    static std::atomic<bool> running;
    
    /**
     * @brief Дескриптор серверного сокета.
     */
    int serverSocket;
    
    /**
     * @brief Номер порта, на котором работает сервер.
     */
    int port;
    
    /**
     * @brief Путь к файлу с базой пользователей.
     */
    std::string userFile;
    
    /**
     * @brief Путь к файлу журнала работы.
     */
    std::string logFile;
    
    /**
     * @brief Объект для ведения журнала событий.
     */
    Logger logger;
    
    /**
     * @brief Обрабатывает входящие подключения.
     */
    void handleConnections();
    
    /**
     * @brief Обрабатывает отдельного клиента.
     * @param[in] clientSocket Дескриптор клиентского сокета.
     */
    void handleClient(int clientSocket);
    
    /**
     * @brief Создает серверный сокет.
     * @return true, если сокет успешно создан, false в случае ошибки.
     */
    bool createSocket();
    
    /**
     * @brief Устанавливает обработчики сигналов.
     */
    static void setupSignalHandlers();
};

/**
 * @brief Глобальный указатель на экземпляр сервера.
 */
extern Server* serverInstance;

/**
 * @brief Обработчик сигналов.
 * @param[in] signal Номер полученного сигнала.
 */
void signalHandler(int signal);

#endif
