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
  virtual void handle(Buf* source, Buf* target) = 0;
  virtual void onFinish(Buf* source, Buf* target) = 0;
};

class PktDummyMiddleware : public PktMiddleware {
public:
  void handle(Buf* source, Buf* target) override;
  void onFinish(Buf* source, Buf* target) override;
};

class PktAesMiddleware : public PktMiddleware {
private:
  AesMode mode;
  EVP_CIPHER_CTX* ctx;

public:
  PktAesMiddleware(AesMode mode, const unsigned char* key, const unsigned char* iv);
  ~PktAesMiddleware();

  void handle(Buf* source, Buf* target) override;
  void onFinish(Buf* source, Buf* target) override;
};

struct PktRWOptions {
  PktMode mode;
  lluint offset;
  fs::path pkt;
  fs::path target;
};

class PktRW {
private:
  PktMode mode;
  lluint offset;
  std::ifstream source;
  std::ofstream target;

public:
  PktRW(PktRWOptions& options);
  ~PktRW();

  void process(PktMiddleware& middleware);
  void process(PktMiddleware& middleware, lluint maxBytesToRead);
};
