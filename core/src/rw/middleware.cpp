#include <iostream>
#include <openssl/aes.h>
#include <openssl/evp.h>

#include "rw.h"

void PktDummyMiddleware::handle(Buf *in, Buf *out) {
  memcpy(out->value, in->value, out->size);
  out->wrote = in->read;
}

void PktDummyMiddleware::onFinish(Buf *in, Buf *out) {
  // noop
}

PktAesMiddleware::PktAesMiddleware(
  AesMode mode,
  const unsigned char *key,
  const unsigned char *iv
) {
  this->mode = mode;
  this->ctx = EVP_CIPHER_CTX_new();

  if (!this->ctx) {
    std::cerr << "Error creating cipher context\n";
    exit(EXIT_FAILURE);
  }

  switch (this->mode) {
  case (AesMode::Encrypt):
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, iv) != 1) {
      std::cerr << "Error initializing encryption\n";
      exit(EXIT_FAILURE);
    }
    break;
  case (AesMode::Decrypt):
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, key, iv) != 1) {
      std::cerr << "Error initializing decryption\n";
      exit(EXIT_FAILURE);
    }
    break;
  }
}

PktAesMiddleware::~PktAesMiddleware() { EVP_CIPHER_CTX_free(this->ctx); }

void PktAesMiddleware::handle(Buf *in, Buf *out) {
  switch (this->mode) {
  case AesMode::Encrypt:
    if (EVP_EncryptUpdate(ctx, out->value, &out->wrote, in->value, in->read) != 1) {
      std::cerr << "Error encrypting data\n";
      exit(EXIT_FAILURE);
    }
    break;
  case AesMode::Decrypt:
    if (EVP_DecryptUpdate(ctx, out->value, &out->wrote, in->value, in->read) != 1) {
      std::cerr << "Error decrypting data\n";
      exit(EXIT_FAILURE);
    }
    break;
  }
}

void PktAesMiddleware::onFinish(Buf *in, Buf *out) {
  switch (this->mode) {
  case AesMode::Encrypt:
    if (EVP_EncryptFinal_ex(ctx, out->value, &out->wrote) != 1) {
      std::cerr << "Error finalizing encryption\n";
      exit(EXIT_FAILURE);
    }
    break;
  case AesMode::Decrypt:
    if (EVP_DecryptFinal_ex(ctx, out->value, &out->wrote) != 1) {
      std::cerr << "Error finalizing decryption\n";
      exit(EXIT_FAILURE);
    }
    break;
  }
}
