#import "bitset-serialise.hpp"

static int charToInt(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

boost::dynamic_bitset<uchar> stringToBitset(const char* str, int size) {
  boost::dynamic_bitset<uchar> bitset(size);

  for (int i = size - 4; i > -4; i -= 4) {
    char part = str[(i + 3) / 4];
    int partInt = charToInt(part);

    for (int j = 0; j < 4; j++) {
      if (i + j >= 0) {
        bitset[i + j] = (partInt & (1 << (3 - j))) > 0;
      }
    }
  }

  return std::move(bitset);
}

std::string bitsetToString(const boost::dynamic_bitset<uchar> &bitset) {
  const int size = bitset.size();

  char resStr[size / 4 + 2];
  std::memset(resStr, '\0', sizeof(resStr));
  char resStrPart[2];

  for (int i = size - 4; i > -4; i -= 4) {
    int resInt = 0;

    for (int j = i; j < i + 4; ++j) {
      if (j >= 0) {
        resInt = (resInt << 1) | bitset[j];
      }
    }

    sprintf(resStrPart, "%X", resInt);

    resStr[(i + 3) / 4] = resStrPart[0];
    resInt = 0;
  }

  return resStr;
}
