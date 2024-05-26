#pragma once

#include <string>

enum struct PaketRes {
  Ok,
  PathNotFound,
  NotValidHeader,
  WrongKey,
};

PaketRes encrypt(std::string rootPath, std::string outputPath, std::string _key);
PaketRes decrypt(std::string paketFile, std::string outputPath, std::string _key);
