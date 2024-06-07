#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <openssl/aes.h>
#include <openssl/evp.h>

#include "table.h"

std::ostream &operator<<(std::ostream &stream, const SerializedLeaf leaf) {
  stream << "[ path: " << leaf.path << ", isFolder: " << leaf.isFolder
         << ", contents: " << leaf.contents << ", attrs: " << short(leaf.attrs)
         << " ]";

  return stream;
}

std::vector<SerializedLeaf> serializeLeafs(std::vector<Leaf> &leafs, int baseOffset) {
  lluint prevContentsEnd = baseOffset;

  int filenameLength = leafs[0].path.filename().string().length();
  int totalPathLength = leafs[0].path.string().length();

  int absolutePathOffset = totalPathLength - filenameLength;

  std::vector<SerializedLeaf> serialized(leafs.size());

  for (size_t i = 0; i < leafs.size(); i++) {
    const char *path = leafs[i].path.c_str();
    const short pathLen = strlen(path) - absolutePathOffset;

    if (pathLen > MAX_PATH_LENGTH) {
      std::cout << std::string("path length must be less then ") +
                       std::to_string(MAX_PATH_LENGTH) +
                       std::string(" but encourted: ") + (path + absolutePathOffset)
                << std::endl;

      exit(EXIT_FAILURE);
    }

    memcpy(&serialized[i].path, path + absolutePathOffset, pathLen);

    serialized[i].attrs = leafs[i].attrs;
    serialized[i].isFolder = leafs[i].isFolder;
    serialized[i].contents = prevContentsEnd;

    prevContentsEnd += leafs[i].length;

#ifdef DEBUG
    std::cout << leafs[i] << " -> " << serialized[i] << std::endl;
#endif
  }

  return serialized;
}

void makeTable(Buf *table, std::vector<SerializedLeaf> leafs) {
  int offset;

  for (size_t i = 0; i < leafs.size(); i++) {
    offset = SERIALIZED_LEAF_SIZE * i;

    memcpy(table->value + offset, &leafs[i], SERIALIZED_LEAF_SIZE);
  }
}

std::vector<SerializedLeaf> parseTable(Buf *table) {
  const int leafsCount = table->size / SERIALIZED_LEAF_SIZE;
  std::vector<SerializedLeaf> leafs(leafsCount);

  int offset;
  for (size_t i = 0; i < leafsCount; i++) {
    offset = SERIALIZED_LEAF_SIZE * i;

    memcpy(&leafs[i], table->value + offset, SERIALIZED_LEAF_SIZE);
  }

  return leafs;
}

std::vector<Leaf> deserializeLeafs(std::vector<SerializedLeaf> &serialized, lluint pktFileSize) {
  int leafsCount = serialized.size();
  std::vector<Leaf> leafs(leafsCount);

  for (size_t i = 0; i < leafsCount; i++) {
    leafs[i].path = fs::path(serialized[i].path).lexically_normal();
    leafs[i].isFolder = serialized[i].isFolder;

    leafs[i].attrs = serialized[i].attrs;
    leafs[i].contents = serialized[i].contents;
    leafs[i].length = i == leafsCount - 1
                          ? pktFileSize - serialized[i].contents
                          : serialized[i + 1].contents - serialized[i].contents;
  }

  return leafs;
}

void encryptBuf(Buf *key, Buf *iv, Buf *buf, Buf *encryptedBuf) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
    std::cerr << "Error creating cipher context\n";
    exit(EXIT_FAILURE);
  }

  if (
    EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key->value, iv->value) != 1
  ) {
    std::cerr << "Error initializing encryption\n";
    exit(EXIT_FAILURE);
  }

  if (
    EVP_EncryptUpdate(ctx, encryptedBuf->value, &encryptedBuf->wrote, buf->value, buf->size) != 1
  ) {
    std::cerr << "Error encrypting data\n";
    exit(EXIT_FAILURE);
  }

  if (
    EVP_EncryptFinal_ex(ctx, encryptedBuf->value, &encryptedBuf->wrote) != 1
  ) {
    std::cerr << "Error finalizing encryption\n";
    exit(EXIT_FAILURE);
  }

  EVP_CIPHER_CTX_free(ctx);
}

void decryptBuf(Buf *key, Buf *iv, Buf *encryptedBuf, Buf *buf) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
    std::cerr << "Error creating cipher context\n";
    exit(EXIT_FAILURE);
  }

  if (
    EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key->value, iv->value) != 1
  ) {
    std::cerr << "Error initializing encryption\n";
    exit(EXIT_FAILURE);
  }

  if (
    EVP_DecryptUpdate(ctx, buf->value, &buf->wrote, encryptedBuf->value,encryptedBuf->size) != 1
  ) {
    std::cerr << "Error encrypting data\n";
    exit(EXIT_FAILURE);
  }

  if (EVP_DecryptFinal_ex(ctx, buf->value, &buf->wrote) != 1) {
    std::cerr << "Error finalizing encryption\n";
    exit(EXIT_FAILURE);
  }

  EVP_CIPHER_CTX_free(ctx);
}
