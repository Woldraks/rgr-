#include "utils.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <limits> // Добавляем для clearInputBuffer

using namespace std;

// Определение константы в .cpp файле
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
    cout << question << " (y/n): ";
    getline(cin, answer);
    return (answer == "y" || answer == "Y" || answer == "yes" || answer == "Yes" ||
            answer == "д" || answer == "Д" || answer == "да" || answer == "Да");
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