#pragma once
#include "Common.h"

class CentralCache
{
public:
	static CentralCache* GetInstance() { return &_sInst; }

	//��ȡһ���ǿ�span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	//�����Ļ����ȡһ���������Ķ����thread cache
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	void ReleaseListToSpans(void* start, size_t size);
private:
	SpanList _spanLists[FREElIST_NUM];
	CentralCache(){}
	CentralCache(const CentralCache&) = delete;
	static CentralCache _sInst;
};