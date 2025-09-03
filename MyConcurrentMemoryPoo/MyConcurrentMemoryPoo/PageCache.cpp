#include "PageCache.h"
#include "CentralCache.h"
//����ģʽ��������
PageCache PageCache::_sInst;

//-----�����߼�----///

Span* PageCache::NewSpan(size_t npage)
{
	assert(npage > 0 && npage < NPAGES);
	SpanList& spanList = _spanLists[npage];
	if (!spanList.Empty())
		return spanList.PopFront();
	//��ǰspanlist��û�ж�Ӧ��С��span��������listѰ�ң�Ȼ���и�
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

			for (size_t i = 0; i < bSpan->_n; i++) {
				_idSpanMap[bSpan->_pageId + i] = bSpan;
			}

			return aSpan;
		}
	}
	//_spanList��û�д���ڴ�,��Ҫ��ϵͳ����
	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_n = NPAGES - 1;
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	_spanLists[bigSpan->_n].PushFront(bigSpan);
	return NewSpan(npage);
}


//------�ͷ��߼�------//
void PageCache::ReleaseSpanToPageCache(Span* span)
{

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
