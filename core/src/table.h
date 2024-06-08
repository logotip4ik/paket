#pragma once

#include <vector>

#include "files.h"
#include "leafs.h"
#include "rw/rw.h"

#include "constants.h"

struct SerializedLeaf {
  unsigned char attrs = 0;
  char path[MAX_PATH_LENGTH] = {0};
  // where the file contents (bytes) starts in pkt file
  // they end with next leaf contents or up to the end of the file
  lluint contents = 0;
};

const static int SERIALIZED_LEAF_SIZE = sizeof(SerializedLeaf);

std::ostream &operator<<(std::ostream &stream, const SerializedLeaf leaf);

void makeTable(Buf *table, std::vector<SerializedLeaf> leafs);
void encryptBuf(Buf *key, Buf *iv, Buf *buf, Buf *encryptedBuf);
void decryptBuf(Buf *key, Buf *iv, Buf *encryptedBuf, Buf *buf);
std::vector<SerializedLeaf> parseTable(Buf *table);
std::vector<SerializedLeaf> serializeLeafs(std::vector<Leaf> &leafs, int baseOffset);
std::vector<Leaf> deserializeLeafs(std::vector<SerializedLeaf> &leafs, lluint pktFileSize);

bool isLeafMarked(const SerializedLeaf &leaf, LeafAttrs attr);
void markLeaf(SerializedLeaf &leaf, LeafAttrs attr);
