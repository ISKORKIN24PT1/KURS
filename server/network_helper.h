/**
 * @file network_helper.h
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Заголовочный файл класса NetworkHelper для сетевого взаимодействия.
 * @details Класс инкапсулирует низкоуровневые операции сетевого взаимодействия,
 *          обеспечивая отправку и прием данных различных типов.
 */

#ifndef NETWORK_HELPER_H
#define NETWORK_HELPER_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Класс для управления сетевыми операциями.
 * 
 * @details Инкапсулирует низкоуровневые операции сетевого взаимодействия,
 *          обеспечивая отправку и прием данных различных типов. Поддерживает
 *          как серверные, так и клиентские функции.
 */
class NetworkHelper {
private:
    /**
     * @brief Дескриптор серверного сокета.
     */
    int serverSocketfd;
    
    /**
     * @brief Дескриптор клиентского сокета.
     */
    int clientSocketfd;
    
public:
    /**
     * @brief Конструктор класса NetworkHelper.
     */
    NetworkHelper();
    
    /**
     * @brief Деструктор класса NetworkHelper.
     */
    ~NetworkHelper();
    
    // Серверные методы
    
    /**
     * @brief Создает сокет для сервера.
     * @return true, если сокет успешно создан, false в случае ошибки.
     */
    bool createSocket();
    
    /**
     * @brief Привязывает сокет к указанному порту.
     * @param[in] port Номер порта для привязки.
     * @return true, если привязка успешна, false в случае ошибки.
     */
    bool bindSocket(int port);
    
    /**
     * @brief Начинает прослушивание порта.
     * @return true, если операция успешна, false в случае ошибки.
     */
    bool startListening();
    
    /**
     * @brief Принимает входящее соединение.
     * @return Дескриптор клиентского сокета, или -1 в случае ошибки.
     */
    int acceptConnection();
    
    /**
     * @brief Закрывает серверный сокет.
     * @return true, если операция успешна, false в случае ошибки.
     */
    bool closeServer();
    
    // Клиентские методы
    
    /**
     * @brief Устанавливает клиентский сокет для сетевых операций.
     * @param[in] socket Дескриптор клиентского сокета.
     * @return true, если операция успешна, false в случае ошибки.
     */
    bool setClientSocket(int socket);
    
    /**
     * @brief Закрывает клиентский сокет.
     * @return true, если операция успешна, false в случае ошибки.
     */
    bool closeClient();
    
    // Базовые функции отправки/приема
    
    /**
     * @brief Отправляет произвольные данные через сокет.
     * @param[in] data Указатель на данные для отправки.
     * @param[in] size Размер данных в байтах.
     * @return true, если отправка успешна, false в случае ошибки.
     */
    bool sendData(const void* data, size_t size);
    
    /**
     * @brief Принимает произвольные данные через сокет.
     * @param[out] buffer Буфер для приема данных.
     * @param[in] size Ожидаемый размер данных в байтах.
     * @return true, если прием успешен, false в случае ошибки.
     */
    bool receiveData(void* buffer, size_t size);
    
    // Специализированные функции для протокола
    
    /**
     * @brief Отправляет строку через сокет.
     * @param[in] str Строка для отправки.
     * @return true, если отправка успешна, false в случае ошибки.
     */
    bool sendString(const std::string& str);
    
    /**
     * @brief Принимает строку через сокет.
     * @param[in] maxLength Максимальная длина принимаемой строки.
     * @return Принятая строка.
     */
    std::string receiveString(size_t maxLength = 1024);
    
    /**
     * @brief Отправляет 32-битное целое число.
     * @param[in] value Число для отправки.
     * @return true, если отправка успешна, false в случае ошибки.
     */
    bool sendUint32(uint32_t value);
    
    /**
     * @brief Принимает 32-битное целое число.
     * @param[out] value Переменная для сохранения принятого числа.
     * @return true, если прием успешен, false в случае ошибки.
     */
    bool receiveUint32(uint32_t& value);
    
    /**
     * @brief Отправляет 64-битное целое число.
     * @param[in] value Число для отправки.
     * @return true, если отправка успешна, false в случае ошибки.
     */
    bool sendUint64(uint64_t value);
    
    /**
     * @brief Принимает 64-битное целое число.
     * @param[out] value Переменная для сохранения принятого числа.
     * @return true, если прием успешен, false в случае ошибки.
     */
    bool receiveUint64(uint64_t& value);
    
    /**
     * @brief Отправляет вектор 64-битных целых чисел.
     * @param[in] vector Вектор для отправки.
     * @return true, если отправка успешна, false в случае ошибки.
     */
    bool sendVector(const std::vector<uint64_t>& vector);
    
    /**
     * @brief Принимает вектор 64-битных целых чисел.
     * @param[out] vector Вектор для сохранения принятых данных.
     * @param[in] size Ожидаемое количество элементов в векторе.
     * @return true, если прием успешен, false в случае ошибки.
     */
    bool receiveVector(std::vector<uint64_t>& vector, uint32_t size);
};

#endif
