#pragma once

#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "constants.h"
#include "leafs.h"

struct Header {
  char name[3];
  short version;
};

std::ofstream preparePktFile(fs::path filename);
bool validateHeader(std::ifstream& os);
lluint padFileSize(lluint size);
lluint getFileSize(fs::path path);

// it filters out files inside
void rebuildFolderTree(std::vector<Leaf>& leafs);

