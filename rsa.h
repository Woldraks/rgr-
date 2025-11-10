#ifndef RSA_H
#define RSA_H

#include <string>

bool isPrime(int n);
int getRandomNumber(int min, int max);
int NOD(int a, int b);
int EilerFunc(int p);
int extendedEuclid(int a, int b, int &x, int &y);
int modInverse(int e, int phi);
long long modPower(long long base, long long exponent, long long modulus);
std::string rsa_encode(const std::string &text, int e, long long n);
std::string rsa_decode(const std::string &text, int d, long long n);
bool getCustomKeys(int &e, int &d, long long &n);
void rsa(std::string &password);

#endif