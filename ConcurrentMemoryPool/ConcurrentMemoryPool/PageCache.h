#pragma once
#include "Common.h"
class PageCache
{

public:
	static PageCache* GetInstance() { return &_sInst; }
	//获取一个k页的span
	Span* NewSpan(size_t k);
	std::recursive_mutex _pageMtx;
private:

	PageCache() {};
	PageCache(const PageCache&) = delete;
	SpanList _spanLists[PAGE_NUM];
	static PageCache _sInst;
};
