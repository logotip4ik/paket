#include "rw.h"

void PktMiddleware::handle(Buf* in, Buf* out) {
  memcpy(out->value, in->value, out->size);
}

