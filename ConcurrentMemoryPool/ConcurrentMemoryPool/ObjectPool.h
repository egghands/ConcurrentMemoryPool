#pragma once
#include "Common.h"

template<class T>
class ObjectPool
{
public:
	T* New()
	{
		T* obj = nullptr;
		if (_freeList) {//如果有挂载的内存优先使用
			void* next = *(void**)_freeList;
			obj = (T*)_freeList;
			_freeList = next;
		}
		else {
			if (remainBytes < sizeof(T)) {
				remainBytes = 1024 * 128;
				_memory = (char*)malloc(remainBytes);
				if (!_memory)
					throw std::bad_alloc();
			}
			size_t outSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			obj = (T*)_memory;
			_memory += outSize;
			remainBytes -= outSize;

		}
		new(obj)T;
		return obj;
	}

	void Delete(T* obj) {//头插到_freeList中
		obj->~T();//定位new需要显示的调用析构函数
		*(void**)obj = _freeList;//指针的大小在不同的操作系统中是变化的
		_freeList = obj;
	}

private:
	void* _freeList = nullptr;//挂载被析构的内存链表
	size_t remainBytes = 0;//剩下的内存
	char* _memory = nullptr;//申请的内存
};

