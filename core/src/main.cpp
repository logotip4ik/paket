#include <iostream>

#include "paket.h"
#include "constants.h"

const static std::string _key = "someting";

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "requires at least one file" << std::endl;
    return 1;
  }

  std::string pktExt = ".pkt";
  std::string path = argv[1];
  if (std::equal(pktExt.rbegin(), pktExt.rend(), path.rbegin()) &&
      fs::is_regular_file(path)) {
    decrypt(path, ".", _key);
  } else {
    encrypt(path, "text.pkt", _key);
  }

  return 0;
}
