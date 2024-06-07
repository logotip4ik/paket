#include <format>
#include <iostream>

#include "files.h"
#include "leafs.h"

std::ostream &operator<<(std::ostream &stream, const Leaf leaf) {
  stream << "[ path: " << leaf.path.c_str()
         << ", isFolder: " << leaf.isFolder
         << ", attrs: " << (short)leaf.attrs
         << ", contents: " << leaf.contents
         << ", length: " << leaf.length
         << " ]";

  return stream;
}

Leaf traverse_path(std::string &_path, std::string &output) {
  Leaf leaf;

  if (isPathBlacklisted(_path)) {
    std::cout << "ERROR: the root file or folder is blacklisted -> " << _path << std::endl;
    exit(EXIT_FAILURE);
  }

  if (_path == output) {
    std::cout << "ERROR: the root file must not be the target -> " << _path << " -> " << output << std::endl;
    exit(EXIT_FAILURE);
  }

  leaf.path = fs::path(_path);
  leaf.isFolder = fs::is_directory(_path);

  if (leaf.isFolder) {
    leaf.children = traverse_leaf(leaf, output);
  } else {
    leaf.length = getFileSize(leaf.path);
    leaf.attrs = getFileAttrs(leaf.path);
  }

  return leaf;
}

// we assume that everything that comes into here is a folder
std::vector<Leaf> traverse_leaf(Leaf &leaf, std::string &output) {
  std::vector<Leaf> children;

  for (const fs::directory_entry &entry : fs::directory_iterator(leaf.path)) {
    Leaf child;

    child.path = entry.path();

    if (isPathBlacklisted(child.path)) {
#ifdef DEBUG
      std::cout << "WARNING: this file or folder is blacklisted, ignoring -> " << child.path << std::endl;
#endif
      continue;
    }

    if (child.path == output) {
#ifdef DEBUG
      std::cout << "WARNING: ignoring the output file -> " << output << std::endl;
#endif
      continue;
    }

    child.isFolder = entry.is_directory();

    if (child.isFolder) {
      child.children = traverse_leaf(child, output);
    } else {
      child.length = getFileSize(child.path);
      child.attrs = getFileAttrs(child.path);
    }

    children.push_back(child);
  }

  return children;
}

std::vector<Leaf> unwind(Leaf &leaf) {
  std::vector<Leaf> items;

  items.push_back(leaf);

  if (leaf.isFolder) {
    for (Leaf &child : leaf.children) {
      std::vector<Leaf> childrenItems = unwind(child);

      items.insert(items.end(), childrenItems.begin(), childrenItems.end());
    }
  }

  return items;
}
