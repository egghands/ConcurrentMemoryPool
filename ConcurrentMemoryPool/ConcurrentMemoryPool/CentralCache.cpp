#include "CentralCache.h"
#include "PageCache.h"
#include "Common.h"
CentralCache CentralCache::_sInst;

Span* CentralCache::GetOneSpan(SpanList& list, size_t byte_size)
{
	//查看是否有空闲的span
	Span* it = list.Begin();
	while (it != list.End()) {
		if (it->_freeList)
			return it;
		else
			it = it->_next;
	}
	//吧central cache的桶锁解除掉，其他线程可以释放内存对象
	list._mtx.unlock(); 
	//到这里，没有空闲的span
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(byte_size));
	PageCache::GetInstance()->_pageMtx.unlock();

	// 计算span的大块内存的起始地址和大块内存的大小（字节数）
	char* start = (char*)(span->_pageid << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// 切割span，并连接到spanlists上
	span->_freeList = start;
	void* tail = start;
	start += byte_size;
	while (start < end) {
		NextObj(tail) = start;
		tail = start;
		start += byte_size;
	}
	list._mtx.lock();
	list.PushFront(span);
	return span;
}


// 从中心缓存获取一定数量的对象给thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();

	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// 从span中获取batchNum个对象
	// 如果不够batchNum个，有多少拿多少
	start = span->_freeList;
	end = start;
	size_t i = 0;
	size_t actualNum = 1;
	while (i < batchNum - 1 && NextObj(end) != nullptr)
	{
		end = NextObj(end);
		++i;
		++actualNum;
	}
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;
	span->_useCount += actualNum;
	_spanLists[index]._mtx.unlock();

	return actualNum;
}
