#include "keys.h"

#include <iostream>
#include <openssl/evp.h>

void makeIvKeyFromKey(std::string key, Buf* buf) {
  const EVP_MD *md = EVP_get_digestbyname("SHA256");
  if (md == NULL) {
    std::cerr << "someting wrong with digest name " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }

  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len;

  if (!EVP_DigestInit_ex2(mdctx, md, NULL)) {
    std::cerr << "Message digest initialization failed. " << strerror(errno) << std::endl;
    EVP_MD_CTX_free(mdctx);
    exit(1);
  }

  if (!EVP_DigestUpdate(mdctx, key.c_str(), key.size())) {
    std::cerr << "Message digest update failed. " << strerror(errno) << std::endl;
    EVP_MD_CTX_free(mdctx);
    exit(1);
  }

  if (!EVP_DigestFinal_ex(mdctx, md_value, &md_len)) {
    std::cerr << "Message digest finalization failed. " << strerror(errno) << std::endl;
    EVP_MD_CTX_free(mdctx);
    exit(1);
  }

  EVP_MD_CTX_free(mdctx);

  #ifndef __PERF
  if (buf->size > EVP_MAX_MD_SIZE) {
    std::cerr << "iv bug size MUST not be bigger then sha256 md" << std::endl;
    exit(EXIT_FAILURE);
  }
  #endif

  memcpy(buf->value, md_value, buf->size);
}
