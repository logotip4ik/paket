#include <bitset>
#include <iostream>

#include "./rw.h"

Buf::Buf(int size) {
  this->size = size;
  this->value = new unsigned char[size];
}

Buf::~Buf() {
  free(this->value);
}

PktRW::PktRW(PktRWOptions &options) {
  this->mode = options.mode;
  this->offset = options.offset;

  // NOTE: there is a possibility to write and read from the same file!
  // and this must not happen!!! It will eat all your ssd in seconds...

  if (this->mode == PktMode::Source) {
    this->source = std::ifstream(options.pkt, std::ios::binary | std::ios::in);
    this->target = std::ofstream(options.target, std::ios::binary | std::ios::out);

    this->source.seekg(this->offset);
    if (this->source.fail()) {
      std::cout << "Pkt failed: " << strerror(errno) << std::endl;
    }
  } else if (this->mode == PktMode::Dest) {
    this->source = std::ifstream(options.target, std::ios::binary | std::ios::in);
    // out|in allows to constant reads and writes. Without this only last leaf
    // contents would appear in the .pkt file, everything before (until the
    // table ?) would be overwritten with 0
    this->target = std::ofstream(options.pkt, std::ios::binary | std::ios::out |
                                                  std::ios::in);

    this->target.seekp(this->offset);
    if (this->target.fail() || this->target.bad()) {
      std::cout << "Pkt failed: " << strerror(errno) << std::endl;
    }
  }
}

PktRW::~PktRW() {
  this->source.close();
  this->target.close();
}

void PktRW::process(PktMiddleware &middleware) {
  Buf inBuffer = Buf(CHUNK_SIZE);
  Buf outBuffer = Buf(CHUNK_SIZE);

  this->source.read((char *)inBuffer.value, inBuffer.size);

  while ((inBuffer.read = this->source.gcount()) > 0) {
    middleware.handle(&inBuffer, &outBuffer);

#ifndef __PERF
    if (outBuffer.wrote != inBuffer.read) {
      std::cerr << "Bytes wrote and bytes read MUST match!" << std::endl;
      exit(EXIT_FAILURE);
    }
#endif

    this->target.write((char *)outBuffer.value, outBuffer.wrote);

    this->source.read((char *)inBuffer.value, inBuffer.size);
  }

  middleware.onFinish(&inBuffer, &outBuffer);
}

void PktRW::process(PktMiddleware &middleware, lluint maxBytesToRead) {
  lluint totalBytesRead = 0;

  Buf inBuffer = Buf(CHUNK_SIZE);
  Buf outBuffer = Buf(CHUNK_SIZE);

  lluint chunkSize = inBuffer.size;

  do {
    inBuffer.read = std::min(chunkSize, maxBytesToRead - totalBytesRead);
    this->source.read((char *)inBuffer.value, inBuffer.read);
    totalBytesRead += inBuffer.read;

    middleware.handle(&inBuffer, &outBuffer);

#ifndef __PERF
    if (outBuffer.wrote != inBuffer.read) {
      std::cerr << "Bytes wrote and bytes read MUST match!" << std::endl;
      exit(EXIT_FAILURE);
    }
#endif

    this->target.write((char *)outBuffer.value, outBuffer.wrote);
  } while (totalBytesRead < maxBytesToRead);

  middleware.onFinish(&inBuffer, &outBuffer);
}
