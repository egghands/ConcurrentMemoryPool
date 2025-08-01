#pragma once
#include "Common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);
	void* FetchFromCentralCache(size_t index,size_t size);
private:
	FreeList _freeLists[NFREELIST];
};

// every threads has its ThreadCache
static __declspec(thread) ThreadCache* pTLSThreadCache = nullptr;