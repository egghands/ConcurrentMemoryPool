#pragma once
#include "Common.h"
#include "CentralCache.h"
#include "PageCache.h"
class ThreadCache
{
public:
	void* Allocate(size_t size);
	void Deallcate(void* ptr, size_t size);
	void* FetchFromCentralCache(size_t index, size_t alignSize);
	void ListTooLong(FreeList& list, size_t size);
private:
	FreeList _freeLists[NFREELIST];

};

#ifdef _WIN32
static _declspec (thread) ThreadCache* pTLSThreadCache = nullptr;
#elif __linux__
static thread_local ThreadCache* pTLSThreadCache = nullptr;
#endif // 


   