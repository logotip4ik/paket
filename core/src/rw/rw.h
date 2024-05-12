#pragma once

#include <fstream>
#include <openssl/evp.h>
#include <openssl/aes.h>

#include "../constants.h"

struct Buf {
  int size;
  int read = 0;
  int wrote = 0;
  unsigned char* value;

  Buf(int size);
  ~Buf();
};

class PktMiddleware {
public:
  void handle(Buf* source, Buf* target);
  void onFinish(Buf* source, Buf* target);
};

class PktAesMiddleware : public PktMiddleware {
private:
  AesMode mode;
  EVP_CIPHER_CTX* ctx;

public:
  PktAesMiddleware(AesMode mode, const unsigned char* key, const unsigned char* iv);
  ~PktAesMiddleware();

  void handle(Buf* source, Buf* target);
  void onFinish(Buf* source, Buf* target);
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
