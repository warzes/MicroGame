#include "stdafx.h"
#include "FileSystem.h"
#include "TempBase.h"
#include "ZipFile.h"
#include "Streams.h"
#include "utf8Conv.h"

#ifdef _WIN32
#define NOMINMAX
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif


string homedir = "";

struct packagedir
{
	char* dir, *filter;
	size_t dirlen, filterlen;
};
Vector<packagedir> packagedirs;

char* makerelpath(const char* dir, const char* file, const char* prefix, const char* cmd)
{
	static string tmp;
	if (prefix) copystring(tmp, prefix);
	else tmp[0] = '\0';
	if (file[0] == '<')
	{
		const char* end = strrchr(file, '>');
		if (end)
		{
			size_t len = strlen(tmp);
			copystring(&tmp[len], file, std::min(sizeof(tmp) - len, size_t(end + 2 - file)));
			file = end + 1;
		}
	}
	if (cmd) concatstring(tmp, cmd);
	if (dir)
	{
		defformatstring(pname, "%s/%s", dir, file);
		concatstring(tmp, pname);
	}
	else concatstring(tmp, file);
	return tmp;
}

char* path(char* s)
{
	for (char* curpart = s;;)
	{
		char* endpart = strchr(curpart, '&');
		if (endpart) *endpart = '\0';
		if (curpart[0] == '<')
		{
			char* file = strrchr(curpart, '>');
			if (!file) return s;
			curpart = file + 1;
		}
		for (char* t = curpart; (t = strpbrk(t, "/\\")); *t++ = PATHDIV);
		for (char* prevdir = NULL, *curdir = curpart;;)
		{
			prevdir = curdir[0] == PATHDIV ? curdir + 1 : curdir;
			curdir = strchr(prevdir, PATHDIV);
			if (!curdir) break;
			if (prevdir + 1 == curdir && prevdir[0] == '.')
			{
				memmove(prevdir, curdir + 1, strlen(curdir + 1) + 1);
				curdir = prevdir;
			}
			else if (curdir[1] == '.' && curdir[2] == '.' && curdir[3] == PATHDIV)
			{
				if (prevdir + 2 == curdir && prevdir[0] == '.' && prevdir[1] == '.') continue;
				memmove(prevdir, curdir + 4, strlen(curdir + 4) + 1);
				if (prevdir - 2 >= curpart && prevdir[-1] == PATHDIV)
				{
					prevdir -= 2;
					while (prevdir - 1 >= curpart && prevdir[-1] != PATHDIV) --prevdir;
				}
				curdir = prevdir;
			}
		}
		if (endpart)
		{
			*endpart = '&';
			curpart = endpart + 1;
		}
		else break;
	}
	return s;
}

char* path(const char* s, bool copy)
{
	static string tmp;
	copystring(tmp, s);
	path(tmp);
	return tmp;
}

const char* parentdir(const char* directory)
{
	const char* p = directory + strlen(directory);
	while (p > directory && *p != '/' && *p != '\\') p--;
	static string parent;
	size_t len = p - directory + 1;
	copystring(parent, directory, len);
	return parent;
}

bool fileexists(const char* path, const char* mode)
{
	bool exists = true;
	if (mode[0] == 'w' || mode[0] == 'a') path = parentdir(path);
#ifdef _WIN32
	if (GetFileAttributesA(path[0] ? path : ".\\") == INVALID_FILE_ATTRIBUTES) exists = false;
#else
	if (access(path[0] ? path : ".", mode[0] == 'w' || mode[0] == 'a' ? W_OK : (mode[0] == 'd' ? X_OK : R_OK)) == -1) exists = false;
#endif
	return exists;
}

bool createdir(const char* path)
{
	size_t len = strlen(path);
	if (path[len - 1] == PATHDIV)
	{
		static string strip;
		path = copystring(strip, path, len);
	}
#ifdef _WIN32
	return CreateDirectoryA(path, NULL) != 0;
#else
	return mkdir(path, 0777) == 0;
#endif
}

size_t fixpackagedir(char* dir)
{
	path(dir);
	size_t len = strlen(dir);
	if (len > 0 && dir[len - 1] != PATHDIV)
	{
		dir[len] = PATHDIV;
		dir[len + 1] = '\0';
	}
	return len;
}

bool subhomedir(char* dst, int len, const char* src)
{
	const char* sub = strstr(src, "$HOME");
	if (!sub) sub = strchr(src, '~');
	if (sub && sub - src < len)
	{
#ifdef _WIN32
		char home[MAX_PATH + 1];
		home[0] = '\0';
		if (SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, home) != S_OK || !home[0]) return false;
#else
		const char* home = getenv("HOME");
		if (!home || !home[0]) return false;
#endif
		dst[sub - src] = '\0';
		concatstring(dst, home, len);
		concatstring(dst, sub + (*sub == '~' ? 1 : strlen("$HOME")), len);
	}
	return true;
}

const char* sethomedir(const char* dir)
{
	string pdir;
	copystring(pdir, dir);
	if (!subhomedir(pdir, sizeof(pdir), dir) || !fixpackagedir(pdir)) return NULL;
	copystring(homedir, pdir);
	return homedir;
}

const char* addpackagedir(const char* dir)
{
	string pdir;
	copystring(pdir, dir);
	if (!subhomedir(pdir, sizeof(pdir), dir) || !fixpackagedir(pdir)) return NULL;
	char* filter = pdir;
	for (;;)
	{
		static int len = strlen("packages");
		filter = strstr(filter, "packages");
		if (!filter) break;
		if (filter > pdir && filter[-1] == PATHDIV && filter[len] == PATHDIV) break;
		filter += len;
	}
	packagedir& pf = packagedirs.Add();
	pf.dir = filter ? newstring(pdir, filter - pdir) : newstring(pdir);
	pf.dirlen = filter ? filter - pdir : strlen(pdir);
	pf.filter = filter ? newstring(filter) : NULL;
	pf.filterlen = filter ? strlen(filter) : 0;
	return pf.dir;
}

const char* findfile(const char* filename, const char* mode)
{
	static string s;
	if (homedir[0])
	{
		formatstring(s, "%s%s", homedir, filename);
		if (fileexists(s, mode)) return s;
		if (mode[0] == 'w' || mode[0] == 'a')
		{
			string dirs;
			copystring(dirs, s);
			char* dir = strchr(dirs[0] == PATHDIV ? dirs + 1 : dirs, PATHDIV);
			while (dir)
			{
				*dir = '\0';
				if (!fileexists(dirs, "d") && !createdir(dirs)) return s;
				*dir = PATHDIV;
				dir = strchr(dir + 1, PATHDIV);
			}
			return s;
		}
	}
	if (mode[0] == 'w' || mode[0] == 'a') return filename;
	loopv(packagedirs)
	{
		packagedir& pf = packagedirs[i];
		if (pf.filter && strncmp(filename, pf.filter, pf.filterlen)) continue;
		formatstring(s, "%s%s", pf.dir, filename);
		if (fileexists(s, mode)) return s;
	}
	if (mode[0] == 'e') return NULL;
	return filename;
}

char* loadfile(const char* fn, size_t* size, bool utf8)
{
	stream* f = openfile(fn, "rb");
	if (!f) return NULL;
	stream::offset fsize = f->size();
	if (fsize <= 0) { delete f; return NULL; }
	size_t len = fsize;
	char* buf = new char[len + 1];
	if (!buf) { delete f; return NULL; }
	size_t offset = 0;
	if (utf8 && len >= 3)
	{
		if (f->read(buf, 3) != 3) { delete f; delete[] buf; return NULL; }
		if (((uchar*)buf)[0] == 0xEF && ((uchar*)buf)[1] == 0xBB && ((uchar*)buf)[2] == 0xBF) len -= 3;
		else offset += 3;
	}
	size_t rlen = f->read(&buf[offset], len - offset);
	delete f;
	if (rlen != len - offset) { delete[] buf; return NULL; }
	if (utf8) len = decodeutf8((uchar*)buf, len, (uchar*)buf, len);
	buf[len] = '\0';
	if (size != NULL) *size = len;
	return buf;
}

bool listdir(const char* dirname, bool rel, const char* ext, Vector<char*>& files)
{
	size_t extsize = ext ? strlen(ext) + 1 : 0;
#ifdef _WIN32
	defformatstring(pathname, rel ? ".\\%s\\*.%s" : "%s\\*.%s", dirname, ext ? ext : "*");
	WIN32_FIND_DATAA FindFileData;
	HANDLE Find = FindFirstFileA(pathname, &FindFileData);
	if (Find != INVALID_HANDLE_VALUE)
	{
		do {
			if (!ext) files.Add(newstring(FindFileData.cFileName));
			else
			{
				size_t namelen = strlen(FindFileData.cFileName);
				if (namelen > extsize)
				{
					namelen -= extsize;
					if (FindFileData.cFileName[namelen] == '.' && strncmp(FindFileData.cFileName + namelen + 1, ext, extsize - 1) == 0)
						files.Add(newstring(FindFileData.cFileName, namelen));
				}
			}
		} while (FindNextFileA(Find, &FindFileData));
		FindClose(Find);
		return true;
	}
#else
	defformatstring(pathname, rel ? "./%s" : "%s", dirname);
	DIR* d = opendir(pathname);
	if (d)
	{
		struct dirent* de;
		while ((de = readdir(d)) != NULL)
		{
			if (!ext) files.add(newstring(de->d_name));
			else
			{
				size_t namelen = strlen(de->d_name);
				if (namelen > extsize)
				{
					namelen -= extsize;
					if (de->d_name[namelen] == '.' && strncmp(de->d_name + namelen + 1, ext, extsize - 1) == 0)
						files.add(newstring(de->d_name, namelen));
				}
			}
		}
		closedir(d);
		return true;
	}
#endif
	else return false;
}

int listfiles(const char* dir, const char* ext, Vector<char*>& files)
{
	string dirname;
	copystring(dirname, dir);
	path(dirname);
	size_t dirlen = strlen(dirname);
	while (dirlen > 1 && dirname[dirlen - 1] == PATHDIV) dirname[--dirlen] = '\0';
	int dirs = 0;
	if (listdir(dirname, true, ext, files)) dirs++;
	string s;
	if (homedir[0])
	{
		formatstring(s, "%s%s", homedir, dirname);
		if (listdir(s, false, ext, files)) dirs++;
	}
	loopv(packagedirs)
	{
		packagedir& pf = packagedirs[i];
		if (pf.filter && strncmp(dirname, pf.filter, dirlen == pf.filterlen - 1 ? dirlen : pf.filterlen))
			continue;
		formatstring(s, "%s%s", pf.dir, dirname);
		if (listdir(s, false, ext, files)) dirs++;
	}
	dirs += listzipfiles(dirname, ext, files);
	return dirs;
}