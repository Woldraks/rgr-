#include "utils.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <limits> // Добавляем для clearInputBuffer

using namespace std;

const char *SYSTEM_CLEAR = "clear";

void writeToFile(const std::string &filename, const std::string &content)
{
    ofstream f(filename);
    if (!f)
        throw logic_error("Не удалось открыть файл для записи: " + filename);
    f << content;
}

std::string readFromFile(const std::string &filename)
{
    ifstream f(filename);
    if (!f)
        throw logic_error("Не удалось открыть файл для чтения: " + filename);
    string res, line;
    while (getline(f, line))
        res += line + "\n";

    // Убираем последний \n если он есть
    if (!res.empty() && res.back() == '\n')
    {
        res.pop_back();
    }
    return res;
}

void clearInputBuffer()
{
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool askUser(const std::string &question)
{
    string answer;

    while (true)
    {
        cout << question << " (y/n): ";
        getline(cin, answer);

        // Удаляем лишние пробелы в начале и конце
        size_t start = answer.find_first_not_of(" \t");
        if (start == string::npos)
        {
            // Строка пустая или только пробелы
            cout << "Ошибка: пустой ввод. Пожалуйста, введите 'y' или 'n'." << endl;
            continue;
        }

        size_t end = answer.find_last_not_of(" \t");
        answer = answer.substr(start, end - start + 1);

        // Проверяем корректные варианты для "да"
        if (answer == "y" || answer == "Y" ||
            answer == "yes" || answer == "Yes" ||
            answer == "YES" ||
            answer == "д" || answer == "Д" ||
            answer == "да" || answer == "Да" ||
            answer == "ДА")
        {
            return true;
        }

        // Проверяем корректные варианты для "нет"
        if (answer == "n" || answer == "N" ||
            answer == "no" || answer == "No" ||
            answer == "NO" ||
            answer == "н" || answer == "Н" ||
            answer == "нет" || answer == "Нет" ||
            answer == "НЕТ")
        {
            return false;
        }

        // Если дошли сюда - ввод некорректный
        cout << "Ошибка: некорректный ввод '" << answer << "'. Пожалуйста, введите 'y' или 'n'." << endl;
        cout << "Допустимые варианты: y, n, yes, no, д, н, да, нет" << endl;
    }
}

std::string getInputFilename(const std::string &defaultFile, const std::string &operation)
{
    if (askUser("Хотите указать файл для " + operation + "?"))
    {
        string filename;
        cout << "Введите путь к файлу: ";
        getline(cin, filename);
        return filename;
    }
    return defaultFile;
}

std::string getOutputFilename(const std::string &defaultFile, const std::string &operation)
{
    if (askUser("Хотите указать файл для сохранения результата " + operation + "?"))
    {
        string filename;
        cout << "Введите путь к файлу: ";
        getline(cin, filename);
        return filename;
    }
    return defaultFile;
}

void safeSystemCall(const char *command)
{
    system(command);
}