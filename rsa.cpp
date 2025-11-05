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

// УБИРАЕМ определение safeSystemCall отсюда

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

long long modPower(long long b, long long e, long long m)
{
    long long r = 1;
    b %= m;
    while (e > 0)
    {
        if (e & 1)
            r = (r * b) % m;
        e >>= 1;
        b = (b * b) % m;
    }
    return r;
}

string rsa_encode(const string &text, int e, int n)
{
    string out;
    for (char c : text)
    {
        long long m = static_cast<unsigned char>(c);
        long long c_enc = modPower(m, e, n);
        out += to_string(c_enc) + " ";
    }
    return out;
}

string rsa_decode(const string &text, int d, int n)
{
    string out;
    istringstream iss(text);
    long long num;
    while (iss >> num)
    {
        long long dec = modPower(num, d, n);
        out += static_cast<char>(dec);
    }
    return out;
}

bool getCustomKeys(int &e, int &d, int &n)
{
    if (askUser("Хотите ввести свои ключи?"))
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
    int p = getRandomNumber(5000, 10000), q = getRandomNumber(5000, 10000);
    int n = p * q, phi = EilerFunc(p) * EilerFunc(q);
    int e = getRandomNumber(5000, phi);
    while (NOD(phi, e) != 1)
        e = getRandomNumber(5000, phi);
    int d = modInverse(e, phi);

    string userpass, text;
    int choice, src;
    while (true)
    {
        try
        {
            safeSystemCall(SYSTEM_CLEAR); // Используем из utils
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

                string enc = rsa_encode(text, e, n);
                string outputFile = getOutputFilename("RSAencrypted.txt", "шифрования");
                writeToFile(outputFile, enc);
                cout << "Зашифровано в " << outputFile << endl;
                this_thread::sleep_for(chrono::milliseconds(2000));
            }
            else
            {
                int current_e = e, current_d = d, current_n = n;
                if (!getCustomKeys(current_e, current_d, current_n))
                {
                    current_e = e;
                    current_d = d;
                    current_n = n;
                }

                string inputFile = getInputFilename("RSAencrypted.txt", "дешифрования");
                string enc = readFromFile(inputFile);
                string dec = rsa_decode(enc, current_d, current_n);

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