#ifndef _INCLUDE_SYMBOLTABLE_PROPER_H_
#define _INCLUDE_SYMBOLTABLE_PROPER_H_

#include "../core/logic/common_logic.h"
#include <../core/logic/sm_symtable.h>
#include <sh_vector.h>
#include <string.h>
#include <../core/logic/MemoryUtils.h>
#include <sm_platform.h>

#ifdef PLATFORM_LINUX
#include <fcntl.h>
#include <link.h>
#include <sys/mman.h>
#include <gelf.h>
#include <cxxabi.h>

#define PAGE_SIZE			4096
#define PAGE_ALIGN_UP(x)	((x + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#endif
#ifdef PLATFORM_APPLE
#include <mach-o/dyld_images.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#endif

class DemangledSymbolTable
{
public:
	~DemangledSymbolTable()
	{
		for (uint32_t i = 0; i < nbuckets; i++)
		{
			Symbol *sym = buckets[i];
			while (sym != NULL)
			{
				Symbol *next = sym->tbl_next;
				free(sym);
				sym = next;
			}
		}
		free(buckets);
	}

	bool Initialize()
	{
		buckets = (Symbol **)malloc(sizeof(Symbol *) * KESTRING_TABLE_START_SIZE);
		if (buckets == NULL)
		{
			return false;
		}
		memset(buckets, 0, sizeof(Symbol *) * KESTRING_TABLE_START_SIZE);

		nbuckets = KESTRING_TABLE_START_SIZE;
		nused = 0;
		bucketmask = KESTRING_TABLE_START_SIZE - 1;
		return true;
	}

	static inline uint32_t HashString(const char *data, size_t len)
	{
		#undef get16bits
		#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
				|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
			#define get16bits(d) (*((const uint16_t *) (d)))
		#endif
		#if !defined (get16bits)
			#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
											 +(uint32_t)(((const uint8_t *)(d))[0]) )
		#endif
		uint32_t hash = len, tmp;
		int rem;

		if (len <= 0 || data == NULL)
		{
			return 0;
		}

		rem = len & 3;
		len >>= 2;

		/* Main loop */
		for (;len > 0; len--) {
				hash	+= get16bits (data);
				tmp		= (get16bits (data+2) << 11) ^ hash;
				hash	 = (hash << 16) ^ tmp;
				data	+= 2 * sizeof (uint16_t);
				hash	+= hash >> 11;
		}

		/* Handle end cases */
		switch (rem) {
				case 3: hash += get16bits (data);
								hash ^= hash << 16;
								hash ^= data[sizeof (uint16_t)] << 18;
								hash += hash >> 11;
								break;
				case 2: hash += get16bits (data);
								hash ^= hash << 11;
								hash += hash >> 17;
								break;
				case 1: hash += *data;
								hash ^= hash << 10;
								hash += hash >> 1;
		}

		/* Force "avalanching" of final 127 bits */
		hash ^= hash << 3;
		hash += hash >> 5;
		hash ^= hash << 4;
		hash += hash >> 17;
		hash ^= hash << 25;
		hash += hash >> 6;

		return hash;

		#undef get16bits
	}

	Symbol **FindSymbolBucket(const char *str, size_t len, uint32_t hash)
	{
		uint32_t bucket = hash & bucketmask;
		Symbol **pkvs = &buckets[bucket];

		Symbol *kvs = *pkvs;
		while (kvs != NULL)
		{
			if (len == kvs->length && memcmp(str, kvs->buffer(), len * sizeof(char)) == 0)
			{
				return pkvs;
			}
			pkvs = &kvs->tbl_next;
			kvs = *pkvs;
		}

		return pkvs;
	}

	Symbol *FindDemangledSymbol(const char *str, size_t len)
	{
		uint32_t hash = HashString(str, len);
		Symbol **pkvs = FindDemangledSymbolBucket(str, len, hash);
		return *pkvs;
	}

	Symbol **FindDemangledSymbolBucket(const char *str, size_t len, uint32_t hash)
	{
		uint32_t bucket = hash & bucketmask;
		Symbol **pkvs = &buckets[bucket];

		Symbol *kvs = *pkvs;
		while (kvs != NULL)
		{
			if (len == kvs->length && memcmp(str, kvs->buffer(), len * sizeof(char)) == 0)
			{
				return pkvs;
			}
			pkvs = &kvs->tbl_next;
			kvs = *pkvs;
		}

		return pkvs;
	}

	void ResizeSymbolTable()
	{
		uint32_t xnbuckets = nbuckets * 2;
		Symbol **xbuckets = (Symbol **)malloc(sizeof(Symbol *) * xnbuckets);
		if (xbuckets == NULL)
		{
			return;
		}
		memset(xbuckets, 0, sizeof(Symbol *) * xnbuckets);
		uint32_t xbucketmask = xnbuckets - 1;
		for (uint32_t i = 0; i < nbuckets; i++)
		{
			Symbol *sym = buckets[i];
			while (sym != NULL)
			{
				Symbol *next = sym->tbl_next;
				uint32_t bucket = sym->hash & xbucketmask;
				sym->tbl_next = xbuckets[bucket];
				xbuckets[bucket] = sym;
				sym = next;
			}
		}
		free(buckets);
		buckets = xbuckets;
		nbuckets = xnbuckets;
		bucketmask = xbucketmask;
	}

	Symbol *FindSymbol(const char *str, size_t len)
	{
		uint32_t hash = HashString(str, len);
		Symbol **pkvs = FindSymbolBucket(str, len, hash);
		return *pkvs;
	}

	Symbol *InternSymbol(const char* str, size_t len, void *address)
	{
		uint32_t hash = HashString(str, len);
		Symbol **pkvs = FindSymbolBucket(str, len, hash);
		if (*pkvs != NULL)
		{
			return *pkvs;
		}

		Symbol *kvs = (Symbol *)malloc(sizeof(Symbol) + sizeof(char) * (len + 1));
		kvs->length = len;
		kvs->hash = hash;
		kvs->address = address;
		kvs->tbl_next = NULL;
		memcpy(kvs + 1, str, sizeof(char) * (len + 1));
		*pkvs = kvs;
		nused++;

		if (nused > nbuckets && nbuckets <= INT_MAX / 2)
		{
			ResizeSymbolTable();
		}

		return kvs;
	}
private:
	uint32_t nbuckets;
	uint32_t nused;
	uint32_t bucketmask;
	Symbol **buckets;
};

void *ResolveDemangledSymbol(void* handle, const char* symbol, int& type, char* argsBuffer, unsigned int len);
void *GetSymbolAdress(const char *filename, const char *symbol);

#endif // _INCLUDE_SYMBOLTABLE_PROPER_H_
