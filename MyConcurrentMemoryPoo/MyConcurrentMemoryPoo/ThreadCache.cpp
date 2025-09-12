#include "ThreadCache.h"

//-----�����߼�----//
void* ThreadCache::Allocate(size_t size)
{
		size_t alignSize = SizeClass::RoundUp(size);
		size_t index = SizeClass::Index(size);
		if (_freeLists[index].Empty()) {
			return FetchFromCentralCache(index, alignSize);
		}
		else {
			return _freeLists[index].Pop();
		}
	}


void* ThreadCache::FetchFromCentralCache(size_t index, size_t alignSize)
{
	//�������㷨�������������ĸ���
	#ifdef _WIN32 
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(alignSize));
	#elif __linux__
	size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(alignSize));
	#endif
	if (batchNum == _freeLists[index].MaxSize())
		_freeLists[index].MaxSize() += 1;
	void* start = nullptr;
	void* end = nullptr;

	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, alignSize);

	assert(actualNum >= 1);
	if (actualNum == 1)
		assert(start == end);
	else
		_freeLists[index].PushRange(NextObj(start), end,actualNum - 1);

	return start;
}


//-----�ͷ��߼�----//
void ThreadCache::Deallcate(void* ptr, size_t size)
{
		assert(size <= MAX_BYTES);
		assert(ptr);
		//�ҳ���Ӧ��ͰȻ��ͷ������������
		size_t index = SizeClass::Index(size);
		_freeLists[index].Push(ptr);
		if (_freeLists[index].Size() >= _freeLists[index].MaxSize()) {
			ListTooLong(_freeLists[index], size);
		}
	
}

void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr, * end = nullptr;
	list.PopRange(start, end, list.MaxSize());
	CentralCache::GetInstance()->ReleaseListToSpans(start, size);
}