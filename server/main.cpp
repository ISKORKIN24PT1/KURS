/**
 * @file main.cpp
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Главный файл серверного приложения.
 * @details Содержит точку входа программы, обработку аргументов командной строки
 *          и основной цикл управления сервером.
 */

#include "server.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <getopt.h>

/**
 * @brief Отображает справку по использованию программы.
 * @param[in] programName Имя исполняемого файла программы.
 */
void showUsage(const char* programName) {
    std::cout << "Сервер для вычисления суммы квадратов векторов" << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  " << programName << " <port> <user_file> <log_file>" << std::endl;
    std::cout << "  " << programName << " --port <port> --users <user_file> --log <log_file>" << std::endl;
    std::cout << "  " << programName << " -h | --help" << std::endl;
    std::cout << std::endl;
    std::cout << "Обязательные параметры:" << std::endl;
    std::cout << "  -p, --port PORT     Порт сервера (1-65535)" << std::endl;
    std::cout << "  -u, --users FILE    Файл с базой пользователей" << std::endl;
    std::cout << "  -l, --log FILE      Файл для записи логов" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  " << programName << " 33333 users.txt server.log" << std::endl;
    std::cout << "  " << programName << " --port 33333 --users users.txt --log server.log" << std::endl;
    std::cout << "  " << programName << " -p 33333 -u users.txt -l server.log" << std::endl;
}

/**
 * @brief Главная функция программы.
 * @param[in] argc Количество аргументов командной строки.
 * @param[in] argv Массив аргументов командной строки.
 * @return Код завершения программы (0 - успех, 1 - ошибка).
 */
int main(int argc, char *argv[]) {
    // Если нет аргументов, сразу показываем справку
    if (argc == 1) {
        showUsage(argv[0]);
        return 0;
    }
    
    int port = 0;
    std::string userFile;
    std::string logFile;
    bool showHelp = false;

    // Длинные опции
    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"users", required_argument, 0, 'u'},
        {"log", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Обработка аргументов командной строки
    int option_index = 0;
    int c;
    
    // Используем правильную строку опций
    while ((c = getopt_long(argc, argv, ":p:u:l:h", long_options, &option_index)) != -1) {
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
            case 'h':
                showHelp = true;
                break;
            case ':':
                // Отсутствует аргумент для опции
                std::cerr << "Ошибка: отсутствует аргумент для опции '-" 
                          << (char)optopt << "'" << std::endl;
                showUsage(argv[0]);
                return 1;
            case '?':
                // Неизвестный параметр
                std::cerr << "Ошибка: неизвестный параметр '-" 
                          << (char)optopt << "'" << std::endl;
                showUsage(argv[0]);
                return 1;
        }
    }

    // Если запрошена справка
    if (showHelp) {
        showUsage(argv[0]);
        return 0;
    }

    // Улучшенная обработка позиционных аргументов
    if (port == 0 && userFile.empty() && logFile.empty()) {
        // Проверяем, достаточно ли позиционных аргументов
        if (optind + 2 >= argc) {
            std::cerr << "Ошибка: недостаточно параметров" << std::endl;
            showUsage(argv[0]);
            return 1;
        }
        
        // Берем параметры из позиционных аргументов
        port = std::atoi(argv[optind]);
        userFile = argv[optind + 1];
        logFile = argv[optind + 2];
    }

    // Валидация введенных параметров
    if (port <= 0 || port > 65535) {
        std::cerr << "Ошибка: неверный номер порта: " << port << std::endl;
        std::cerr << "Порт должен быть в диапазоне 1-65535" << std::endl;
        showUsage(argv[0]);
        return 1;
    }

    if (userFile.empty() || logFile.empty()) {
        std::cerr << "Ошибка: необходимо указать все параметры" << std::endl;
        std::cerr << "Обязательные параметры: --port, --users, --log" << std::endl;
        showUsage(argv[0]);
        return 1;
    }

    // Вывод информации о параметрах запуска
    std::cout << "Запуск сервера с параметрами:" << std::endl;
    std::cout << "  Порт: " << port << std::endl;
    std::cout << "  Файл пользователей: " << userFile << std::endl;
    std::cout << "  Файл логов: " << logFile << std::endl;
    std::cout << std::endl;

    // Создание экземпляра сервера с указанными параметрами
    Server server(port, userFile, logFile);
    
    // Инициализация сервера
    if (!server.initialize()) {
        std::cerr << "Ошибка инициализации сервера" << std::endl;
        return 1;
    }

    // Запуск основного цикла работы сервера
    std::cout << "Сервер запущен. Для остановки нажмите Ctrl+C" << std::endl;
    server.run();

    // Завершение работы программы
    std::cout << "Сервер остановлен" << std::endl;
    return 0;
}
