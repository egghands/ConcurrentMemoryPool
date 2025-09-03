#pragma once
#include "Common.h"
#include "CentralCache.h"
#include <unordered_map>
class PageCache
{
public:
	static PageCache* GetInstance() { return &_sInst; }			//单例
	Span* NewSpan(size_t npage);								//申请
	void ReleaseSpanToPageCache(Span* span);					//释放span到pageCache
	Span* MapObjToSpan(void* obj);
	std::recursive_mutex& Mtx() { return _mtx; }
private:
	SpanList _spanLists[NPAGES];
	static PageCache _sInst;
	std::recursive_mutex _mtx;
	std::unordered_map<PAGE_ID, Span*>_idSpanMap;				//方便回收小块内存

	PageCache() {};
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;

};