#include <iostream>

#include "files.h"

std::ofstream preparePktFile(fs::path filename) {
  std::ofstream file(
    filename.replace_extension("pkt"),
    std::ios::binary | std::ios::trunc
  );

  file.write(PKT_HEADER, PKT_HEADER_SIZE);
  file.write((char *)(&PKT_VERSION), PKT_VERSION_SIZE);

  return file;
}

bool validateHeader(std::ifstream &os) {
  std::string header = std::string(PKT_HEADER_SIZE, ' ');

  os.read((char *)(&header), PKT_HEADER_SIZE);

  if (header != PKT_HEADER) {
    return false;
  }

  int version;

  os.read((char *)(&version), PKT_VERSION_SIZE);

  if (version != PKT_VERSION) {
    return false;
  }

  return true;
}

lluint getFileSize(fs::path path) {
  std::ifstream ifs(path, std::ios::binary | std::ios::ate);
  std::ifstream::pos_type pos = ifs.tellg();

  return pos;
}

lluint padFileSize(lluint size) {
  int divisibleBy = 16;
  return size + (divisibleBy - (size % divisibleBy));
}

void rebuildFolderTree(std::vector<Leaf> &leafs) {
  for (const Leaf &leaf : leafs) {
    if (!leaf.isFolder) {
      continue;
    }

    fs::create_directories(leaf.path);
  }
}

void rebuildAttrsTree(std::vector<Leaf> &leafs) {
  for (const Leaf &leaf : leafs) {
    if (leaf.isFolder || leaf.attrs == 0) {
      continue;
    }

    fs::perms p;

    if (leaf.attrs & (char)FileAttrs::Execution) {
      p |= fs::perms::owner_exec;
    }

    fs::permissions(leaf.path, p, fs::perm_options::add);
  }
}

char getFileAttrs(fs::path path) {
  char attrs = 0;

  fs::file_status s = fs::status(path);

  if (s.type() == fs::file_type::directory) {
    return attrs;
  }

  fs::perms p = s.permissions();

  if (fs::perms::none != (fs::perms::owner_exec & p)) {
    attrs |= (char)FileAttrs::Execution;
  }

  std::cout << path.c_str() << " attrs: " << (short)attrs << std::endl;

  return attrs;
}
