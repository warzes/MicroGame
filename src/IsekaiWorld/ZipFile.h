#pragma once

struct stream;

bool findzipfile(const char* filename);
stream* openzipfile(const char* filename, const char* mode);
int listzipfiles(const char* dir, const char* ext, Vector<char*>& files);