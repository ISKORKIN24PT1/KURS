#include "server.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <getopt.h>

// Функция для отображения справки по использованию программы
void showUsage(const char* programName) {
    std::cout << "Использование: " << std::endl;
    std::cout << "  " << programName << " <port> <user_file> <log_file>" << std::endl;
    std::cout << "  " << programName << " --port <port> --users <user_file> --log <log_file>" << std::endl;
    std::cout << "Пример: " << std::endl;
    std::cout << "  " << programName << " 33333 users.txt server.log" << std::endl;
    std::cout << "  " << programName << " --port 33333 --users users.txt --log server.log" << std::endl;
}

// Главная функция программы - точка входа
int main(int argc, char *argv[]) {
    int port = 0;              // Порт сервера
    std::string userFile;      // Файл с пользователями для аутентификации
    std::string logFile;       // Файл для записи логов

    // Обработка аргументов командной строки
    
    // Вариант 1: Простая форма с позиционными аргументами
    if (argc == 4) {
        port = std::atoi(argv[1]);      // Порт из первого аргумента
        userFile = argv[2];             // Файл пользователей из второго
        logFile = argv[3];              // Файл логов из третьего
    }
    // Вариант 2: Расширенная форма с именованными параметрами
    else if (argc > 4) {
        // Определение длинных опций для getopt_long
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},   // Опция --port с обязательным аргументом
            {"users", required_argument, 0, 'u'},  // Опция --users с обязательным аргументом
            {"log", required_argument, 0, 'l'},    // Опция --log с обязательным аргументом
            {0, 0, 0, 0}                           // Маркер конца массива
        };

        int option_index = 0;
        int c;
        
        // Парсинг опций командной строки
        while ((c = getopt_long(argc, argv, "p:u:l:", long_options, &option_index)) != -1) {
            switch (c) {
                case 'p':  // Обработка порта
                    port = std::atoi(optarg);  // Преобразование строки в число
                    break;
                case 'u':  // Обработка файла пользователей
                    userFile = optarg;         // Сохранение имени файла
                    break;
                case 'l':  // Обработка файла логов
                    logFile = optarg;          // Сохранение имени файла
                    break;
                default:   // Неизвестная опция
                    showUsage(argv[0]);        // Показать справку
                    return 1;                  // Выход с ошибкой
            }
        }
    }
    // Неправильное количество аргументов
    else {
        showUsage(argv[0]);  // Показать справку по использованию
        return 1;            // Выход с ошибкой
    }

    // Валидация введенных параметров

    // Проверка корректности номера порта
    if (port <= 0 || port > 65535) {
        std::cerr << "Ошибка: неверный номер порта: " << port << std::endl;
        std::cerr << "Порт должен быть в диапазоне 1-65535" << std::endl;
        return 1;
    }

    // Проверка что все необходимые параметры указаны
    if (userFile.empty() || logFile.empty()) {
        std::cerr << "Ошибка: необходимо указать все параметры" << std::endl;
        showUsage(argv[0]);  // Показать правильный формат команд
        return 1;
    }

    // Вывод информации о параметрах запуска
    std::cout << "Запуск сервера с параметрами:" << std::endl;
    std::cout << "  Порт: " << port << std::endl;
    std::cout << "  Файл пользователей: " << userFile << std::endl;
    std::cout << "  Файл логов: " << logFile << std::endl;

    // Создание экземпляра сервера с указанными параметрами
    Server server(port, userFile, logFile);
    
    // Инициализация сервера (создание сокета, привязка к порту)
    if (!server.initialize()) {
        std::cerr << "Ошибка инициализации сервера" << std::endl;
        return 1;  // Выход с ошибкой если инициализация не удалась
    }

    // Запуск основного цикла работы сервера
    std::cout << "Сервер запущен. Для остановки нажмите Ctrl+C" << std::endl;
    server.run();  // Этот метод будет работать пока сервер не получит сигнал остановки

    // Завершение работы программы
    std::cout << "Сервер остановлен" << std::endl;
    return 0;  // Успешное завершение
}
