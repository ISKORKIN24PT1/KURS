/**
 * @file authenticator.h
 * @author Искоркин Андрей Дмитриевич
 * @date 08.12.2025
 * @brief Заголовочный файл класса Authenticator для аутентификации пользователей.
 * @details Класс предоставляет методы для загрузки пользователей из файла,
 * проверки аутентификации и вычисления хеша пароля.
 * @warning Для работы требуется поддержка стандарта C++11 или выше.
 */

#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Класс для управления аутентификацией пользователей.
 * 
 * @details Класс хранит пары логин-пароль в хеш-таблице,
 * позволяет загружать их из файла, проверять соответствие
 * введённых данных и вычислять хеш пароля с солью.
 */
class Authenticator {
private:
    /**
     * @brief Хранилище пользователей: логин -> пароль (в хешированном виде).
     */
    std::unordered_map<std::string, std::string> users;
    
public:
    /**
     * @brief Загружает пользователей из файла.
     * @param filename Имя файла с данными пользователей.
     * @return true, если загрузка прошла успешно, false в противном случае.
     * @throw std::runtime_error если файл не может быть открыт.
     * @warning Формат файла должен быть "логин:хеш_пароля".
     */
    bool loadUsersFromFile(const std::string& filename);

    /**
     * @brief Проверяет аутентификацию пользователя.
     * @param login Логин пользователя.
     * @param salt Соль, переданная клиентом.
     * @param clientHash Хеш, вычисленный клиентом.
     * @return true, если аутентификация успешна, false в противном случае.
     * @throw std::invalid_argument если логин не найден.
     */
    bool authenticate(const std::string& login, const std::string& salt, const std::string& clientHash);

    /**
     * @brief Вычисляет хеш пароля с использованием соли.
     * @param salt Соль для хеширования.
     * @param password Пароль в открытом виде.
     * @return Строка с хешем пароля.
     * @throw std::runtime_error если алгоритм хеширования недоступен.
     */
    std::string computeHash(const std::string& salt, const std::string& password);
};

#endif
