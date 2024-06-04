#pragma once

#include <string>
#include <vector>

#include "constants.h"

struct Leaf {
  char attrs = 0;
  bool isFolder = false;
  fs::path path;

  lluint length = 0;
  // only used at deserialization to simplify workflow
  lluint contents = 0;

  std::vector<Leaf> children;
};

std::ostream &operator<<(std::ostream &stream, const Leaf leaf);
Leaf traverse_path(std::string &path, std::string &outputPath);
std::vector<Leaf> traverse_leaf(Leaf &leaf, std::string &outputPath);
std::vector<Leaf> unwind(Leaf &leaf);
