#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

Logger::Logger(const std::string& filename) : logFileName(filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "ERROR: Cannot open log file: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

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

void Logger::logError(const std::string& errorMessage) {
    log(errorMessage, "ERROR");
}

void Logger::logInfo(const std::string& infoMessage) {
    log(infoMessage, "INFO");
}
