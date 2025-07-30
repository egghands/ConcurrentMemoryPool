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

//慢调节算法，如果对象的大小大，怎返回数量小，如果对象的大小小反之
void* ThreadCache::FetchFromCentralCache(size_t index,size_t size)
{
	size_t batchnum = min(_freeLists->MaxSize(), SizeClass::NumMoveSize(size));
	if (batchnum == _freeLists->MaxSize())_freeLists->MaxSize()++;
	size_t actualNum = 
}
