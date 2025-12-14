/**
 * @file client_handler.h
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Заголовочный файл класса ClientHandler для обработки клиентских подключений.
 * @details Класс отвечает за обработку отдельного клиентского подключения,
 *          включая аутентификацию и обработку векторов данных.
 * @warning Создается для каждого нового клиента и управляет всем циклом взаимодействия.
 */

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "network_helper.h"
#include "authenticator.h"
#include "calculator.h"
#include "logger.h"
#include <memory>

/**
 * @brief Класс для обработки отдельного клиентского подключения.
 * 
 * @details Класс ClientHandler создается для каждого нового клиента и управляет 
 *          всем циклом взаимодействия: аутентификация, прием векторов данных, 
 *          выполнение вычислений и отправка результатов.
 *          Содержит ссылки на основные модули системы для выполнения своих функций.
 */
class ClientHandler {
private:
    /**
     * @brief Дескриптор клиентского сокета.
     */
    int clientSocket;
    
    /**
     * @brief Ссылка на объект аутентификации.
     */
    Authenticator& authenticator;
    
    /**
     * @brief Ссылка на объект вычислений.
     */
    Calculator& calculator;
    
    /**
     * @brief Ссылка на объект логирования.
     */
    Logger& logger;
    
    /**
     * @brief Умный указатель на объект сетевого взаимодействия.
     */
    std::unique_ptr<NetworkHelper> network;
    
public:
    /**
     * @brief Конструктор класса ClientHandler.
     * @param[in] socket Дескриптор клиентского сокета.
     * @param[in] auth Ссылка на объект аутентификации.
     * @param[in] calc Ссылка на объект вычислений.
     * @param[in] log Ссылка на объект логирования.
     */
    ClientHandler(int socket, Authenticator& auth, Calculator& calc, Logger& log);
    
    /**
     * @brief Основной метод обработки клиента.
     * @details Выполняет полный цикл обработки клиента:
     *          1. Создание NetworkHelper.
     *          2. Аутентификация клиента.
     *          3. Обработка векторов данных (если аутентификация успешна).
     *          4. Закрытие соединения.
     *          В случае ошибок вызывает sendErrorAndClose.
     */
    void handleClient();
    
private:
    /**
     * @brief Выполняет аутентификацию клиента.
     * @return true, если аутентификация успешна, false в противном случае.
     */
    bool authenticateClient();
    
    /**
     * @brief Обрабатывает векторы данных от клиента.
     * @return true, если обработка успешна, false в случае ошибки.
     */
    bool processVectors();
    
    /**
     * @brief Отправляет сообщение об ошибке и закрывает соединение.
     * @param[in] error Текст сообщения об ошибке.
     */
    void sendErrorAndClose(const std::string& error);
};

#endif
