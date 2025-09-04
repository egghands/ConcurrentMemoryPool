#pragma once
#include "Common.h"
#include "ObjectPool.h"
#include "CentralCache.h"
#include <unordered_map>
#include <map>
class PageCache
{
public:
	static PageCache* GetInstance() { return &_sInst; }			//����
	Span* NewSpan(size_t npage);								//����
	void ReleaseSpanToPageCache(Span* span);					//�ͷ�span��pageCache
	Span* MapObjToSpan(void* obj);
	std::mutex& Mtx() { return _pageMtx; }
private:
	SpanList _spanLists[NPAGES];
	static PageCache _sInst;
	std::mutex _pageMtx;
	//---------TODO��_idSpanMap�Ļ���---------//
	std::map<PAGE_ID, Span*>_idSpanMap;				//�������С���ڴ�
	ObjectPool<Span>_spanPool;

	PageCache() {};
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;

};