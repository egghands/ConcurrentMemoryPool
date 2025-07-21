#include "PageCache.h"
PageCache PageCache::_sInst;

Span* PageCache::NewSpan(size_t k)
{
	for (size_t i = k; i < PAGE_NUM; i++) {
		if (!_spanLists[i].Empty()) {
			Span* aSpan = _spanLists[i].PopFront();
			if (i == k)return aSpan;
			//进行span的分割
			Span* bSpan = new Span;
			bSpan->_pageid = aSpan->_pageid + k;
			bSpan->_n = aSpan->_n - k;
			aSpan->_n = k;
			_spanLists[bSpan->_n].PushFront(bSpan);
			return aSpan;
		}
	}
	//没有匹配的大页span，找堆要大页
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(PAGE_NUM);
	bigSpan->_pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = PAGE_NUM - 1;  
}
