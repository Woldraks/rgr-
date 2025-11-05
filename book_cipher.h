#ifndef BOOK_CIPHER_H
#define BOOK_CIPHER_H

#include <string>
#include <vector>
#include <utility>

void bookCipher(std::string& password);
std::vector<std::pair<int, int>> book_encode(const std::string& txt, const std::string& book);
std::string book_decode(const std::vector<std::pair<int, int>>& cipher, const std::string& book);

#endif