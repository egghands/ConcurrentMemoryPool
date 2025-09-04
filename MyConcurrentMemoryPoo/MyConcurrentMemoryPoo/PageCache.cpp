#include "PageCache.h"
#include "CentralCache.h"
//单例模式（饿汉）
PageCache PageCache::_sInst;

//-----申请逻辑----///

Span* PageCache::NewSpan(size_t npage)
{
	assert(npage > 0 && npage < NPAGES);
	SpanList& spanList = _spanLists[npage];
	if (!spanList.Empty())
	{
		Span* span =  spanList.PopFront();
		for (int i = 0; i < span->_n; i++)
			_idSpanMap[span->_pageId + i] = span;
		return span;
	}
	//当前spanlist中没有对应大小的span，像更大的list寻找，然后切割
	for (size_t i = npage + 1; i < NPAGES; i++) {
		if (!_spanLists[i].Empty())
		{
			Span* aSpan = _spanLists[i].PopFront();
			Span* bSpan = new Span;
			aSpan->_n = npage;

			bSpan->_n = i - npage;
			bSpan->_pageId = aSpan->_pageId + npage;
			_spanLists[bSpan->_n].PushFront(bSpan);

			for (size_t i = 0; i < aSpan->_n; i++) {
				_idSpanMap[aSpan->_pageId + i] = aSpan;
			}
				
			_idSpanMap[bSpan->_pageId] = bSpan;
			_idSpanMap[bSpan->_pageId + bSpan->_n - 1] = bSpan;

			return aSpan;
		}
	}
	//_spanList中没有大块内存,需要向系统申请
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_n = NPAGES - 1;
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	_spanLists[bigSpan->_n].PushFront(bigSpan);
	return NewSpan(npage);
}


//------释放逻辑------//
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	//对前后相邻span尝试进行合并，有三种情况停止合并
	while (1) {
		auto iter = _idSpanMap.find(span->_pageId + 1);
		if (iter == _idSpanMap.end())
			break;
		Span* preSpan = iter->second;
		if (preSpan->_isUse == true)
			break;
		if (preSpan->_n + span->_n > NPAGES)
			break;
		//完成合并
		span->_n += preSpan->_n;
		span->_pageId = preSpan->_pageId;

		_idSpanMap.erase(iter);
		_spanLists[preSpan->_n].Erase(preSpan);
		delete preSpan;
	}

	while (1) {
		auto iter = _idSpanMap.find(span->_pageId + span->_n  - 1);
		if (iter == _idSpanMap.end())
			break;
		Span* nextSpan = iter->second;
		if (nextSpan->_isUse == true)
			break;
		if (nextSpan->_n + span->_n > NPAGES - 1)
			break;
		//完成合并
		span->_n += nextSpan->_n;

		_idSpanMap.erase(iter);
		_spanLists[nextSpan->_n].Erase(nextSpan);
		delete nextSpan;
	}


	_spanLists[span->_n].PushFront(span);
	span->_isUse = false;
	_idSpanMap[span->_pageId] = span;
	_idSpanMap[span->_pageId + span->_n - 1] = span;

}

Span* PageCache::MapObjToSpan(void* obj)
{
	assert(obj != nullptr);
	PAGE_ID pageId = (PAGE_ID)obj >> PAGE_SHIFT;
	auto itr = _idSpanMap.find(pageId);
	if (itr == _idSpanMap.end()) {
		assert(false);
		return nullptr;
	}
	return itr->second;
}
