#pragma once

#include <vector>

#include "files.h"
#include "leafs.h"
#include "rw/rw.h"

#include "constants.h"

struct SerializedLeaf {
  bool isFolder;
  char attrs;
  char path[MAX_PATH_LENGTH] = {0};
  // where the file contents (bytes) starts in pkt file
  // they end with next leaf contents or up to the end of the file
  lluint contents;
};

const static int SERIALIZED_LEAF_SIZE = sizeof(SerializedLeaf);

void makeTable(Buf *table, std::vector<SerializedLeaf> leafs);
void encryptTable(Buf *key, Buf *iv, Buf *table, Buf *encryptedTable);
void decryptTable(Buf *key, Buf *iv, Buf *encryptedTable, Buf *table);
std::vector<SerializedLeaf> parseTable(Buf *table);
std::vector<SerializedLeaf> serializeLeafs(std::vector<Leaf> &leafs, int baseOffset);
std::vector<Leaf> deserializeLeafs(std::vector<SerializedLeaf> &leafs, lluint pktFileSize);
