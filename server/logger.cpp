/**
 * @file logger.cpp
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Реализация класса Logger для ведения журнала работы программы.
 */

#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

/**
 * @brief Конструктор класса Logger.
 * @param[in] filename Имя файла журнала.
 */
Logger::Logger(const std::string& filename) : logFileName(filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "ERROR: Cannot open log file: " << filename << std::endl;
    }
}

/**
 * @brief Деструктор класса Logger.
 */
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

/**
 * @brief Записывает сообщение в журнал с указанным уровнем важности.
 * @param[in] message Текст сообщения.
 * @param[in] level Уровень важности сообщения.
 */
void Logger::log(const std::string& message, const std::string& level) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
              << "] [" << level << "] " << message << std::endl;
    
    if (logFile.is_open()) {
        logFile << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                << "] [" << level << "] " << message << std::endl;
        logFile.flush();
    }
}

/**
 * @brief Записывает сообщение об ошибке в журнал.
 * @param[in] errorMessage Текст сообщения об ошибке.
 */
void Logger::logError(const std::string& errorMessage) {
    log(errorMessage, "ERROR");
}

/**
 * @brief Записывает информационное сообщение в журнал.
 * @param[in] infoMessage Текст информационного сообщения.
 */
void Logger::logInfo(const std::string& infoMessage) {
    log(infoMessage, "INFO");
}
