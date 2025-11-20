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
#include <map>

using namespace std;

// ================= UTF-8 ⇄ Unicode =================

vector<char32_t> utf8_to_ucs(const string &s)
{
    vector<char32_t> out;
    size_t i = 0;
    while (i < s.size())
    {
        unsigned char c = s[i];
        char32_t cp = 0;
        size_t len = 1;

        if (c <= 0x7F)
        {
            cp = c;
        }
        else if ((c & 0xE0) == 0xC0 && i + 1 < s.size())
        {
            cp = ((c & 0x1F) << 6) | (s[i + 1] & 0x3F);
            len = 2;
        }
        else if ((c & 0xF0) == 0xE0 && i + 2 < s.size())
        {
            cp = ((c & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
            len = 3;
        }
        else if ((c & 0xF8) == 0xF0 && i + 3 < s.size())
        {
            cp = ((c & 0x07) << 18) | ((s[i + 1] & 0x3F) << 12) | ((s[i + 2] & 0x3F) << 6) | (s[i + 3] & 0x3F);
            len = 4;
        }
        else
        {
            cp = c;
        }

        out.push_back(cp);
        i += len;
    }
    return out;
}

string ucs_to_utf8(char32_t cp)
{
    string out;

    if (cp <= 0x7F)
    {
        out += char(cp);
    }
    else if (cp <= 0x7FF)
    {
        out += char(0xC0 | (cp >> 6));
        out += char(0x80 | (cp & 0x3F));
    }
    else if (cp <= 0xFFFF)
    {
        out += char(0xE0 | (cp >> 12));
        out += char(0x80 | ((cp >> 6) & 0x3F));
        out += char(0x80 | (cp & 0x3F));
    }
    else
    {
        out += char(0xF0 | (cp >> 18));
        out += char(0x80 | ((cp >> 12) & 0x3F));
        out += char(0x80 | ((cp >> 6) & 0x3F));
        out += char(0x80 | (cp & 0x3F));
    }

    return out;
}

// ================= Проверки букв и регистр =================

bool is_letter(char32_t c)
{
    // русские
    if ((c >= 0x0410 && c <= 0x042F) ||
        (c >= 0x0430 && c <= 0x044F) ||
        c == 0x0401 || c == 0x0451)
        return true;

    // латиница
    if ((c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z'))
        return true;

    return false;
}

char32_t tolower_u(char32_t c)
{
    // русские
    if (c >= 0x0410 && c <= 0x042F)
        return c + 32;
    if (c == 0x0401)
        return 0x0451;

    // латиница
    if (c >= 'A' && c <= 'Z')
        return c + 32;

    return c;
}

// ================= Класс книжного шифра =================

class BookCipherInternal
{
private:
    vector<vector<char32_t>> book; // декодированные строки
    bool ok = false;
    mt19937 rng;

public:
    BookCipherInternal() : rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count()) {}

    bool load(const string &path)
    {
        ifstream f(path, ios::binary);
        if (!f)
        {
            throw runtime_error("Не удалось открыть файл: " + path);
        }

        book.clear();
        string line;

        while (getline(f, line))
        {
            book.push_back(utf8_to_ucs(line));
        }

        if (book.empty())
        {
            throw runtime_error("Файл пуст: " + path);
        }

        ok = true;
        return true;
    }

    vector<pair<int, int>> encrypt(const string &msg)
    {
        if (!ok)
            throw runtime_error("Книга не загружена");

        vector<char32_t> letters;
        for (char32_t c : utf8_to_ucs(msg))
        {
            if (is_letter(c))
                letters.push_back(tolower_u(c));
        }

        if (letters.empty())
            throw runtime_error("В сообщении нет букв");

        vector<pair<int, int>> out;
        map<char32_t, vector<pair<int, int>>> index;

        for (char32_t c : letters)
        {
            if (!index.count(c))
            {
                vector<pair<int, int>> pos;
                for (int i = 0; i < (int)book.size(); i++)
                {
                    for (int j = 0; j < (int)book[i].size(); j++)
                    {
                        if (is_letter(book[i][j]) &&
                            tolower_u(book[i][j]) == c)
                            pos.emplace_back(i + 1, j + 1);
                    }
                }
                if (pos.empty())
                    throw runtime_error("Буква '" + ucs_to_utf8(c) + "' не найдена в книге");
                index[c] = pos;
            }

            auto &arr = index[c];
            out.push_back(arr[rng() % arr.size()]);
        }

        return out;
    }

    string decrypt(const vector<pair<int, int>> &coords)
    {
        if (!ok)
            throw runtime_error("Книга не загружена");

        string out;

        for (auto &p : coords)
        {
            int line = p.first - 1;
            int col = p.second - 1;

            if (line < 0 || line >= (int)book.size())
                throw runtime_error("Неверная строка: " + to_string(p.first));

            if (col < 0 || col >= (int)book[line].size())
                throw runtime_error("Неверная позиция: " + to_string(p.second));

            out += ucs_to_utf8(book[line][col]);
        }

        return out;
    }
};

// ================= Интерфейсные функции =================

vector<pair<int, int>> book_encode(const string &txt, const string &bookPath)
{
    vector<pair<int, int>> out;

    cout << "=== ОТЛАДКА ШИФРОВАНИЯ ===" << endl;
    cout << "Сообщение для шифрования: '" << txt << "' (" << txt.size() << " байт)" << endl;

    BookCipherInternal cipher;
    if (!cipher.load(bookPath))
    {
        throw runtime_error("Не удалось загрузить книгу: " + bookPath);
    }

    out = cipher.encrypt(txt);

    cout << "=== ШИФРОВАНИЕ ЗАВЕРШЕНО ===" << endl;
    cout << "Получено " << out.size() << " координат" << endl;

    return out;
}

string book_decode(const vector<pair<int, int>> &cipher, const string &bookPath)
{
    string out;

    cout << "=== ОТЛАДКА ДЕШИФРОВАНИЯ ===" << endl;
    cout << "Координат для дешифрования: " << cipher.size() << endl;

    BookCipherInternal decoder;
    if (!decoder.load(bookPath))
    {
        throw runtime_error("Не удалось загрузить книгу: " + bookPath);
    }

    out = decoder.decrypt(cipher);

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