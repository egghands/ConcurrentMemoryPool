#include "CentralCache.h"
#include "PageCache.h"
CentralCache CentralCache::_sInst;

Span* CentralCache::GetOneSpan(SpanList& list, size_t byte_size)
{
	//如果当前list中有直接返回
	Span* it = list.Begin();
	while (it != list.End()) {
		if (it->_freeList == nullptr)
			return it;
		else
			it = it->_next;
	}
	list._mtx.unlock();
	//找pageCache要
	PageCache::GetInstance()->PageMtx().lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(byte_size));
	PageCache::GetInstance()->PageMtx().unlock();
	
	//切分
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = (span->_n << PAGE_SHIFT);
	char* end = (char*)start + byte_size;
	span->_freeList = start;
	void* tail = start;
	start += byte_size;
	while (start < end) {
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += byte_size;
	}
	NextObj(tail) = nullptr;

	list._mtx.lock();
	list.PushFront(span);	
	return span;
}

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();
	Span* span = GetOneSpan(_spanLists[index], size);
	start = (void*)(span->_pageId << PAGE_SHIFT);
	end = start;
	int i = 1;
	int actualNum = 1;
	while (NextObj(end) != nullptr && i < batchNum) {
		end = NextObj(end);
		i++;
		actualNum++;
	}
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;
	span->_useCount += actualNum;
	_spanLists[index]._mtx.unlock();
	return actualNum;
}
