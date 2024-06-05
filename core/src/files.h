#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "constants.h"
#include "leafs.h"

std::ofstream preparePktFile(fs::path filename);
bool validateHeader(std::ifstream &os);
lluint getFileSize(fs::path path);
char getFileAttrs(fs::path path);
bool isPathBlacklisted(fs::path path);

// it filters out files inside
void rebuildFolderTree(std::vector<Leaf> &leafs);
void rebuildAttrsTree(std::vector<Leaf> &leafs);
