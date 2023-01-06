#pragma once

#include "MicroString.h"

#ifdef _WIN32
#define PATHDIV '\\'
#else
#define PATHDIV '/'
#endif

extern string homedir;

char* makerelpath(const char* dir, const char* file, const char* prefix = NULL, const char* cmd = NULL);
char* path(char* s);
char* path(const char* s, bool copy);
const char* parentdir(const char* directory);
bool fileexists(const char* path, const char* mode);
bool createdir(const char* path);
size_t fixpackagedir(char* dir);
const char* sethomedir(const char* dir);
const char* addpackagedir(const char* dir);
const char* findfile(const char* filename, const char* mode);
char* loadfile(const char* fn, size_t* size, bool utf8 = true);
bool listdir(const char* dir, bool rel, const char* ext, Vector<char*>& files);
int listfiles(const char* dir, const char* ext, Vector<char*>& files);
