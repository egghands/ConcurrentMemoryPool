#include "PageCache.h"
PageCache PageCache::_sInst;

Span* PageCache::NewSpan(size_t k)
{
	for (size_t i = k; i < PAGE_NUM; i++) {
		if (!_spanLists[i].Empty()) {
			Span* aSpan = _spanLists[i].PopFront();
			if (i == k)return aSpan;
			//����span�ķָ�
			Span* bSpan = new Span;
			bSpan->_pageid = aSpan->_pageid + k;
			bSpan->_n = aSpan->_n - k;
			aSpan->_n = k;
			_spanLists[bSpan->_n].PushFront(bSpan);
			return aSpan;
		}
	}
	//û��ƥ��Ĵ�ҳspan���Ҷ�Ҫ��ҳ
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(PAGE_NUM);
	bigSpan->_pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = PAGE_NUM - 1;  
}
