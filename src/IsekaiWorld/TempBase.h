#pragma once

typedef signed char schar;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef signed long long int llong;
typedef unsigned long long int ullong;


#define loop(v,m) for(int v = 0; v < int(m); ++v)
#define loopi(m) loop(i,m)
#define loopj(m) loop(j,m)
#define loopk(m) loop(k,m)
#define loopl(m) loop(l,m)

#define DELETEP(p) if(p) { delete   p; p = 0; }
#define DELETEA(p) if(p) { delete[] p; p = 0; }

#define loopv(v)    for(int i = 0; i<(v).Length(); i++)
#define loopvj(v)   for(int j = 0; j<(v).Length(); j++)
#define loopvk(v)   for(int k = 0; k<(v).Length(); k++)
#define loopvrev(v) for(int i = (v).Length()-1; i>=0; i--)

#define enumeratekt(ht,k,e,t,f,b) loopi((ht).size) for(void *ec = (ht).chains[i]; ec;) { k &e = (ht).EnumKey(ec); t &f = (ht).EnumData(ec); ec = (ht).EnumNext(ec); b; }
#define enumerate(ht,t,e,b)       loopi((ht).size) for(void *ec = (ht).chains[i]; ec;) { t &e = (ht).EnumData(ec); ec = (ht).EnumNext(ec); b; }

inline int cube2uni(uchar c)
{
	extern const int cube2unichars[256];
	return cube2unichars[c];
}

inline uchar uni2cube(int c)
{
	extern const int uni2cubeoffsets[8];
	extern const uchar uni2cubechars[];
	return uint(c) <= 0x7FF ? uni2cubechars[uni2cubeoffsets[c >> 8] + (c & 0xFF)] : 0;
}