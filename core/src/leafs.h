#pragma once

#include <string>
#include <vector>

#include "constants.h"

struct Leaf {
  bool isFolder;
  fs::path path;

  lluint length;
  // only used at deserialization to simplify workflow
  lluint contents;

  std::vector<Leaf> children;
};

std::ostream& operator <<(std::ostream& stream, const Leaf leaf);
Leaf traverse_path(std::string& path);
std::vector<Leaf> traverse_leaf(Leaf& leaf);
std::vector<Leaf> unwind(Leaf& leaf);
