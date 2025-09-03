#pragma once
#include "Common.h"
#include "ThreadCache.h"

class CentralCache
{
public:
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t alignSize);		//获取一组对象
	Span* GetOneSpan(SpanList& list, size_t alignSize);										//获取一个非空的span
	void ReleaseListToSpans(void* start, size_t byte_size);									//将自由链表中的元素回收


	static CentralCache* GetInstance() { return &_sInst; }
private:
	SpanList _spanLists[NFREELIST];
	
	//实现饿汉单例模式
	static CentralCache _sInst;

	CentralCache() {};
	CentralCache(const CentralCache&) = delete;
	CentralCache& operator = (const CentralCache&) = delete;
};