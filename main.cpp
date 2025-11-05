#include <iostream>
#include <string>
#include <stdexcept>
#include "utils.h"
#include "book_cipher.h"
#include "binary_cipher.h"
#include "rsa.h"

using namespace std;

// Убираем повторное определение safeSystemCall, используем из utils.cpp

int main()
{
    string sysPass = "nstu", userSys, password = "nstu1", joke = "пирожковый колледж";
    char choice;

    cout << "Пароль системы: ";
    getline(cin, userSys);
    while (userSys != sysPass)
    {
        if (userSys == joke)
        {
            cout << "Какой пирожковый колледж, чел?\nОтчислен\n";
            cout << "       _______\n      /       \\\n     /  НКТП   \\\n    /___________\\\n"
                    "    |  _    _   |\n    | | |  | |  |\n    | | |  | |  |\n    |_|_|__|_|__|\n"
                    "Вот на что ты променял НГТУ\n";
            return -10;
        }
        cout << "Неверный пароль! Введите: ";
        getline(cin, userSys);
    }

    safeSystemCall(SYSTEM_CLEAR);
    while (true)
    {
        try
        {
            safeSystemCall(SYSTEM_CLEAR);
            cout << "------------------MENU------------------\n";
            cout << "1. Книжный шифр\n";
            cout << "2. Шифр последовательностей 0 и 1\n";
            cout << "3. Шифр RSA\n";
            cout << "4. Выход\n";
            cout << "Выбор: ";
            cin >> choice;

            if (choice != '1' && choice != '2' && choice != '3' && choice != '4')
                throw logic_error("Нет такой операции");

            if (choice == '4')
            {
                cout << " ______________\n|    _         |\n|   / \\        |     НГТУ НЭТИ\n"
                        "|  /   \\       |\n|       \\   /  |       АВТФ\n|        \\_/   |\n"
                        "|              |\n----------------\n";
                return 0;
            }

            switch (choice)
            {
            case '1':
                safeSystemCall(SYSTEM_CLEAR);
                bookCipher(password);
                break;
            case '2':
                safeSystemCall(SYSTEM_CLEAR);
                binarySequenceCipher(password);
                break;
            case '3':
                safeSystemCall(SYSTEM_CLEAR);
                rsa(password);
                break;
            }
        }
        catch (const exception &e)
        {
            cerr << "Ошибка: " << e.what() << endl;
            cin.clear();
            cin.ignore(256, '\n');
        }
    }
}