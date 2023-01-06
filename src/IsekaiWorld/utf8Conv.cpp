#include "stdafx.h"
#include "utf8Conv.h"

size_t decodeutf8(uchar* dstbuf, size_t dstlen, const uchar* srcbuf, size_t srclen, size_t* carry)
{
	uchar* dst = dstbuf, * dstend = &dstbuf[dstlen];
	const uchar* src = srcbuf, * srcend = &srcbuf[srclen];
	if (dstbuf == srcbuf)
	{
		int len = std::min(dstlen, srclen);
		for (const uchar* end4 = &srcbuf[len & ~3]; src < end4; src += 4) if (*(const int*)src & 0x80808080) goto decode;
		for (const uchar* end = &srcbuf[len]; src < end; src++) if (*src & 0x80) goto decode;
		if (carry) *carry += len;
		return len;
	}

decode:
	dst += src - srcbuf;
	while (src < srcend && dst < dstend)
	{
		int c = *src++;
		if (c < 0x80) *dst++ = c;
		else if (c >= 0xC0)
		{
			int uni;
			if (c >= 0xE0)
			{
				if (c >= 0xF0)
				{
					if (c >= 0xF8)
					{
						if (c >= 0xFC)
						{
							if (c >= 0xFE) continue;
							uni = c & 1; if (srcend - src < 5) break;
							c = *src; if ((c & 0xC0) != 0x80) continue; src++; uni = (uni << 6) | (c & 0x3F);
						}
						else { uni = c & 3; if (srcend - src < 4) break; }
						c = *src; if ((c & 0xC0) != 0x80) continue; src++; uni = (uni << 6) | (c & 0x3F);
					}
					else { uni = c & 7; if (srcend - src < 3) break; }
					c = *src; if ((c & 0xC0) != 0x80) continue; src++; uni = (uni << 6) | (c & 0x3F);
				}
				else { uni = c & 0xF; if (srcend - src < 2) break; }
				c = *src; if ((c & 0xC0) != 0x80) continue; src++; uni = (uni << 6) | (c & 0x3F);
			}
			else { uni = c & 0x1F; if (srcend - src < 1) break; }
			c = *src; if ((c & 0xC0) != 0x80) continue; src++; uni = (uni << 6) | (c & 0x3F);
			c = uni2cube(uni);
			if (!c) continue;
			*dst++ = c;
		}
	}
	if (carry) *carry += src - srcbuf;
	return dst - dstbuf;
}

size_t encodeutf8(uchar* dstbuf, size_t dstlen, const uchar* srcbuf, size_t srclen, size_t* carry)
{
	uchar* dst = dstbuf, * dstend = &dstbuf[dstlen];
	const uchar* src = srcbuf, * srcend = &srcbuf[srclen];
	if (src < srcend && dst < dstend) do
	{
		int uni = cube2uni(*src);
		if (uni <= 0x7F)
		{
			if (dst >= dstend) goto done;
			const uchar* end = std::min(srcend, &src[dstend - dst]);
			do
			{
				if (uni == '\f')
				{
					if (++src >= srcend) goto done;
					goto uni1;
				}
				*dst++ = uni;
				if (++src >= end) goto done;
				uni = cube2uni(*src);
			} while (uni <= 0x7F);
		}
		if (uni <= 0x7FF) { if (dst + 2 > dstend) goto done; *dst++ = 0xC0 | (uni >> 6); goto uni2; }
		else if (uni <= 0xFFFF) { if (dst + 3 > dstend) goto done; *dst++ = 0xE0 | (uni >> 12); goto uni3; }
		else if (uni <= 0x1FFFFF) { if (dst + 4 > dstend) goto done; *dst++ = 0xF0 | (uni >> 18); goto uni4; }
		else if (uni <= 0x3FFFFFF) { if (dst + 5 > dstend) goto done; *dst++ = 0xF8 | (uni >> 24); goto uni5; }
		else if (uni <= 0x7FFFFFFF) { if (dst + 6 > dstend) goto done; *dst++ = 0xFC | (uni >> 30); goto uni6; }
		else goto uni1;
	uni6: *dst++ = 0x80 | ((uni >> 24) & 0x3F);
	uni5: *dst++ = 0x80 | ((uni >> 18) & 0x3F);
	uni4: *dst++ = 0x80 | ((uni >> 12) & 0x3F);
	uni3: *dst++ = 0x80 | ((uni >> 6) & 0x3F);
	uni2: *dst++ = 0x80 | (uni & 0x3F);
	uni1:;
	} while (++src < srcend);

done:
	if (carry) *carry += src - srcbuf;
	return dst - dstbuf;
}