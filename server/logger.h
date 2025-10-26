#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class Logger {
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string logFileName;

public:
    Logger(const std::string& filename);
    ~Logger();
    
    void log(const std::string& message, const std::string& level = "INFO");
    void logError(const std::string& errorMessage);
    void logInfo(const std::string& infoMessage);
};

#endif
