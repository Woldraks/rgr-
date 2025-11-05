#include "binary_cipher.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace std;

// УБИРАЕМ определение safeSystemCall отсюда

string textToBinary(const string &text)
{
    string binary;
    for (char c : text)
    {
        for (int i = 7; i >= 0; --i)
        {
            binary += ((c >> i) & 1) ? '1' : '0';
        }
    }
    return binary;
}

string binaryToText(const string &binary)
{
    string text;
    string clean_binary;
    for (char c : binary)
    {
        if (c == '0' || c == '1')
            clean_binary += c;
    }

    if (clean_binary.length() % 8 != 0)
    {
        throw logic_error("Длина бинарной строки должна быть кратна 8");
    }

    for (size_t i = 0; i < clean_binary.length(); i += 8)
    {
        char c = 0;
        for (int j = 0; j < 8; ++j)
        {
            c = (c << 1) | (clean_binary[i + j] - '0');
        }
        text += c;
    }
    return text;
}

string seq_encode(const string &txt)
{
    if (txt.empty())
        return "";
    string out;
    out += txt[0];

    for (size_t i = 1; i < txt.size(); ++i)
    {
        out += (txt[i] == txt[i - 1]) ? '1' : '0';
    }
    return out;
}

string seq_decode(const string &c)
{
    if (c.empty())
        return "";
    string out;
    out += c[0];

    for (size_t i = 1; i < c.size(); ++i)
    {
        if (c[i] == '1')
        {
            out += out.back();
        }
        else
        {
            out += (out.back() == '0') ? '1' : '0';
        }
    }
    return out;
}

void binarySequenceCipher(string &password)
{
    string pass, txt;
    int ch, src, mode;
    while (true)
    {
        try
        {
            safeSystemCall(SYSTEM_CLEAR); // Используем из utils
            cout << "ШИФР 0 И 1\n1. Шифрование\n2. Дешифрование\n3. Выход\n> ";
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

            cout << "Режим:\n1. Бинарные данные (только 0 и 1)\n2. Текст\n> ";
            cin >> mode;
            if (cin.fail() || cin.peek() != '\n')
                throw logic_error("Неверный ввод");
            if (mode != 1 && mode != 2)
                throw logic_error("Нет операции");
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
                    if (mode == 1)
                    {
                        cout << "Введите последовательность из 0 и 1: ";
                        getline(cin, txt);
                        for (char c : txt)
                        {
                            if (c != '0' && c != '1' && c != ' ')
                            {
                                throw logic_error("Последовательность должна содержать только 0 и 1");
                            }
                        }
                    }
                    else
                    {
                        cout << "Введите текст: ";
                        getline(cin, txt);
                    }
                }
                else
                {
                    string inputFile = getInputFilename("BINinput.txt", "чтения");
                    txt = readFromFile(inputFile);
                }

                string processed_txt, enc, final_result;

                if (mode == 1)
                {
                    for (char c : txt)
                    {
                        if (c == '0' || c == '1')
                            processed_txt += c;
                    }
                    enc = seq_encode(processed_txt);
                    final_result = enc;
                }
                else
                {
                    processed_txt = textToBinary(txt);
                    enc = seq_encode(processed_txt);
                    final_result = enc;

                    cout << "\n--- Информация о шифровании ---" << endl;
                    cout << "Исходный текст: " << txt << endl;
                    cout << "Бинарное представление: " << processed_txt << endl;
                    cout << "Зашифрованная бинарная последовательность: " << enc << endl;
                }

                string outputFile = getOutputFilename("BINencrypted.txt", "шифрования");
                writeToFile(outputFile, final_result);
                cout << "\nЗашифровано в " << outputFile << endl;
                this_thread::sleep_for(chrono::milliseconds(3000));
            }
            else
            {
                string inputFile = getInputFilename("BINencrypted.txt", "дешифрования");
                string encrypted_data = readFromFile(inputFile);

                string outputFile;

                if (mode == 1)
                {
                    string clean_data;
                    for (char c : encrypted_data)
                    {
                        if (c == '0' || c == '1')
                            clean_data += c;
                    }
                    string dec = seq_decode(clean_data);

                    outputFile = getOutputFilename("BINdecrypted.txt", "дешифрования");
                    writeToFile(outputFile, dec);

                    cout << "--- Информация о расшифровке ---" << endl;
                    cout << "Зашифрованные данные: " << clean_data << endl;
                    cout << "Расшифрованные данные: " << dec << endl;
                }
                else
                {
                    string clean_data;
                    for (char c : encrypted_data)
                    {
                        if (c == '0' || c == '1')
                            clean_data += c;
                    }
                    string decoded_binary = seq_decode(clean_data);
                    string final_text = binaryToText(decoded_binary);

                    outputFile = getOutputFilename("BINdecrypted.txt", "дешифрования");
                    writeToFile(outputFile, final_text);

                    cout << "--- Информация о расшифровке ---" << endl;
                    cout << "Зашифрованные данные: " << clean_data << endl;
                    cout << "Расшифрованное бинарное представление: " << decoded_binary << endl;
                    cout << "Итоговый текст: " << final_text << endl;
                }

                cout << "\nРасшифровано в " << outputFile << endl;
                this_thread::sleep_for(chrono::milliseconds(3000));
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