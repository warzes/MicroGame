#pragma once

#include "TempBase.h"

size_t decodeutf8(uchar* dst, size_t dstlen, const uchar* src, size_t srclen, size_t* carry = NULL);
size_t encodeutf8(uchar* dstbuf, size_t dstlen, const uchar* srcbuf, size_t srclen, size_t* carry = NULL);