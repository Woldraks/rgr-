#include "utils.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

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
    return res;
}

void clearInputBuffer()
{
    cin.clear();
    cin.ignore(256, '\n');
}

bool askUser(const string &question)
{
    string answer;
    cout << question << " (y/n): ";
    getline(cin, answer);
    return (answer == "y" || answer == "Y" || answer == "yes" || answer == "Yes");
}

string getInputFilename(const string &defaultFile, const string &operation)
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

string getOutputFilename(const string &defaultFile, const string &operation)
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

// Определение функции в .cpp файле
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
void safeSystemCall(const char *command)
{
    system(command);
}
#pragma GCC diagnostic pop