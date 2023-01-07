#pragma once

#include "BaseHeader.h"

template<class T> struct IsClass
{
	template<class C> static char Test(void (C::*)(void));
	template<class C> static int Test(...);
	enum { yes = sizeof(Test<T>(0)) == 1 ? 1 : 0, no = yes ^ 1 };
};

template<class T>
constexpr inline float HeapScore(const T& n) { return n; }

struct Sortless
{
	template<class T> bool operator()(const T& x, const T& y) const { return x < y; }
	bool operator()(char* x, char* y) const { return strcmp(x, y) < 0; }
	bool operator()(const char* x, const char* y) const { return strcmp(x, y) < 0; }
};

struct SortNameless
{
	template<class T> bool operator()(const T& x, const T& y) const { return Sortless()(x.name, y.name); }
	template<class T> bool operator()(T* x, T* y) const { return Sortless()(x->name, y->name); }
	template<class T> bool operator()(const T* x, const T* y) const { return Sortless()(x->name, y->name); }
};

template<class T, class F>
inline void InsertionSort(T* start, T* end, F fun)
{
	for (T* i = start + 1; i < end; i++)
	{
		if (fun(*i, i[-1]))
		{
			T tmp = *i;
			*i = i[-1];
			T* j = i - 1;
			for (; j > start && fun(tmp, j[-1]); --j)
				*j = j[-1];
			*j = tmp;
		}
	}
}

template<class T, class F>
inline void InsertionSort(T* buf, int n, F fun)
{
	InsertionSort(buf, buf + n, fun);
}

template<class T>
inline void InsertionSort(T* buf, int n)
{
	InsertionSort(buf, buf + n, Sortless());
}

template<class T, class F>
inline void QuickSort(T* start, T* end, F fun)
{
	while (end - start > 10)
	{
		T* mid = &start[(end - start) / 2], * i = start + 1, * j = end - 2, pivot;
		if (fun(*start, *mid)) /* start < mid */
		{
			if (fun(end[-1], *start)) { pivot = *start; *start = end[-1]; end[-1] = *mid; } /* end < start < mid */
			else if (fun(end[-1], *mid)) { pivot = end[-1]; end[-1] = *mid; } /* start <= end < mid */
			else { pivot = *mid; } /* start < mid <= end */
		}
		else if (fun(*start, end[-1])) { pivot = *start; *start = *mid; } /*mid <= start < end */
		else if (fun(*mid, end[-1])) { pivot = end[-1]; end[-1] = *start; *start = *mid; } /* mid < end <= start */
		else { pivot = *mid; Swap(*start, end[-1]); }  /* end <= mid <= start */
		*mid = end[-2];
		do
		{
			while (fun(*i, pivot)) if (++i >= j) goto partitioned;
			while (fun(pivot, *--j)) if (i >= j) goto partitioned;
			Swap(*i, *j);
		} while (++i < j);
	partitioned:
		end[-2] = *i;
		*i = pivot;

		if (i - start < end - (i + 1))
		{
			QuickSort(start, i, fun);
			start = i + 1;
		}
		else
		{
			QuickSort(i + 1, end, fun);
			end = i;
		}
	}

	InsertionSort(start, end, fun);
}

template<class T, class F>
inline void QuickSsort(T* buf, int n, F fun)
{
	QuickSort(buf, buf + n, fun);
}

template<class T>
inline void QuickSort(T* buf, int n)
{
	QuickSort(buf, buf + n, Sortless());
}

inline uint32_t HTHash(const char* key)
{
	uint32_t h = 5381;
	for (int i = 0, k; (k = key[i]); i++) h = ((h << 5) + h) ^ k;    // bernstein k=33 xor
	return h;
}

inline bool HTCmp(const char* x, const char* y)
{
	return !strcmp(x, y);
}

template <class T>
class DataBuf
{
public:
	enum
	{
		OVERREAD = 1 << 0,
		OVERWROTE = 1 << 1
	};

	DataBuf() = default;
	template<class U>
	DataBuf(T* buf, U maxlen) : buf(buf), len(0), maxlen((int)maxlen), flags(0) {}

	void Reset()
	{
		len = 0;
		flags = 0;
	}

	void Reset(T* buf_, int maxlen_)
	{
		Reset();
		buf = buf_;
		maxlen = maxlen_;
	}

	const T& Get()
	{
		static T overreadval = 0;
		if (len < maxlen) return buf[len++];
		flags |= OVERREAD;
		return overreadval;
	}

	DataBuf SubBuf(int sz)
	{
		sz = Clamp(sz, 0, maxlen - len);
		len += sz;
		return databuf(&buf[len - sz], sz);
	}

	T* Pad(int numvals)
	{
		T* vals = &buf[len];
		len += Min(numvals, maxlen - len);
		return vals;
	}

	void Put(const T& val)
	{
		if (len < maxlen) buf[len++] = val;
		else flags |= OVERWROTE;
	}

	void Put(const T* vals, int numvals)
	{
		if (maxlen - len < numvals) flags |= OVERWROTE;
		memcpy(&buf[len], (const void*)vals, Min(maxlen - len, numvals) * sizeof(T));
		len += Min(maxlen - len, numvals);
	}

	int Get(T* vals, int numvals)
	{
		int read = Min(maxlen - len, numvals);
		if (read < numvals) flags |= OVERREAD;
		memcpy(vals, (void*)&buf[len], read * sizeof(T));
		len += read;
		return read;
	}

	void Offset(int n)
	{
		n = Min(n, maxlen);
		buf += n;
		maxlen -= n;
		len = Max(len - n, 0);
	}

	T* GetBuf() const { return buf; }
	bool Empty() const { return len == 0; }
	int Length() const { return len; }
	int Remaining() const { return maxlen - len; }
	bool Overread() const { return (flags & OVERREAD) != 0; }
	bool Overwrote() const { return (flags & OVERWROTE) != 0; }

	bool Check(int n) { return Remaining() >= n; }

	void ForceOverread()
	{
		len = maxlen;
		flags |= OVERREAD;
	}

	T* buf = nullptr;
	int len = 0;
	int maxlen = 0;
	uint8_t flags = 0;
};

template <class T>
class Vector
{
public:
	static const int MINSIZE = 8;

	Vector() = default;
	Vector(const Vector& v) : buf(nullptr), alen(0), ulen(0) { *this = v; }

	~Vector() { Shrink(0); if (buf) delete[](uint8_t*)buf; }

	Vector<T>& operator=(const Vector<T>& v)
	{
		Shrink(0);
		if (v.Length() > alen) GrowBuf(v.LSength());
		for(int i = 0; i < v.Length(); i++)
			Add(v[i]);
		return *this;
	}

	T& Add(const T& x)
	{
		if (ulen == alen) GrowBuf(ulen + 1);
		new (&buf[ulen]) T(x);
		return buf[ulen++];
	}

	T& Add()
	{
		if (ulen == alen) GrowBuf(ulen + 1);
		new (&buf[ulen]) T;
		return buf[ulen++];
	}

	T& Dup()
	{
		if (ulen == alen) GrowBuf(ulen + 1);
		new (&buf[ulen]) T(buf[ulen - 1]);
		return buf[ulen++];
	}

	void Move(Vector<T>& v)
	{
		if (!ulen)
		{
			Swap(buf, v.buf);
			Swap(ulen, v.ulen);
			Swap(alen, v.alen);
		}
		else
		{
			GrowBuf(ulen + v.ulen);
			if (v.ulen) memcpy(&buf[ulen], (void*)v.buf, v.ulen * sizeof(T));
			ulen += v.ulen;
			v.ulen = 0;
		}
	}

	bool InRange(size_t i) const { return i < size_t(ulen); }
	bool InRange(int i) const { return i >= 0 && i < ulen; }

	T& Pop() { return buf[--ulen]; }
	T& Last() { return buf[ulen - 1]; }
	void Drop() { ulen--; buf[ulen].~T(); }
	bool Empty() const { return ulen == 0; }

	int Capacity() const { return alen; }
	int Length() const { return ulen; }

	T& operator[](int i) { assert(i >= 0 && i < ulen); return buf[i]; }
	const T& operator[](int i) const { assert(i >= 0 && i < ulen); return buf[i]; }

	void Disown() { buf = nullptr; alen = ulen = 0; }

	void Shrink(int i) { assert(i <= ulen); if (IsClass<T>::no) ulen = i; else while (ulen > i) Drop(); }
	void SetSize(int i) { assert(i <= ulen); ulen = i; }

	void DeleteContents() { while (!Empty()) delete Pop(); }
	void DeleteArrays() { while (!Empty()) delete[] Pop(); }

	T* GetBuf() { return buf; }
	const T* GetBuf() const { return buf; }
	bool InBuf(const T* e) const { return e >= buf && e < &buf[ulen]; }

	template<class F>
	void Sort(F fun, int i = 0, int n = -1)
	{
		QuickSort(&buf[i], n < 0 ? ulen - i : n, fun);
	}

	void Sort() { Sort(Sortless()); }
	void SortName() { Sort(SortNameless()); }

	void GrowBuf(int sz)
	{
		int olen = alen;
		if (alen <= 0) alen = Max(MINSIZE, sz);
		else while (alen < sz) alen += alen / 2;
		if (alen <= olen) return;
		uint8_t* newbuf = new uint8_t[alen * sizeof(T)];
		if (olen > 0)
		{
			if (ulen > 0) memcpy(newbuf, (void*)buf, ulen * sizeof(T));
			delete[](uint8_t*)buf;
		}
		buf = (T*)newbuf;
	}

	DataBuf<T> Reserve(int sz)
	{
		if (alen - ulen < sz) GrowBuf(ulen + sz);
		return DataBuf<T>(&buf[ulen], sz);
	}

	void Advance(int sz)
	{
		ulen += sz;
	}

	void Addbuf(const DataBuf<T>& p)
	{
		Advance(p.Length());
	}

	T* Pad(int n)
	{
		T* buf = Reserve(n).buf;
		Advance(n);
		return buf;
	}

	void Put(const T& v) { Add(v); }

	void Put(const T* v, int n)
	{
		DataBuf<T> buf = Reserve(n);
		buf.Put(v, n);
		Addbuf(buf);
	}

	void Remove(int i, int n)
	{
		for (int p = i + n; p < ulen; p++) buf[p - n] = buf[p];
		ulen -= n;
	}

	T Remove(int i)
	{
		T e = buf[i];
		for (int p = i + 1; p < ulen; p++) buf[p - 1] = buf[p];
		ulen--;
		return e;
	}

	T RemoveUnordered(int i)
	{
		T e = buf[i];
		ulen--;
		if (ulen > 0) buf[i] = buf[ulen];
		return e;
	}

	template<class U>
	int Find(const U& o)
	{
		for(int i = 0; i < ulen; i++)
			if (buf[i] == o) return i;
		return -1;
	}

	void AddUnique(const T& o)
	{
		if (Find(o) < 0) Add(o);
	}

	void removeobj(const T& o)
	{
		for (int i = 0; i < ulen; i++)
			if (buf[i] == o)
			{
				int dst = i;
				for (int j = i + 1; j < ulen; j++) if (!(buf[j] == o)) buf[dst++] = buf[j];
				SetSize(dst);
				break;
			}
	}

	void ReplaceWithLast(const T& o)
	{
		if (!ulen) return;
		for (int i = 0; i < ulen - 1; i++)
			if (buf[i] == o)
			{
				buf[i] = buf[ulen - 1];
				break;
			}
		ulen--;
	}

	T& Insert(int i, const T& e)
	{
		Add(T());
		for (int p = ulen - 1; p > i; p--) buf[p] = buf[p - 1];
		buf[i] = e;
		return buf[i];
	}

	T* Insert(int i, const T* e, int n)
	{
		if (alen - ulen < n) GrowBuf(ulen + n);

		for (int j = 0; j < n; j++) Add(T());
		for (int p = ulen - 1; p >= i + n; p--) buf[p] = buf[p - n];
		for (int j = 0; j < n; j++) buf[i + j] = e[j];
		return &buf[i];
	}

	void Reverse()
	{
		for (int i = 0; i < ulen / 2; i++)
			Swap(buf[i], buf[ulen - 1 - i]);
	}

	static int HeapParent(int i) { return (i - 1) >> 1; }
	static int HeapChild(int i) { return (i << 1) + 1; }

	void BuildHeap()
	{
		for (int i = ulen / 2; i >= 0; i--) Downheap(i);
	}

	int Upheap(int i)
	{
		float score = HeapScore(buf[i]);
		while (i > 0)
		{
			int pi = HeapParent(i);
			if (score >= HeapScore(buf[pi])) break;
			Swap(buf[i], buf[pi]);
			i = pi;
		}
		return i;
	}

	T& Addheap(const T& x)
	{
		Add(x);
		return buf[Upheap(ulen - 1)];
	}

	int Downheap(int i)
	{
		float score = HeapScore(buf[i]);
		for (;;)
		{
			int ci = HeapChild(i);
			if (ci >= ulen) break;
			float cscore = HeapScore(buf[ci]);
			if (score > cscore)
			{
				if (ci + 1 < ulen && HeapScore(buf[ci + 1]) < cscore) { Swap(buf[ci + 1], buf[i]); i = ci + 1; }
				else { Swap(buf[ci], buf[i]); i = ci; }
			}
			else if (ci + 1 < ulen && HeapScore(buf[ci + 1]) < score) { Swap(buf[ci + 1], buf[i]); i = ci + 1; }
			else break;
		}
		return i;
	}

	T RemoveHeap()
	{
		T e = RemoveUnordered(0);
		if (ulen) Downheap(0);
		return e;
	}

	template<class K>
	int HTFind(const K& key)
	{
		for (int i = 0; i < ulen; i++)
		{
			if (HTCmp(key, buf[i])) return i;
		}
		return -1;
	}

	T* buf = nullptr;
	int alen = 0;
	int ulen = 0;
};

template<class H, class E, class K, class T>
struct HashBase
{
	typedef E elemtype;
	typedef K keytype;
	typedef T datatype;

	enum { CHUNKSIZE = 64 };

	struct chain { E elem; chain* next; };
	struct chainchunk { chain chains[CHUNKSIZE]; chainchunk* next; };

	int size;
	int numelems;
	chain** chains;

	chainchunk* chunks;
	chain* unused;

	enum { DEFAULTSIZE = 1 << 10 };

	HashBase(int size = DEFAULTSIZE) : size(size)
	{
		numelems = 0;
		chunks = nullptr;
		unused = nullptr;
		chains = new chain*[size];
		memset(chains, 0, size * sizeof(chain*));
	}

	~HashBase()
	{
		delete[] chains; chains = nullptr;
		DeleteChunks();
	}

	chain* Insert(uint32_t h)
	{
		if (!unused)
		{
			chainchunk* chunk = new chainchunk;
			chunk->next = chunks;
			chunks = chunk;
			for (int i = 0; i < CHUNKSIZE - 1; i++)
				chunk->chains[i].next = &chunk->chains[i + 1];

			chunk->chains[CHUNKSIZE - 1].next = unused;
			unused = chunk->chains;
		}
		chain* c = unused;
		unused = unused->next;
		c->next = chains[h];
		chains[h] = c;
		numelems++;
		return c;
	}

	template<class U>
	T& Insert(uint32_t h, const U& key)
	{
		chain* c = Insert(h);
		H::SetKey(c->elem, key);
		return H::GetData(c->elem);
	}

#define HTFIND(success, fail) \
	uint32_t h = HTHash(key)&(this->size-1); \
	for(chain *c = this->chains[h]; c; c = c->next) \
	{ \
		if(HTCmp(key, H::GetKey(c->elem))) return success H::GetData(c->elem); \
	} \
	return (fail);

	template<class U>
	T* Access(const U& key)
	{
		HTFIND(&, nullptr);
	}

	template<class U, class V>
	T& Access(const U& key, const V& elem)
	{
		HTFIND(, Insert(h, key) = elem);
	}

	template<class U>
	T& operator[](const U& key)
	{
		HTFIND(, Insert(h, key));
	}

	template<class U>
	T& Find(const U& key, T& notfound)
	{
		HTFIND(, notfound);
	}

	template<class U>
	const T& Find(const U& key, const T& notfound)
	{
		HTFIND(, notfound);
	}

	template<class U>
	bool Remove(const U& key)
	{
		uint32_t h = HTHash(key) & (size - 1);
		for (chain** p = &chains[h], *c = chains[h]; c; p = &c->next, c = c->next)
		{
			if (HTCmp(key, H::GetKey(c->elem)))
			{
				*p = c->next;
				c->elem.~E();
				new (&c->elem) E;
				c->next = unused;
				unused = c;
				numelems--;
				return true;
			}
		}
		return false;
	}

	void DeleteChunks()
	{
		for (chainchunk* nextchunk; chunks; chunks = nextchunk)
		{
			nextchunk = chunks->next;
			delete chunks;
		}
	}

	void Clear()
	{
		if (!numelems) return;
		memset(chains, 0, size * sizeof(chain*));
		numelems = 0;
		unused = nullptr;
		DeleteChunks();
	}

	static inline chain* EnumNext(void* i) { return ((chain*)i)->next; }
	static inline K& EnumKey(void* i) { return H::GetKey(((chain*)i)->elem); }
	static inline T& EnumData(void* i) { return H::GetData(((chain*)i)->elem); }
};

template<class T>
struct HashSet : HashBase<HashSet<T>, T, T, T>
{
	typedef HashBase<HashSet<T>, T, T, T> basetype;

	HashSet(int size = basetype::DEFAULTSIZE) : basetype(size) {}

	static inline const T& GetKey(const T& elem) { return elem; }
	static inline T& GetData(T& elem) { return elem; }
	template<class K> static inline void SetKey(T& elem, const K& key) {}

	template<class V>
	T& Add(const V& elem)
	{
		return basetype::Access(elem, elem);
	}
};

template<class T>
struct HashNameSet : HashBase<HashNameSet<T>, T, const char*, T>
{
	typedef HashBase<HashNameSet<T>, T, const char*, T> basetype;

	HashNameSet(int size = basetype::DEFAULTSIZE) : basetype(size) {}

	template<class U> static inline const char* GetKey(const U& elem) { return elem.name; }
	template<class U> static inline const char* GetKey(U* elem) { return elem->name; }
	static inline T& GetData(T& elem) { return elem; }
	template<class K> static inline void SetKey(T& elem, const K& key) {}

	template<class V>
	T& Add(const V& elem)
	{
		return basetype::Access(GetKey(elem), elem);
	}
};

template<class K, class T>
struct HashTableEntry
{
	K key;
	T data;
};

template<class K, class T>
struct HashTable : HashBase<HashTable<K, T>, HashTableEntry<K, T>, K, T>
{
	typedef HashBase<HashTable<K, T>, HashTableEntry<K, T>, K, T> basetype;
	typedef typename basetype::elemtype elemtype;

	HashTable(int size = basetype::DEFAULTSIZE) : basetype(size) {}

	static inline K& GetKey(elemtype& elem) { return elem.key; }
	static inline T& GetData(elemtype& elem) { return elem.data; }
	template<class U> static inline void SetKey(elemtype& elem, const U& key) { elem.key = key; }
};