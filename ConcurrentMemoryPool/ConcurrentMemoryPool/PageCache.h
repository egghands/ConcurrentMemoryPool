#pragma once
#include "Common.h"
class PageCache
{

public:
	static PageCache* GetInstance() { return &_sInst; }
	//��ȡһ��kҳ��span
	Span* NewSpan(size_t k);
private:

	PageCache() {};
	PageCache(const PageCache&) = delete;
	SpanList _spanLists[PAGE_NUM];
	std::recursive_mutex _pageMtx;
	static PageCache _sInst;
};
