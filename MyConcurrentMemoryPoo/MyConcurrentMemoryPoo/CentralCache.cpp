#include "CentralCache.h"
#include "PageCache.h"
CentralCache CentralCache::_sInst;


size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t alignSize)
{
	size_t actualNum = 1;
	size_t index = SizeClass::Index(alignSize);
	//SpanList& spanList = _spanLists[index];
	_spanLists[index].Mtx().lock();//操作桶锁

	Span* newSpan = GetOneSpan(_spanLists[index], alignSize);
	assert(newSpan);
	assert(newSpan->_freeList);

	start = newSpan->_freeList;
	end = start;
	while(actualNum < batchNum && NextObj(end) != nullptr) {
		end = NextObj(end);
		++actualNum;
	}
	newSpan->_freeList = NextObj(end);
	NextObj(end) = nullptr;
	newSpan->_useCount += actualNum;

	_spanLists[index].Mtx().unlock();

	return actualNum;
}

Span* CentralCache::GetOneSpan(SpanList& list, size_t alignSize)
{
	Span* itr = list.Begin();
	while (itr != list.End()) {
		if (itr->_freeList != nullptr)
			return itr;
		itr = itr->_next;
	}
	//list没有而可用的span向下一层申请
	list.Mtx().unlock();//释放桶锁给其他线程
	PageCache::GetInstance()->Mtx().lock();

	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(alignSize));
	assert(span);

	PageCache::GetInstance()->Mtx().unlock();
	//对span进行切割
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = (span->_n << PAGE_SHIFT);
	char* end = start + bytes;
	span->_freeList = start;
	start += alignSize;
	void* tail = span->_freeList;
	int i = 0;
	while (start < end) {
		NextObj(tail) = start;
		tail = start;
		start += alignSize;
		i++;
	}
	NextObj(tail) = nullptr;
	list.Mtx().lock();
	list.PushFront(span);
	return span;
}

//-------------ToDo 未考虑加锁---------------
void CentralCache::ReleaseListToSpans(void* start, size_t byte_size)
{
	size_t index = SizeClass::Index(byte_size);
	SpanList& spanList = _spanLists[index];
	spanList.Mtx().lock();
	while (start) {
		void* next = NextObj(start);
		Span* span = PageCache::GetInstance()->MapObjToSpan(start);
		
		//将小对象头插入span的自由链表
		NextObj(start) = span->_freeList;
		span->_freeList = start;
		span->_useCount--;

		if (span->_useCount == 0) {//回收至下一层
			spanList.Erase(span);
			span->_freeList = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;

			spanList.Mtx().unlock();
			PageCache::GetInstance()->Mtx().lock();

			PageCache::GetInstance()->ReleaseSpanToPageCache(span);

			PageCache::GetInstance()->Mtx().unlock();
			spanList.Mtx().lock();

		}

		start = next;
	}
	spanList.Mtx().unlock();

}
