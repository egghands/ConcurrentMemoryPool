#pragma once
#include "Common.h"
#include "ThreadCache.h"

class CentralCache
{
public:
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t alignSize);		//��ȡһ�����
	Span* GetOneSpan(SpanList& list, size_t alignSize);										//��ȡһ���ǿյ�span
	void ReleaseListToSpans(void* start, size_t byte_size);									//�����������е�Ԫ�ػ���


	static CentralCache* GetInstance() { return &_sInst; }
private:
	SpanList _spanLists[NFREELIST];
	
	//ʵ�ֶ�������ģʽ
	static CentralCache _sInst;

	CentralCache() {};
	CentralCache(const CentralCache&) = delete;
	CentralCache& operator = (const CentralCache&) = delete;
};