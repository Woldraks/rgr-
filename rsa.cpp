#include "rsa.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace std;

bool isPrime(int n)
{
    if (n <= 1)
        return false;
    if (n <= 3)
        return true;
    if (n % 2 == 0 || n % 3 == 0)
        return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    return true;
}

int getRandomNumber(int min, int max)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(min, max);
    while (true)
    {
        int n = dist(gen);
        if (isPrime(n))
            return n;
    }
}

int NOD(int a, int b) { return b == 0 ? a : NOD(b, a % b); }

int EilerFunc(int p)
{
    int res = p;
    for (int i = 2; i * i <= p; ++i)
        if (p % i == 0)
        {
            while (p % i == 0)
                p /= i;
            res -= res / i;
        }
    if (p > 1)
        res -= res / p;
    return res;
}

int extendedEuclid(int a, int b, int &x, int &y)
{
    if (b == 0)
    {
        x = 1;
        y = 0;
        return a;
    }
    int x1, y1, gcd = extendedEuclid(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}

int modInverse(int e, int phi)
{
    int x, y;
    int g = extendedEuclid(e, phi, x, y);
    if (g != 1)
        return -1;
    return (x % phi + phi) % phi;
}

long long modPower(long long base, long long exponent, long long modulus)
{
    if (modulus == 1)
        return 0;

    long long result = 1;
    base = base % modulus;

    while (exponent > 0)
    {
        if (exponent % 2 == 1)
            result = (result * base) % modulus;

        exponent = exponent >> 1;
        base = (base * base) % modulus;
    }

    return result;
}

string rsa_encode(const string &text, int e, long long n)
{
    string out;
    string separator = " ";

    for (size_t i = 0; i < text.size(); ++i)
    {
        unsigned char byte = static_cast<unsigned char>(text[i]);
        long long c_enc = modPower(byte, e, n);
        out += to_string(c_enc) + separator;
    }

    // Убираем последний пробел
    if (!out.empty() && !separator.empty())
    {
        out.erase(out.size() - separator.size());
    }

    return out;
}

string rsa_decode(const string &text, int d, long long n)
{
    string out;
    string token;

    for (size_t i = 0; i < text.size();)
    {
        // Ищем последовательности цифр
        if (isdigit(text[i]))
        {
            token.clear();
            while (i < text.size() && isdigit(text[i]))
            {
                token += text[i];
                i++;
            }

            try
            {
                if (!token.empty())
                {
                    // Используем long long для больших чисел
                    long long num = stoll(token);

                    // Проверяем диапазон
                    if (num < 0 || num >= n)
                    {
                        cerr << "Предупреждение: число " << num << " вне диапазона [0, " << n - 1 << "]" << endl;
                        continue;
                    }

                    // Дешифруем
                    long long dec = modPower(num, d, n);

                    if (dec < 0 || dec > 255)
                    {
                        cerr << "Ошибка: некорректное значение " << dec << " при дешифровании" << endl;
                        continue;
                    }

                    out += static_cast<unsigned char>(dec);
                }
            }
            catch (const exception &e)
            {
                cerr << "Ошибка обработки токена '" << token << "': " << e.what() << endl;
            }
        }
        else
        {
            i++; // Пропускаем не-цифровые символы
        }
    }

    return out;
}

bool getCustomKeys(int &e, int &d, long long &n)
{
    if (askUser("Хотите ввести свои ключи?  "))
    {
        cout << "Введите открытый ключ e: ";
        cin >> e;
        if (cin.fail())
            return false;

        cout << "Введите закрытый ключ d: ";
        cin >> d;
        if (cin.fail())
            return false;

        cout << "Введите модуль n: ";
        cin >> n;
        if (cin.fail())
            return false;

        clearInputBuffer();
        return true;
    }
    return false;
}

void rsa(string &password)
{
    // Используем большие простые числа для увеличения n
    int p = getRandomNumber(10000, 30000), q = getRandomNumber(10000, 30000);
    long long n = (long long)p * q;
    int phi = EilerFunc(p) * EilerFunc(q);
    int e = getRandomNumber(5000, phi);
    while (NOD(phi, e) != 1)
        e = getRandomNumber(5000, phi);
    int d = modInverse(e, phi);

    // Проверка корректности ключей
    if (d == -1)
    {
        cerr << "Ошибка: не удалось вычислить обратный элемент для e" << endl;
        return;
    }

    string userpass, text;
    int choice, src;
    while (true)
    {
        try
        {
            safeSystemCall(SYSTEM_CLEAR);
            cout << "RSA\n1. Шифрование\n2. Дешифрование\n3. Выход\n> ";
            cin >> choice;
            if (cin.fail() || cin.peek() != '\n')
                throw logic_error("Неверный ввод");
            if (choice == 3)
                break;
            if (choice != 1 && choice != 2)
                throw logic_error("Нет операции");

            cout << "Пароль: ";
            cin >> userpass;
            if (userpass != password)
                throw logic_error("Неверный пароль");
            clearInputBuffer();

            if (choice == 1)
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
                    getline(cin, text);
                }
                else
                {
                    string inputFile = getInputFilename("RSAinput.txt", "чтения");
                    text = readFromFile(inputFile);
                }

                if (text.empty())
                {
                    throw logic_error("Текст для шифрования пуст");
                }

                string enc = rsa_encode(text, e, n);
                string outputFile = getOutputFilename("RSAencrypted.txt", "шифрования");
                writeToFile(outputFile, enc);
                cout << "Зашифровано в " << outputFile << endl;
                cout << "Использованные ключи:\n";
                cout << "e = " << e << "\nd = " << d << "\nn = " << n << endl;
                this_thread::sleep_for(chrono::milliseconds(3000));
            }
            else
            {
                int current_e = e;
                int current_d = d;
                long long current_n = n;

                if (!getCustomKeys(current_e, current_d, current_n))
                {
                    // Используем ключи по умолчанию
                    current_e = e;
                    current_d = d;
                    current_n = n;
                    cout << "Используются ключи по умолчанию" << endl;
                }

                string inputFile = getInputFilename("RSAencrypted.txt", "дешифрования");
                string enc = readFromFile(inputFile);

                if (enc.empty())
                {
                    throw logic_error("Файл для дешифрования пуст");
                }

                string dec = rsa_decode(enc, current_d, current_n);

                if (dec.empty())
                {
                    throw logic_error("Результат дешифрования пуст - проверьте ключи");
                }

                string outputFile = getOutputFilename("RSAdecrypted.txt", "дешифрования");
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