#pragma once
#include <iostream>
using std::cout;
using std::endl;

template<class T>
class ObjectPool
{
public:
	T* New() {
		T* obj = nullptr;

		if (_freeList) {//自由链表中有在已经开辟的闲余空间在_freeList中取
			void* next = *(void**)_freeList;
			obj = _freeList;
			_freeList = next;
		}
		else {//在_memory中取
			if (_remainBytes < sizeof(T)) {
				_remainBytes = 128 * 1024;
				_memory = (char*)malloc(_remainBytes);
				if (!_memory) {
					throw std::bad_alloc();
				}
			}
			obj = (T*)_memory;
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_remainBytes -= objSize;
			_memory += objSize;
		}

		//定位New，之后需要显示调用析构函数
		new(obj)T;

		return obj;
	}

	void Delete(T* obj) {
		//显示调用析构函数
		obj->~T();

		//头插入自由链表
		*(**void)obj = _freeList;
		_freeList = (void*)obj;
	}
	

private:
	char* _memory = nullptr;//开辟的空间
	size_t _remainBytes = 0//_memory空间的大小
	void* _freeList = nullptr;//储存开辟的闲余空间的自由链表
};
