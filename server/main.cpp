#include "server.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <getopt.h>

void showUsage(const char* programName) {
    std::cout << "Использование: " << std::endl;
    std::cout << "  " << programName << " <port> <user_file> <log_file>" << std::endl;
    std::cout << "  " << programName << " --port <port> --users <user_file> --log <log_file>" << std::endl;
    std::cout << "Пример: " << std::endl;
    std::cout << "  " << programName << " 8080 users.txt server.log" << std::endl;
    std::cout << "  " << programName << " --port 33333 --users users.txt --log server.log" << std::endl;
}

int main(int argc, char *argv[]) {
    int port = 0;
    std::string userFile;
    std::string logFile;

    if (argc == 4) {
        port = std::atoi(argv[1]);
        userFile = argv[2];
        logFile = argv[3];
    }
    else if (argc > 4) {
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {"users", required_argument, 0, 'u'},
            {"log", required_argument, 0, 'l'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c;
        
        while ((c = getopt_long(argc, argv, "p:u:l:", long_options, &option_index)) != -1) {
            switch (c) {
                case 'p':
                    port = std::atoi(optarg);
                    break;
                case 'u':
                    userFile = optarg;
                    break;
                case 'l':
                    logFile = optarg;
                    break;
                default:
                    showUsage(argv[0]);
                    return 1;
            }
        }
    }
    else {
        showUsage(argv[0]);
        return 1;
    }

    if (port <= 0 || port > 65535) {
        std::cerr << "Ошибка: неверный номер порта: " << port << std::endl;
        return 1;
    }

    if (userFile.empty() || logFile.empty()) {
        std::cerr << "Ошибка: необходимо указать все параметры" << std::endl;
        showUsage(argv[0]);
        return 1;
    }

    std::cout << "Запуск сервера с параметрами:" << std::endl;
    std::cout << "  Порт: " << port << std::endl;
    std::cout << "  Файл пользователей: " << userFile << std::endl;
    std::cout << "  Файл логов: " << logFile << std::endl;

    Server server(port, userFile, logFile);
    
    if (!server.initialize()) {
        std::cerr << "Ошибка инициализации сервера" << std::endl;
        return 1;
    }

    std::cout << "Сервер запущен. Для остановки нажмите Ctrl+C" << std::endl;
    server.run();

    std::cout << "Сервер остановлен" << std::endl;
    return 0;
}
