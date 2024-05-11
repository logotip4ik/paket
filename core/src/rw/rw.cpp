#include <iostream>
#include <bitset>

#include "./rw.h"

Buf::Buf(int size) {
  this->size = size;
  this->value = new unsigned char[size];
}

Buf::~Buf() {
  free(this->value);
}

PktRW::PktRW(PktRWOptions& options) {
  this->mode = options.mode;
  this->offset = options.offset;

  this->middleware = options.middleware;

  if (this->mode == PktMode::Source) {
    this->source = std::ifstream(options.pkt, std::ios::binary | std::ios::in);
    this->target = std::ofstream(options.target, std::ios::binary | std::ios::out);

    this->source.seekg(this->offset);
    if (this->source.fail()) {
      std::cout << "Pkt failed: " << strerror(errno) << std::endl;
    }
  } else if (this->mode == PktMode::Dest) {
    this->source = std::ifstream(options.target, std::ios::binary | std::ios::in);
    // out|in allows to constant reads and writes. Without this only last leaf contents would appear
    // in the .pkt file, everything before (until the table ?) would be overwritten with 0
    this->target = std::ofstream(options.pkt, std::ios::binary | std::ios::out | std::ios::in);

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

void PktRW::process() {
  int bytesRead;

  Buf inBuffer = Buf(CHUNK_SIZE);
  Buf outBuffer = Buf(CHUNK_SIZE);

  this->source.read((char*)inBuffer.value, inBuffer.size);

  while ((bytesRead = this->source.gcount()) > 0) {
    this->middleware.handle(&inBuffer, &outBuffer);

    this->target.write((char*)outBuffer.value, bytesRead);

    this->source.read((char*)inBuffer.value, inBuffer.size);
  }
}

void PktRW::process(lluint maxBytesToRead) {
  lluint totalBytesRead = 0;

  int bytesToRead;

  Buf inBuffer = Buf(CHUNK_SIZE);
  Buf outBuffer = Buf(CHUNK_SIZE);

  lluint chunkSize = inBuffer.size;

  do {
    bytesToRead = std::min(chunkSize, maxBytesToRead - totalBytesRead);
    this->source.read((char*)inBuffer.value, bytesToRead);
    totalBytesRead += bytesToRead;

    this->middleware.handle(&inBuffer, &outBuffer);

    this->target.write((char*)outBuffer.value, bytesToRead);
  } while (totalBytesRead < maxBytesToRead);

  /* bytesToRead = std::min(chunkSize, maxBytesToRead - totalBytesRead); */

  /* this->source.read((char*)inBuffer.value, bytesToRead); */
  /* totalBytesRead += bytesToRead; */

  /* while (totalBytesRead < maxBytesToRead) { */
  /*   this->middleware.handle(&inBuffer, &outBuffer); */

  /*   this->target.write((char*)outBuffer.value, bytesRead); */

  /*   bytesToRead = std::min(chunkSize, maxBytesToRead - totalBytesRead); */
  /*   this->source.read((char*)inBuffer.value, bytesToRead); */
  /*   totalBytesRead += bytesToRead; */
  /* } */
}
