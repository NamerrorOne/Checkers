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
        reload(); // ��� �������� ������� Config ������������� ����������� ��������� �� ����� settings.json
    }

    // ������� reload() ��������� ��������� �� ����� settings.json
   // ��� ��������� ����, ��������� ��� ���������� � ������ json � ��������� ����
    void reload()
    {
        std::ifstream fin(project_path + "settings.json"); // ��������� ���� settings.json
        fin >> config; // ��������� ���������� ����� � ������ json
        fin.close(); // ��������� ����
    }

    // �������� ������� ������ (operator()) ��������� �������� �������� ��������� �� ������������
   // ��������� ��� ���������: setting_dir (������ ��������) � setting_name (��� ���������)
   // ���������� �������� ��������� �� ������� json
    auto operator()(const string &setting_dir, const string &setting_name) const
    {
        return config[setting_dir][setting_name]; // ���������� �������� ���������
    }

  private:
    json config; // ������ json, �������� ��� ��������� �� ����� settings.json
};
