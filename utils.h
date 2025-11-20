#ifndef UTILS_H
#define UTILS_H

#include <string>

extern const char *SYSTEM_CLEAR;

void writeToFile(const std::string &filename, const std::string &content);
std::string readFromFile(const std::string &filename);
void clearInputBuffer();
bool askUser(const std::string &question);
std::string getInputFilename(const std::string &defaultFile, const std::string &operation);
std::string getOutputFilename(const std::string &defaultFile, const std::string &operation);
void safeSystemCall(const char *command);

#endif