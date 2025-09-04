#pragma once
#include "Common.h"
#include "ObjectPool.h"
#include "CentralCache.h"
#include <unordered_map>
class PageCache
{
public:
	static PageCache* GetInstance() { return &_sInst; }			//����
	Span* NewSpan(size_t npage);								//����
	void ReleaseSpanToPageCache(Span* span);					//�ͷ�span��pageCache
	Span* MapObjToSpan(void* obj);
	std::mutex& Mtx() { return _mtx; }
private:
	SpanList _spanLists[NPAGES];
	static PageCache _sInst;
	std::mutex _mtx;
	std::unordered_map<PAGE_ID, Span*>_idSpanMap;				//�������С���ڴ�
	ObjectPool<Span>_spanPool;

	PageCache() {};
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;

};