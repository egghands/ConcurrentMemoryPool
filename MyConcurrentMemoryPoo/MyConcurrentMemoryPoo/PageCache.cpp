#include "PageCache.h"
#include "CentralCache.h"
//����ģʽ��������
PageCache PageCache::_sInst;

//-----�����߼�----///

Span* PageCache::NewSpan(size_t npage)
{
	//����256kb�Ŀռ�
	if (npage >= NPAGES) {
		void* p = SystemAlloc(npage);
		//Span* bigSpan = new Span;
		Span* bigSpan = _spanPool.New();
		bigSpan->_n = npage;
		bigSpan->_pageId = (PAGE_ID)p >> PAGE_SHIFT;	
		_idSpanMap[bigSpan->_pageId] = bigSpan;
		return bigSpan;
	}

	assert(npage > 0 && npage < NPAGES);
	SpanList& spanList = _spanLists[npage];
	if (!spanList.Empty())
	{
		Span* span =  spanList.PopFront();
		for (int i = 0; i < span->_n; i++)
			_idSpanMap[span->_pageId + i] = span;
		return span;
	}
	//��ǰspanlist��û�ж�Ӧ��С��span��������listѰ�ң�Ȼ���и�
	for (size_t i = npage + 1; i < NPAGES; i++) {
		if (!_spanLists[i].Empty())
		{
			Span* aSpan = _spanLists[i].PopFront();
			//Span* bSpan = new Span;
			Span* bSpan = _spanPool.New();
			
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
	//_spanList��û�д���ڴ�,��Ҫ��ϵͳ����
	//Span* bigSpan = new Span;
	Span* bigSpan = _spanPool.New();
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_n = NPAGES - 1;
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	_spanLists[bigSpan->_n].PushFront(bigSpan);
	return NewSpan(npage);
}


//------�ͷ��߼�------//
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	if (span->_n >= NPAGES) {
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		 _spanPool.Delete(span);	
	}
	else{

		//��ǰ������span���Խ��кϲ������������ֹͣ�ϲ�
		while (1) {
			auto iter = _idSpanMap.find(span->_pageId + 1);
			if (iter == _idSpanMap.end())
				break;
			Span* preSpan = iter->second;
			if (preSpan->_isUse == true)
				break;
			if (preSpan->_n + span->_n > NPAGES)
				break;
			//��ɺϲ�
			span->_n += preSpan->_n;
			span->_pageId = preSpan->_pageId;

			_idSpanMap.erase(iter);
			_spanLists[preSpan->_n].Erase(preSpan);
			//delete preSpan;
			_spanPool.Delete(preSpan);
		}

		while (1) {
			auto iter = _idSpanMap.find(span->_pageId + span->_n - 1);
			if (iter == _idSpanMap.end())
				break;
			Span* nextSpan = iter->second;
			if (nextSpan->_isUse == true)
				break;
			if (nextSpan->_n + span->_n > NPAGES - 1)
				break;
			//��ɺϲ�
			span->_n += nextSpan->_n;

			_idSpanMap.erase(iter);
			_spanLists[nextSpan->_n].Erase(nextSpan);
			//delete nextSpan;
			_spanPool.Delete(nextSpan);
		}


		_spanLists[span->_n].PushFront(span);
		span->_isUse = false;
		_idSpanMap[span->_pageId] = span;
		_idSpanMap[span->_pageId + span->_n - 1] = span;
	}

}

Span* PageCache::MapObjToSpan(void* obj)
{
	assert(obj != nullptr);
	std::unique_lock<std::mutex>lock(_mtx);
	PAGE_ID pageId = (PAGE_ID)obj >> PAGE_SHIFT;
	auto itr = _idSpanMap.find(pageId);
	if (itr == _idSpanMap.end()) {
		assert(false);
		return nullptr;
	}
	return itr->second;
}
