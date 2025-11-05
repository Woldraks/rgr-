#ifndef BINARY_CIPHER_H
#define BINARY_CIPHER_H

#include <string>

void binarySequenceCipher(std::string& password);
std::string textToBinary(const std::string& text);
std::string binaryToText(const std::string& binary);
std::string seq_encode(const std::string& txt);
std::string seq_decode(const std::string& c);

#endif