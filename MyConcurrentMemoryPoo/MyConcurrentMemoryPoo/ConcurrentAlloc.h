#pragma once
#include "Common.h"
#include "ThreadCache.h"


static void* ConcurrentAlloc(size_t size) {
	if (pTLSThreadCache == nullptr) {
		pTLSThreadCache = new ThreadCache;
	}

	return pTLSThreadCache->Allocate(size);
}

static void ConcurrentFree(void* ptr,long long size) {
	assert(pTLSThreadCache);
	pTLSThreadCache->Deallcate(ptr, size);
}