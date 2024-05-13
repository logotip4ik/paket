#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <openssl/evp.h>
#include <openssl/aes.h>

#include "table.h"

std::vector<SerializedLeaf> serializeLeafs(std::vector<Leaf>& leafs, int baseOffset) {
  lluint prevContentsEnd = baseOffset;

  std::vector<SerializedLeaf> serialized(leafs.size());

  for (size_t i = 0; i < leafs.size(); i++) {
    const char* path = leafs[i].path.c_str();

    if (strlen(path) > MAX_PATH_LENGTH) {
      throw std::length_error(
        std::string("path length must be less then ") + std::to_string(MAX_PATH_LENGTH) + std::string(" but encourted: ") + path
      );
    }

    memcpy(&serialized[i].path, path, MAX_PATH_LENGTH);

    serialized[i].contents = 0;

    serialized[i].isFolder = leafs[i].isFolder;
    serialized[i].contents = prevContentsEnd;

    prevContentsEnd = serialized[i].contents + leafs[i].length;
  }

  return serialized;
}

void makeTable(Buf* table, std::vector<SerializedLeaf> leafs) {
  int offset;

  for (size_t i = 0; i < leafs.size(); i++) {
    offset = SERIALIZED_LEAF_SIZE * i;

    memcpy(table->value + offset, &leafs[i], SERIALIZED_LEAF_SIZE);
  }
}

std::vector<SerializedLeaf> parseTable(Buf* table) {
  const int leafsCount = table->size / SERIALIZED_LEAF_SIZE;
  std::vector<SerializedLeaf> leafs(leafsCount);

  int offset;
  for (size_t i = 0; i < leafsCount; i++) {
    offset = SERIALIZED_LEAF_SIZE * i;

    memcpy(&leafs[i], table->value + offset, SERIALIZED_LEAF_SIZE);
  }

  return leafs;
}

std::vector<Leaf> deserializeLeafs(std::vector<SerializedLeaf>& serialized, lluint pktFileSize) {
  int leafsCount = serialized.size();
  std::vector<Leaf> leafs(leafsCount);

  for (size_t i = 0; i < leafsCount; i++) {
    leafs[i].path = fs::path(serialized[i].path);
    leafs[i].isFolder = serialized[i].isFolder;

    leafs[i].contents = serialized[i].contents;
    leafs[i].length = i == leafsCount - 1
      ? pktFileSize - serialized[i].contents
      : serialized[i + 1].contents - serialized[i].contents;
  }

  return leafs;
}

void encryptTable(Buf* key, Buf* iv, Buf* table, Buf* encryptedTable) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
      std::cerr << "Error creating cipher context\n";
      exit(EXIT_FAILURE);
  }

  if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key->value, iv->value) != 1) {
    std::cerr << "Error initializing encryption\n";
    exit(EXIT_FAILURE);
  }

  if (EVP_EncryptUpdate(ctx, encryptedTable->value, &encryptedTable->wrote, table->value, table->size) != 1) {
    std::cerr << "Error encrypting data\n";
    exit(EXIT_FAILURE);
  }

  if (EVP_EncryptFinal_ex(ctx, encryptedTable->value, &encryptedTable->wrote) != 1) {
    std::cerr << "Error finalizing encryption\n";
    exit(EXIT_FAILURE);
  }

  EVP_CIPHER_CTX_free(ctx);
}

void decryptTable(Buf* key, Buf* iv, Buf* encryptedTable, Buf* table) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
      std::cerr << "Error creating cipher context\n";
      exit(EXIT_FAILURE);
  }

  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key->value, iv->value) != 1) {
    std::cerr << "Error initializing encryption\n";
    exit(EXIT_FAILURE);
  }

  if (EVP_DecryptUpdate(ctx, table->value, &table->wrote, encryptedTable->value, encryptedTable->size) != 1) {
    std::cerr << "Error encrypting data\n";
    exit(EXIT_FAILURE);
  }

  if (EVP_DecryptFinal_ex(ctx, table->value, &table->wrote) != 1) {
    std::cerr << "Error finalizing encryption\n";
    exit(EXIT_FAILURE);
  }

  EVP_CIPHER_CTX_free(ctx);
}
