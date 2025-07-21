#include "Common.h"
#include"ThreadCache.h"
#include "CentralCache.h"
void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	size_t index = SizeClass::Index(size);
	size_t alignSize = SizeClass::RoundUp(size);
	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	//cout << "my id:" << std::this_thread::get_id()<<endl;
	return FetchFromCentralCache(index, alignSize);
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr && size <MAX_BYTES);
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
	//当链表的长度大于一次申请时开始回收
	if (_freeLists[index].Size() >= _freeLists[index].MaxSize())
		ListTooLong(_freeLists[index], size);
}


void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr;
	void* end = nullptr;
	list.PopRange(start, end, list.MaxSize());
	CentralCache::GetInstance()->ReleaseListToSpans(start, size);
}
void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// 慢开始反馈调节算法
	// 1、最开始不会一次向central cache一次批量要太多，因为要太多了可能用不完
	// 2、如果你不要这个size大小内存需求，那么batchNum就会不断增长，直到上限
	// 3、size越大，一次向central cache要的batchNum就越小
	// 4、size越小，一次向central cache要的batchNum就越大
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	if (_freeLists[index].MaxSize() == batchNum)
	{
		_freeLists[index].MaxSize() += 1;
	}

	void* start = nullptr;
	void* end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum > 0);

	if (actualNum == 1)
	{
		assert(start == end);
		return start;
	}
	else
	{
		_freeLists[index].PushRange(NextObj(start), end, actualNum - 1);
		return start;
	}
}
