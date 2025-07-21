#pragma once
#include "Common.h"

template<class T>
class ObjectPool
{
public:
	T* New()
	{
		T* obj = nullptr;
		if (_freeList) {//����й��ص��ڴ�����ʹ��
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

	void Delete(T* obj) {//ͷ�嵽_freeList��
		obj->~T();//��λnew��Ҫ��ʾ�ĵ�����������
		*(void**)obj = _freeList;//ָ��Ĵ�С�ڲ�ͬ�Ĳ���ϵͳ���Ǳ仯��
		_freeList = obj;
	}

private:
	void* _freeList = nullptr;//���ر��������ڴ�����
	size_t remainBytes = 0;//ʣ�µ��ڴ�
	char* _memory = nullptr;//������ڴ�
};

