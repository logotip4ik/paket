#pragma once

#include <filesystem>

#define lluint long long unsigned int
#define MAX_PATH_LENGTH 63
#define MAX_LEAFS_COUNT 100

// in bytes
#define MAX_FILE_SIZE 128000000 // 128mb

#define CHUNK_SIZE 4096

enum struct PktMode {
  Source,
  Dest,
};

enum struct AesMode {
  Encrypt,
  Decrypt,
};

enum struct LeafAttrs : unsigned char {
  Execution = 1,
  Folder = 1 << 7
};

const static char *PKT_HEADER = "PKT";
const static int PKT_HEADER_SIZE = sizeof(char) * 3;

const static short PKT_VERSION = 6;
const static short PKT_VERSION_SIZE = sizeof(PKT_VERSION);

const static char *BlacklistedPaths[] = {
  ".git",
  ".DS_Store"
};
const static short BlacklistedPathsLen = sizeof(BlacklistedPaths) / sizeof(BlacklistedPaths[0]);

namespace fs = std::filesystem;
