#include "CentralCache.h"
#include "PageCache.h"
#include "Common.h"
CentralCache CentralCache::_sInst;

Span* CentralCache::GetOneSpan(SpanList& list, size_t byte_size)
{
	//�鿴�Ƿ��п��е�span
	Span* it = list.Begin();
	while (it != list.End()) {
		if (it->_freeList)
			return it;
		else
			it = it->_next;
	}
	//��central cache��Ͱ��������������߳̿����ͷ��ڴ����
	list._mtx.unlock(); 
	//�����û�п��е�span
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(byte_size));
	PageCache::GetInstance()->_pageMtx.unlock();

	// ����span�Ĵ���ڴ����ʼ��ַ�ʹ���ڴ�Ĵ�С���ֽ�����
	char* start = (char*)(span->_pageid << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// �и�span�������ӵ�spanlists��
	span->_freeList = start;
	void* tail = start;
	start += byte_size;
	while (start < end) {
		NextObj(tail) = start;
		tail = start;
		start += byte_size;
	}
	list._mtx.lock();
	list.PushFront(span);
	return span;
}


// �����Ļ����ȡһ�������Ķ����thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();

	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// ��span�л�ȡbatchNum������
	// �������batchNum�����ж����ö���
	start = span->_freeList;
	end = start;
	size_t i = 0;
	size_t actualNum = 1;
	while (i < batchNum - 1 && NextObj(end) != nullptr)
	{
		end = NextObj(end);
		++i;
		++actualNum;
	}
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;
	span->_useCount += actualNum;
	_spanLists[index]._mtx.unlock();

	return actualNum;
}
