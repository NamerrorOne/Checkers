#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config
{
public:
    Config()
    {
        reload(); // При создании объекта Config автоматически загружаются настройки из файла settings.json
    }

    // Функция reload() загружает настройки из файла settings.json
   // Она открывает файл, считывает его содержимое в объект json и закрывает файл
    void reload()
    {
        std::ifstream fin(project_path + "settings.json"); // Открываем файл settings.json
        fin >> config; // Считываем содержимое файла в объект json
        fin.close(); // Закрываем файл
    }

    // Оператор круглые скобки (operator()) позволяет получить значение настройки из конфигурации
   // Принимает два аргумента: setting_dir (раздел настроек) и setting_name (имя настройки)
   // Возвращает значение настройки из объекта json
    auto operator()(const string& setting_dir, const string& setting_name) const
    {
        return config[setting_dir][setting_name]; // Возвращаем значение настройки
    }

private:
    json config; // Объект json, хранящий все настройки из файла settings.json
};
