#pragma once
#include "Common.h"
#include "ObjectPool.h"
#include "CentralCache.h"
#include <unordered_map>
#include <map>
class PageCache
{
public:
	static PageCache* GetInstance() { return &_sInst; }			//单例
	Span* NewSpan(size_t npage);								//申请
	void ReleaseSpanToPageCache(Span* span);					//释放span到pageCache
	Span* MapObjToSpan(void* obj);
	std::mutex& Mtx() { return _pageMtx; }
private:
	SpanList _spanLists[NPAGES];
	static PageCache _sInst;
	std::mutex _pageMtx;
	//---------TODO将_idSpanMap改回来---------//
	std::map<PAGE_ID, Span*>_idSpanMap;				//方便回收小块内存
	ObjectPool<Span>_spanPool;

	PageCache() {};
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;

};