#pragma once

#include <fstream>

#include "../constants.h"

#define CHUNK_SIZE 4096

enum struct PktMode {
  Source,
  Dest,
};

struct Buf {
  int size;
  unsigned char* value;

  Buf(int size);
  ~Buf();
};

class PktMiddleware {
public:
  void handle(Buf* source, Buf* target);
};

struct PktRWOptions {
  PktMode mode;
  lluint offset;
  fs::path pkt;
  fs::path target;

  PktMiddleware middleware;
};

class PktRW {
private:
  PktMode mode;
  lluint offset;
  std::ifstream source;
  std::ofstream target;

  PktMiddleware middleware;

public:
  PktRW(PktRWOptions& options);
  ~PktRW();

  void process();
  void process(lluint maxBytesToRead);
};
