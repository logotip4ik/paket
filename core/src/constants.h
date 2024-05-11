#pragma once

#include <filesystem>

#define lluint long long unsigned int
#define MAX_PATH_LENGTH 64

const static char* PKT_HEADER = "PKT";
const static int PKT_HEADER_SIZE = sizeof(char) * 3;

const static short PKT_VERSION = 5;
const static short PKT_VERSION_SIZE = sizeof(PKT_VERSION);

namespace fs = std::filesystem;

