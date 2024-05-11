#include <iostream>
#include <algorithm>
#include <thread>

#include "leafs.h"
#include "files.h"
#include "serialize.h"
#include "rw/rw.h"

void encrypt(std::string rootPath) {
  fs::file_status pathStatus = fs::status(rootPath);

  if (pathStatus.type() == fs::file_type::not_found) {
    std::cout << "path not found" << std::endl;
    return;
  }

  Leaf root = traverse_path(rootPath);

  fs::path output("text.pkt");
  std::ofstream file = preparePktFile(output);

  std::vector<Leaf> leafs = unwind(root);

  const int baseOffset =
    PKT_HEADER_SIZE
    + PKT_VERSION_SIZE
    + sizeof(int) // number of leafs
    + SERIALIZED_LEAF_SIZE * leafs.size(); // table itself

  std::vector<SerializedLeaf> serialized = serializeLeafs(leafs, baseOffset);
  std::cout << "base offset: " << baseOffset << std::endl;

  // todo: encrypt table
  Buf table(SERIALIZED_LEAF_SIZE * leafs.size());
  makeTable(&table, serialized);

  int leafsCount = serialized.size();
  // order is important
  file.write((char*)(&leafsCount), sizeof(int));
  file.write((char*)(table.value), table.size);

  // todo: add multithreading
  /* std::vector<std::thread> threads(serialized.size()); */

  for (size_t i = 0; i < serialized.size(); i++) {
    const SerializedLeaf& leaf = serialized[i];

    if (leaf.isFolder) {
      continue;
    }

    std::cout << leaf.path << " start: " << leaf.contents << std::endl;

    PktRWOptions options;

    options.mode = PktMode::Dest;
    options.pkt = output;
    options.offset = leaf.contents;
    options.target = leaf.path;
    options.middleware = PktMiddleware();

    PktRW rw(options);

    rw.process();
  }
}

void decrypt(std::string paket) {
  std::ifstream file(paket);

  if (!validateHeader(file)) {
    std::cout << "Not valid header" << std::endl;
    return;
  }

  int leafsCount;
  file.read((char*)(&leafsCount), sizeof(leafsCount));

  Buf table(SERIALIZED_LEAF_SIZE * leafsCount);

  file.read((char*)(table.value), table.size);

  std::vector<SerializedLeaf> serialized = parseTable(&table);
  std::vector<Leaf> leafs = deserializeLeafs(serialized, getFileSize(paket));

  for (const Leaf& leaf : leafs) {
    std::cout << leaf << std::endl;
  }

  rebuildFolderTree(leafs);

  // todo: add multithreading
  /* std::vector<std::thread> threads(serialized.size()); */
  for (const Leaf& leaf: leafs) {
    if (leaf.isFolder) {
      continue;
    }

    std::cout << leaf.path << std::endl;

    PktRWOptions options;

    options.mode = PktMode::Source;
    options.pkt = paket;
    options.offset = leaf.contents;
    options.target = leaf.path;
    options.middleware = PktMiddleware();

    PktRW rw(options);

    rw.process(leaf.length);
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "requires at least one file" << std::endl;
    return 1;
  }

  std::string pktExt = ".pkt";
  std::string path = argv[1];
  if (std::equal(pktExt.rbegin(), pktExt.rend(), path.rbegin()) && fs::is_regular_file(path)) {
    decrypt(path);
  } else {
    encrypt(path);
  }

  return 0;
}
