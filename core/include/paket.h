#pragma once

#include <string>

int encrypt(std::string rootPath, std::string outputPath, std::string _key);
int decrypt(std::string paketFile, std::string outputPath, std::string _key);
