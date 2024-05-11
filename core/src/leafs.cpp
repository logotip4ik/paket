#include <iostream>
#include <format>

#include "leafs.h"
#include "files.h"

std::ostream& operator <<(std::ostream& stream, const Leaf leaf) {
  stream <<
    "[ path: " << leaf.path.c_str() <<
    ", isFolder: " << leaf.isFolder <<
    ", contents: " << leaf.contents <<
    ", length: " << leaf.length <<
    " ]";

  return stream;
}

Leaf traverse_path(std::string& _path) {
  Leaf leaf;

  leaf.path = fs::path(_path);
  leaf.length = 0;
  leaf.isFolder = fs::is_directory(_path);

  if (leaf.isFolder) {
    leaf.children = traverse_leaf(leaf);
  } else {
    leaf.length = getFileSize(leaf.path);
  }

  return leaf;
}

// we assume that everything that comes into here is a folder
std::vector<Leaf> traverse_leaf(Leaf& leaf) {
  std::vector<Leaf> children;

  for (const fs::directory_entry& entry : fs::directory_iterator(leaf.path)) {
    Leaf child;

    child.path = entry.path();
    child.length = 0;
    child.isFolder = entry.is_directory();

    if (child.isFolder) {
      child.children = traverse_leaf(child);
    } else {
      child.length = getFileSize(child.path);
    }

    children.push_back(child);
  }

  return children;
}

std::vector<Leaf> unwind(Leaf& leaf) {
  std::vector<Leaf> items;

  items.push_back(leaf);

  if (leaf.isFolder) {
    for (Leaf& child : leaf.children) {
      std::vector<Leaf> childrenItems = unwind(child);

      items.insert(items.end(), childrenItems.begin(), childrenItems.end());
    }
  }

  return items;
}
