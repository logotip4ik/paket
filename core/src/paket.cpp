#include <iostream>

#include "paket.h"

#include "rw/rw.h"
#include "keys/keys.h"
#include "constants.h"
#include "table.h"

#define PATH_NOT_FOUND 1
#define NOT_VALID_HEADER 2
#define WRONG_KEY 3

int encrypt(std::string rootPath, std::string outputPath, std::string _key) {
  fs::file_status pathStatus = fs::status(rootPath);

  if (pathStatus.type() == fs::file_type::not_found) {
    return PATH_NOT_FOUND;
  }

  Buf key(32);
  memcpy(key.value, _key.c_str(), _key.size());
  Buf iv(16);
  makeIvKeyFromKey(std::string(_key), &iv);

  Leaf root = traverse_path(rootPath);

  fs::path output(outputPath);
  std::ofstream file = preparePktFile(output);

  std::vector<Leaf> leafs = unwind(root);

  const int baseOffset = PKT_HEADER_SIZE + PKT_VERSION_SIZE +
                         sizeof(int) // number of leafs
                         + SERIALIZED_LEAF_SIZE * leafs.size(); // table itself

  std::vector<SerializedLeaf> serialized = serializeLeafs(leafs, baseOffset);
#ifdef DEBUG
  std::cout << "base offset: " << baseOffset << std::endl;
#endif

  Buf table(SERIALIZED_LEAF_SIZE * leafs.size());
  Buf encryptedTable(SERIALIZED_LEAF_SIZE * leafs.size());

  makeTable(&table, serialized);
  encryptBuf(&key, &iv, &table, &encryptedTable);

  int leafsCount = serialized.size();
  Buf leafsCountBuf = Buf(sizeof(leafsCount));
  Buf encrypedLeafsCountBuf = Buf(sizeof(leafsCount));

  memcpy(leafsCountBuf.value, &leafsCount, sizeof(leafsCount));
  encryptBuf(&key, &iv, &leafsCountBuf, &encrypedLeafsCountBuf);

  // order is important
  file.write((char *)(encrypedLeafsCountBuf.value), encrypedLeafsCountBuf.size);
  file.write((char *)(encryptedTable.value), table.size);

  // todo: add multithreading
  /* std::vector<std::thread> threads(serialized.size()); */

  for (size_t i = 0; i < serialized.size(); i++) {
    const SerializedLeaf &leaf = serialized[i];

    if (leaf.isFolder) {
      continue;
    }

#ifdef DEBUG
    std::cout << leaf.path << " start: " << leaf.contents << std::endl;
#endif

    PktRWOptions options;

    options.mode = PktMode::Dest;
    options.pkt = output;
    options.offset = leaf.contents;
    options.target = leaf.path;

    PktRW rw(options);

    /* PktDummyMiddleware middleware = PktDummyMiddleware(); */
    PktAesMiddleware middleware = PktAesMiddleware(AesMode::Encrypt, key.value, iv.value);

    rw.process(middleware);
  }

  return 0;
}

int decrypt(std::string paket, std::string outputPath, std::string _key) {
  std::ifstream file(paket);

  if (!validateHeader(file)) {
    return NOT_VALID_HEADER;
  }

  Buf key(32);
  memcpy(key.value, _key.c_str(), _key.size());
  Buf iv(16);
  makeIvKeyFromKey(std::string(_key), &iv);

  int leafsCount;
  Buf encrypedLeafsCountBuf = Buf(sizeof(leafsCount));
  Buf leafsCountBuf = Buf(sizeof(leafsCount));

  file.read((char *)(encrypedLeafsCountBuf.value), encrypedLeafsCountBuf.size);

  decryptBuf(&key, &iv, &encrypedLeafsCountBuf, &leafsCountBuf);
  memcpy(&leafsCount, leafsCountBuf.value, leafsCountBuf.size);

  if (leafsCount > 99) {
    return WRONG_KEY;
  }

  Buf encryptedTable(SERIALIZED_LEAF_SIZE * leafsCount);
  Buf table(SERIALIZED_LEAF_SIZE * leafsCount);

  file.read((char *)(encryptedTable.value), encryptedTable.size);
  decryptBuf(&key, &iv, &encryptedTable, &table);

  std::vector<SerializedLeaf> serialized = parseTable(&table);
  std::vector<Leaf> leafs = deserializeLeafs(serialized, getFileSize(paket));

  for (Leaf &leaf : leafs) {
    leaf.path = fs::path(outputPath + "/" + leaf.path.string());

#ifdef DEBUG
    std::cout << leaf << std::endl;
#endif

    if (leaf.contents > 99999) {
#ifdef DEBUG
      std::cerr << "i mean, just for your computer safety..." << std::endl;
#endif
      exit(EXIT_FAILURE);
    }
  }

  rebuildFolderTree(leafs);

  // todo: add multithreading
  /* std::vector<std::thread> threads(serialized.size()); */
  for (const Leaf &leaf : leafs) {
    if (leaf.isFolder) {
      continue;
    }

#ifdef DEBUG
    std::cout << leaf.path << std::endl;
#endif

    PktRWOptions options;

    options.mode = PktMode::Source;
    options.pkt = paket;
    options.offset = leaf.contents;
    options.target = leaf.path;

    PktRW rw(options);

    /* PktDummyMiddleware middleware = PktDummyMiddleware(); */
    PktAesMiddleware middleware = PktAesMiddleware(AesMode::Encrypt, key.value, iv.value);

    rw.process(middleware, leaf.length);
  }

  rebuildAttrsTree(leafs);

  return 0;
}

