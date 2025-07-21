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

	//ToDO...
}

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// ����ʼ���������㷨
	// 1���ʼ����һ����central cacheһ������Ҫ̫�࣬��ΪҪ̫���˿����ò���
	// 2������㲻Ҫ���size��С�ڴ�������ôbatchNum�ͻ᲻��������ֱ������
	// 3��sizeԽ��һ����central cacheҪ��batchNum��ԽС
	// 4��sizeԽС��һ����central cacheҪ��batchNum��Խ��
	size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	if (_freeLists[index].MaxSize() == batchNum)
	{
		_freeLists[index].MaxSize() += 1;
	}

	void* start = nullptr;
	void* end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum > 1);

	if (actualNum == 1)
	{
		assert(start == end);
		return start;
	}
	else
	{
		_freeLists[index].PushRange(NextObj(start), end);
		return start;
	}
}

