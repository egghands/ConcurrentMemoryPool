#include "ThreadCache.h"
#include "CentralCache.h"
void* ThreadCache::Allocate(size_t size)
{
	assert(size < 0);
	assert(size > MAX_BYTES);
	size_t index = SizeClass::Index(size);
	size_t alignSize = SizeClass::RoundUp(size);
	void* ret = nullptr;
	if (_freeLists[index].Empty()) {
		ret = FetchFromCentralCache(index,alignSize);
	}
	else {
		ret = _freeLists[index].Pop();
	}
	return ret;
}

//�������㷨���������Ĵ�С������������С���������Ĵ�СС��֮
void* ThreadCache::FetchFromCentralCache(size_t index,size_t size)
{
	size_t batchnum = min(_freeLists->MaxSize(), SizeClass::NumMoveSize(size));
	if (batchnum == _freeLists->MaxSize())_freeLists->MaxSize()++;

	void* start = nullptr, * end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchnum, size);
	
	assert(actualNum > 0);
	if (actualNum == 1) {
		assert(start == end);
		return start;
	}
	_freeLists[index].PushRange(NextObj(start), end, actualNum - 1);
	return start;
}
