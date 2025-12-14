/**
 * @file logger.h
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Заголовочный файл класса Logger для ведения журнала работы программы.
 * @details Класс обеспечивает потокобезопасную запись сообщений в файл журнала
 *          с указанием даты, времени и уровня важности.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

/**
 * @brief Класс для ведения журнала работы программы.
 * 
 * @details Обеспечивает запись сообщений в файл журнала с указанием даты,
 *          времени и уровня важности. Все операции потокобезопасны благодаря
 *          использованию мьютекса.
 */
class Logger {
private:
    /**
     * @brief Поток для записи в файл журнала.
     */
    std::ofstream logFile;
    
    /**
     * @brief Мьютекс для обеспечения потокобезопасности операций записи.
     */
    std::mutex logMutex;
    
    /**
     * @brief Имя файла журнала.
     */
    std::string logFileName;

public:
    /**
     * @brief Конструктор класса Logger.
     * @param[in] filename Имя файла журнала.
     * @details Открывает файл для записи логов в режиме добавления.
     * @throw std::runtime_error если файл не может быть открыт.
     */
    Logger(const std::string& filename);
    
    /**
     * @brief Деструктор класса Logger.
     * @details Закрывает файл журнала.
     */
    ~Logger();
    
    /**
     * @brief Записывает сообщение в журнал с указанным уровнем важности.
     * @param[in] message Текст сообщения.
     * @param[in] level Уровень важности сообщения (по умолчанию "INFO").
     * @details Формат записи: [дата время] [уровень] сообщение
     */
    void log(const std::string& message, const std::string& level = "INFO");
    
    /**
     * @brief Записывает сообщение об ошибке в журнал.
     * @param[in] errorMessage Текст сообщения об ошибке.
     * @details Использует уровень "ERROR".
     */
    void logError(const std::string& errorMessage);
    
    /**
     * @brief Записывает информационное сообщение в журнал.
     * @param[in] infoMessage Текст информационного сообщения.
     * @details Использует уровень "INFO".
     */
    void logInfo(const std::string& infoMessage);
};

#endif
