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
#include <random>

using namespace std;

// Функция для определения длины UTF-8 символа по первому байту
size_t get_utf8_char_length(unsigned char first_byte)
{
    if ((first_byte & 0x80) == 0x00)
        return 1; // ASCII (1 байт)
    if ((first_byte & 0xE0) == 0xC0)
        return 2; // 2 байта
    if ((first_byte & 0xF0) == 0xE0)
        return 3; // 3 байта
    if ((first_byte & 0xF8) == 0xF0)
        return 4; // 4 байта
    return 1;     // По умолчанию 1 байт
}

// Функция для поиска ВСЕХ вхождений символа в книге
vector<size_t> findAllSymbolPositions(const string &book, const string &symbol)
{
    vector<size_t> positions;
    size_t pos = 0;

    while ((pos = book.find(symbol, pos)) != string::npos)
    {
        positions.push_back(pos);
        pos += symbol.length();
    }

    return positions;
}

// Преобразует позицию в книге в координаты (строка, столбец)
pair<int, int> positionToCoordinates(const string &book, size_t pos)
{
    int line = 1;
    int col = 1;

    for (size_t i = 0; i < pos && i < book.size(); ++i)
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

    return make_pair(line, col);
}

// Преобразует координаты в позицию в книге
size_t coordinatesToPosition(const string &book, int line, int col)
{
    int current_line = 1;
    int current_col = 1;

    for (size_t i = 0; i < book.size(); ++i)
    {
        if (current_line == line && current_col == col)
        {
            return i;
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

    throw runtime_error("Координаты " + to_string(line) + ":" + to_string(col) + " не найдены в книге");
}

vector<pair<int, int>> book_encode(const string &txt, const string &bookPath)
{
    vector<pair<int, int>> out;

    // Загружаем книгу как ТЕКСТ
    string book = readFromFile(bookPath);
    if (book.empty())
    {
        throw runtime_error("Книга пуста: " + bookPath);
    }

    // Инициализируем генератор случайных чисел
    random_device rd;
    mt19937 gen(rd());

    cout << "=== ОТЛАДКА ШИФРОВАНИЯ ===" << endl;
    cout << "Сообщение для шифрования: '" << txt << "' (" << txt.size() << " байт)" << endl;
    cout << "Размер книги: " << book.size() << " байт" << endl;

    for (size_t i = 0; i < txt.size();)
    {
        string symbol;

        // Определяем размер символа (для UTF-8)
        unsigned char first_byte = static_cast<unsigned char>(txt[i]);
        size_t char_len = get_utf8_char_length(first_byte);

        if (i + char_len <= txt.size())
        {
            symbol = txt.substr(i, char_len);
        }
        else
        {
            symbol = string(1, txt[i]);
        }

        i += symbol.length();

        cout << "Ищем символ: '" << symbol << "' (" << symbol.length() << " байт) в книге..." << endl;

        // Ищем ВСЕ позиции символа в книге
        vector<size_t> positions = findAllSymbolPositions(book, symbol);

        if (positions.empty())
        {
            throw runtime_error("Символ '" + symbol + "' не найден в книге");
        }

        // Выбираем СЛУЧАЙНУЮ позицию из найденных
        uniform_int_distribution<size_t> dist(0, positions.size() - 1);
        size_t random_pos = positions[dist(gen)];

        // Преобразуем позицию в координаты
        auto coords = positionToCoordinates(book, random_pos);
        out.push_back(coords);

        cout << "Найдено " << positions.size() << " вхождений" << endl;
        cout << "Выбрана позиция " << random_pos << " -> координаты: " << coords.first << ":" << coords.second << endl;
    }

    cout << "=== ШИФРОВАНИЕ ЗАВЕРШЕНО ===" << endl;
    cout << "Получено " << out.size() << " координат" << endl;

    return out;
}

string book_decode(const vector<pair<int, int>> &cipher, const string &bookPath)
{
    string out;

    // Загружаем книгу как ТЕКСТ
    string book = readFromFile(bookPath);
    if (book.empty())
    {
        throw runtime_error("Книга пуста: " + bookPath);
    }

    cout << "=== ОТЛАДКА ДЕШИФРОВАНИЯ ===" << endl;
    cout << "Координат для дешифрования: " << cipher.size() << endl;

    for (const auto &coord : cipher)
    {
        try
        {
            cout << "Обрабатываем координаты: " << coord.first << ":" << coord.second << endl;

            // Преобразуем координаты в позицию
            size_t pos = coordinatesToPosition(book, coord.first, coord.second);

            if (pos >= book.size())
            {
                throw runtime_error("Позиция выходит за границы книги");
            }

            // Определяем размер символа в этой позиции
            string symbol;
            unsigned char first_byte = static_cast<unsigned char>(book[pos]);
            size_t char_len = get_utf8_char_length(first_byte);

            if (pos + char_len <= book.size())
            {
                symbol = book.substr(pos, char_len);
            }
            else
            {
                symbol = string(1, book[pos]);
            }

            out += symbol;
            cout << "Найден символ: '" << symbol << "'" << endl;
        }
        catch (const exception &e)
        {
            throw runtime_error("Не найдена позиция " + to_string(coord.first) +
                                ":" + to_string(coord.second) + " в книге. " + e.what());
        }
    }

    cout << "=== ДЕШИФРОВАНИЕ ЗАВЕРШЕНО ===" << endl;
    cout << "Получено: '" << out << "'" << endl;

    return out;
}

void bookCipher(string &password)
{
    string pass, txt;
    int ch, src;

    // Путь к книге по умолчанию
    string defaultBookPath = "book.txt";

    while (true)
    {
        try
        {
            safeSystemCall(SYSTEM_CLEAR);
            cout << "КНИЖНЫЙ ШИФР\n1. Шифрование\n2. Дешифрование\n3. Выход\n> ";
            cin >> ch;

            if (cin.fail() || cin.peek() != '\n')
            {
                throw logic_error("Неверный ввод");
            }

            if (ch == 3)
                break;
            if (ch != 1 && ch != 2)
            {
                throw logic_error("Нет операции");
            }

            cout << "Пароль: ";
            cin >> pass;
            if (pass != password)
            {
                throw logic_error("Неверный пароль");
            }
            clearInputBuffer();

            if (ch == 1)
            {
                // ШИФРОВАНИЕ
                cout << "Источник:\n1. Консоль\n2. Файл\n> ";
                cin >> src;

                if (cin.fail() || cin.peek() != '\n')
                {
                    throw logic_error("Неверный ввод");
                }

                if (src != 1 && src != 2)
                {
                    throw logic_error("Нет операции");
                }
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
                    bookPath = defaultBookPath;
                    cout << "Используется книга по умолчанию: " << defaultBookPath << endl;
                }

                // ШИФРУЕМ
                vector<pair<int, int>> cipher = book_encode(txt, bookPath);

                string outputFile = getOutputFilename("BOOKencrypted.txt", "шифрования");
                ofstream out(outputFile);
                for (size_t i = 0; i < cipher.size(); ++i)
                {
                    out << cipher[i].first << ':' << cipher[i].second;
                    if (i != cipher.size() - 1)
                    {
                        out << ' ';
                    }
                }
                out.close();

                cout << "Шифр сохранен в " << outputFile << endl;
                cout << "Зашифровано " << cipher.size() << " символов" << endl;
                this_thread::sleep_for(chrono::milliseconds(2000));
            }
            else
            {
                // ДЕШИФРОВАНИЕ
                string inputFile = getInputFilename("BOOKencrypted.txt", "дешифрования");
                string data = readFromFile(inputFile);

                vector<pair<int, int>> cipher;
                stringstream ss(data);
                string token;

                while (ss >> token)
                {
                    size_t sep = token.find(':');
                    if (sep == string::npos)
                    {
                        continue;
                    }

                    int line = stoi(token.substr(0, sep));
                    int col = stoi(token.substr(sep + 1));
                    cipher.push_back(make_pair(line, col));
                }

                string bookPath;
                if (askUser("Хотите указать файл с книгой?"))
                {
                    cout << "Введите путь к файлу с книгой: ";
                    getline(cin, bookPath);
                }
                else
                {
                    bookPath = defaultBookPath;
                    cout << "Используется книга по умолчанию: " << defaultBookPath << endl;
                }

                // ДЕШИФРУЕМ
                string dec = book_decode(cipher, bookPath);
                string outputFile = getOutputFilename("BOOKdecrypted.txt", "дешифрования");
                writeToFile(outputFile, dec);

                cout << "Расшифровано в " << outputFile << endl;
                cout << "Расшифрованный текст: " << dec << endl;
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