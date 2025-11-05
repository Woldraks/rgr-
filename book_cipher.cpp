#include "book_cipher.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace std;

// УБИРАЕМ определение safeSystemCall отсюда

vector<pair<int, int>> book_encode(const string &txt, const string &book)
{
    vector<pair<int, int>> out;

    for (char c : txt)
    {
        bool found = false;

        for (size_t pos = 0; pos < book.size(); ++pos)
        {
            if (book[pos] == c)
            {
                int line = 1;
                int col = 1;

                for (size_t i = 0; i < pos; ++i)
                {
                    if (book[i] == '\n')
                    {
                        line++;
                        col = 1;
                    }
                    else
                    {
                        col++;
                    }
                }

                out.push_back(make_pair(line, col));
                found = true;
                break;
            }
        }

        if (!found)
        {
            throw logic_error("Символ '" + string(1, c) + "' не найден в книге");
        }
    }
    return out;
}

string book_decode(const vector<pair<int, int>> &cipher, const string &book)
{
    string out;

    for (const auto &coord : cipher)
    {
        int target_line = coord.first;
        int target_col = coord.second;

        int current_line = 1;
        int current_col = 1;
        bool found = false;

        for (size_t i = 0; i < book.size(); ++i)
        {
            if (current_line == target_line && current_col == target_col)
            {
                out += book[i];
                found = true;
                break;
            }

            if (book[i] == '\n')
            {
                current_line++;
                current_col = 1;
            }
            else
            {
                current_col++;
            }
        }

        if (!found)
        {
            throw logic_error("Не найдена позиция " + to_string(target_line) +
                              ":" + to_string(target_col) + " в книге");
        }
    }
    return out;
}

void bookCipher(string &password)
{
    string pass, txt, book;
    int ch, src;
    while (true)
    {
        try
        {
            safeSystemCall(SYSTEM_CLEAR); // Используем из utils
            cout << "КНИЖНЫЙ ШИФР\n1. Шифрование\n2. Дешифрование\n3. Выход\n> ";
            cin >> ch;
            if (cin.fail() || cin.peek() != '\n')
                throw logic_error("Неверный ввод");
            if (ch == 3)
                break;
            if (ch != 1 && ch != 2)
                throw logic_error("Нет операции");

            cout << "Пароль: ";
            cin >> pass;
            if (pass != password)
                throw logic_error("Неверный пароль");
            clearInputBuffer();

            if (ch == 1)
            {
                cout << "Источник:\n1. Консоль\n2. Файл\n> ";
                cin >> src;
                if (cin.fail() || cin.peek() != '\n')
                    throw logic_error("Неверный ввод");
                if (src != 1 && src != 2)
                    throw logic_error("Нет операции");
                clearInputBuffer();

                if (src == 1)
                {
                    cout << "Текст: ";
                    getline(cin, txt);
                }
                else
                {
                    string inputFile = getInputFilename("BOOKinput.txt", "чтения");
                    txt = readFromFile(inputFile);
                }

                string bookPath;
                if (askUser("Хотите указать файл с книгой?"))
                {
                    cout << "Введите путь к файлу с книгой: ";
                    getline(cin, bookPath);
                }
                else
                {
                    bookPath = "/home/doni/Рабочий стол/rgr/book.txt";
                }
                book = readFromFile(bookPath);

                vector<pair<int, int>> cipher = book_encode(txt, book);

                string outputFile = getOutputFilename("BOOKencrypted.txt", "шифрования");
                ofstream out(outputFile);
                for (size_t i = 0; i < cipher.size(); ++i)
                {
                    out << cipher[i].first << ':' << cipher[i].second << ' ';
                }
                out.close();
                cout << "Шифр сохранен в " << outputFile << endl;
                this_thread::sleep_for(chrono::milliseconds(2000));
            }
            else
            {
                string inputFile = getInputFilename("BOOKencrypted.txt", "дешифрования");
                string data = readFromFile(inputFile);

                vector<pair<int, int>> cipher;
                stringstream ss(data);
                string token;
                while (ss >> token)
                {
                    size_t sep = token.find(':');
                    if (sep == string::npos)
                        continue;
                    int l = stoi(token.substr(0, sep));
                    int c = stoi(token.substr(sep + 1));
                    cipher.push_back(make_pair(l, c));
                }

                string bookPath;
                if (askUser("Хотите указать файл с книгой?"))
                {
                    cout << "Введите путь к файлу с книгой: ";
                    getline(cin, bookPath);
                }
                else
                {
                    cout << "Книга: ";
                    getline(cin, bookPath);
                }
                book = readFromFile(bookPath);

                string dec = book_decode(cipher, book);
                string outputFile = getOutputFilename("BOOKdecrypted.txt", "дешифрования");
                writeToFile(outputFile, dec);
                cout << "Расшифровано в " << outputFile << endl;
                this_thread::sleep_for(chrono::milliseconds(2000));
            }
        }
        catch (const exception &e)
        {
            cerr << "Ошибка: " << e.what() << endl;
            clearInputBuffer();
            this_thread::sleep_for(chrono::milliseconds(2000));
        }
    }
}