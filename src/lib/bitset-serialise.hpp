#pragma once

#import "../precompiled.h"

#import <cstring>

boost::dynamic_bitset<uchar> stringToBitset(const char* str, int size);
std::string bitsetToString(const boost::dynamic_bitset<uchar> &bitset);
