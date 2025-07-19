#include "Common.h"
#include"ThreadCache.h"

void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	size_t index = SizeClass::Index(size);
	size_t alignSize = SizeClass::RoundUp(size);
	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	cout << "my id:" << std::this_thread::get_id()<<endl;
	return FetchFromCentralCache(index, alignSize);
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr && size <MAX_BYTES);
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
}

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	return nullptr;
}

