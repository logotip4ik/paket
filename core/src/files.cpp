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

void rebuildFolderTree(std::vector<Leaf> &leafs) {
  for (const Leaf &leaf : leafs) {
    if (!isLeafMarked(leaf, LeafAttrs::Folder)) {
      continue;
    }

    fs::create_directories(leaf.path);
  }
}

void rebuildAttrsTree(std::vector<Leaf> &leafs) {
  for (const Leaf &leaf : leafs) {
    if (isLeafMarked(leaf, LeafAttrs::Folder) || leaf.attrs == 0) {
      continue;
    }

    fs::perms p;

    if (isLeafMarked(leaf, LeafAttrs::Execution)) {
      p |= fs::perms::owner_exec;
    }

    fs::permissions(leaf.path, p, fs::perm_options::add);
  }
}

unsigned char getPathAttrs(fs::path path) {
  unsigned char attrs = 0;

  fs::file_status s = fs::status(path);

  if (s.type() == fs::file_type::directory) {
    attrs |= (unsigned char)LeafAttrs::Folder;

    return attrs;
  }

  fs::perms p = s.permissions();

  if (fs::perms::none != (fs::perms::owner_exec & p)) {
    attrs |= (unsigned char)LeafAttrs::Execution;
  }

  return attrs;
}

bool isPathBlacklisted(fs::path path) {
  const char* checkpath;

  for (short i = 0; i < BlacklistedPathsLen; i++) {
    checkpath = BlacklistedPaths[i];

    if (path.filename().string() == checkpath) {
      return true;
    }
  }

 return false;
}
