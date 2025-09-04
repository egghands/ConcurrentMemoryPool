#pragma once
#include "Common.h"
#include "ThreadCache.h"
#include "ObjectPool.h"

static void* ConcurrentAlloc(size_t size) {
	
	if (size > MAX_BYTES) {
		size_t alignSize = SizeClass::RoundUp(size);
		size_t kpage = alignSize >> PAGE_SHIFT;

		PageCache::GetInstance()->Mtx().lock();

		Span* bigSpan = PageCache::GetInstance()->NewSpan(kpage);
		assert(bigSpan);
		bigSpan->_isUse = true;
		bigSpan->_objSize = size;
		PageCache::GetInstance()->Mtx().unlock();
		void* p = (void*)(bigSpan->_pageId << PAGE_SHIFT);
		return p;
	}
	if (pTLSThreadCache == nullptr) {
		static ObjectPool<ThreadCache>objPool;
		//pTLSThreadCache = new ThreadCache;
		pTLSThreadCache = objPool.New();;
	}

	return pTLSThreadCache->Allocate(size);
}

static void ConcurrentFree(void* ptr) {
	Span* span = PageCache::GetInstance()->MapObjToSpan(ptr);
	size_t size = span->_objSize;
	if (size > MAX_BYTES) {
		PageCache::GetInstance()->Mtx().lock();

		PageCache::GetInstance()->ReleaseSpanToPageCache(span);

		PageCache::GetInstance()->Mtx().unlock();
	}
	else{
		assert(pTLSThreadCache);
		pTLSThreadCache->Deallcate(ptr, size);
	}
}