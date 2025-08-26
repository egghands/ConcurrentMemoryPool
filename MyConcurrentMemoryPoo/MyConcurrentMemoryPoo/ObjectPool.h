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

		if (_freeList) {//���������������Ѿ����ٵ�����ռ���_freeList��ȡ
			void* next = *(void**)_freeList;
			obj = _freeList;
			_freeList = next;
		}
		else {//��_memory��ȡ
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

		//��λNew��֮����Ҫ��ʾ������������
		new(obj)T;

		return obj;
	}

	void Delete(T* obj) {
		//��ʾ������������
		obj->~T();

		//ͷ������������
		*(**void)obj = _freeList;
		_freeList = (void*)obj;
	}
	

private:
	char* _memory = nullptr;//���ٵĿռ�
	size_t _remainBytes = 0//_memory�ռ�Ĵ�С
	void* _freeList = nullptr;//���濪�ٵ�����ռ����������
};
