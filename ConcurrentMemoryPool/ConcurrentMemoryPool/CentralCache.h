#pragma once
#include "Common.h"

class CentralCache
{
public:
	static CentralCache* GetInstance() { return &_sInst; }

	//获取一个非空span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	//从中心缓存获取一定的数量的对象给thread cache
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	void ReleaseListToSpans(void* start, size_t size);
private:
	SpanList _spanLists[FREElIST_NUM];
	CentralCache(){}
	CentralCache(const CentralCache&) = delete;
	static CentralCache _sInst;
};