#ifndef RSA_H
#define RSA_H

#include <string>

void rsa(std::string &password);
std::string rsa_encode(const std::string &text, int e, int n);
std::string rsa_decode(const std::string &text, int d, int n);
bool isPrime(int n);
int getRandomNumber(int min, int max);
int NOD(int a, int b);
int EilerFunc(int p);
int extendedEuclid(int a, int b, int &x, int &y);
int modInverse(int e, int phi);
long long modPower(long long b, long long e, long long m);
bool getCustomKeys(int &e, int &d, int &n);

#endif