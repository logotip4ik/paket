#include <iostream>

#include "paket.h"

#include "rw/rw.h"
#include "keys/keys.h"
#include "constants.h"
#include "table.h"

PaketRes encrypt(std::string rootPath, std::string outputPath, std::string _key) {
  fs::file_status pathStatus = fs::status(rootPath);

  if (pathStatus.type() == fs::file_type::not_found) {
    return PaketRes::PathNotFound;
  }

  Buf key(32);
  memcpy(key.value, _key.c_str(), _key.size());
  Buf iv(16);
  makeIvKeyFromKey(std::string(_key), &iv);

  fs::path output(outputPath);
  if (output.is_relative()) {
    output = fs::current_path() / output;
  }

  outputPath = output.string();

  Leaf root = traverse_path(rootPath, outputPath);
  std::ofstream file = preparePktFile(output);

  std::vector<Leaf> leafs = unwind(root);

  if (leafs.size() > MAX_LEAFS_COUNT) {
#ifdef DEBUG
    std::cout << "max leafs count reached: " << leafs.size()
              << " please increase this value in `constants.h`" << std::endl;
#endif

    return PaketRes::MaxLeafsCountReached;
  }

  const int baseOffset = PKT_HEADER_SIZE + PKT_VERSION_SIZE +
                         sizeof(int) // number of leafs
                         + SERIALIZED_LEAF_SIZE * leafs.size(); // table itself

#ifdef DEBUG
  std::cout << "base offset: " << baseOffset << std::endl;
#endif

  std::vector<SerializedLeaf> serialized = serializeLeafs(leafs, baseOffset);

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
    const SerializedLeaf &sleaf = serialized[i];

    if (sleaf.isFolder) {
      continue;
    }

#ifdef DEBUG
    std::cout << sleaf.path << " start: " << sleaf.contents << std::endl;
#endif

    PktRWOptions options;

    options.mode = PktMode::Dest;
    options.pkt = output;
    options.offset = sleaf.contents;
    // Because path in serialized leaf is chopped down to the lowest folder, we need to read file
    // contents from true file. Thus we take absolute (correct) path to file;
    options.target = leafs[i].path;

    PktRW rw(options);

    /* PktDummyMiddleware middleware = PktDummyMiddleware(); */
    PktAesMiddleware middleware = PktAesMiddleware(AesMode::Encrypt, key.value, iv.value);

    rw.process(middleware);
  }

  return PaketRes::Ok;
}

PaketRes decrypt(std::string paket, std::string outputPath, std::string _key) {
  std::ifstream file(paket);

  if (!validateHeader(file)) {
    return PaketRes::NotValidHeader;
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

  if (leafsCount > MAX_LEAFS_COUNT || leafsCount < 1) {
#ifdef DEBUG
    std::cout << "Received not valid leafs count number: " << leafsCount << std::endl;
#endif
    return PaketRes::WrongKey;
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

    if (leaf.length > MAX_FILE_SIZE) {
#ifdef DEBUG
      std::cerr << "probably received to big file MAX: " << MAX_FILE_SIZE << " received: " << leaf.length << std::endl;
#endif
      return PaketRes::WrongKey;
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

  return PaketRes::Ok;
}

