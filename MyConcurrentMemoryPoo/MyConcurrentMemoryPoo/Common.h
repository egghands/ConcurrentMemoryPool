#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include "Common.h"
#include <mutex>
#include <assert.h>
#include <thread>
using std::cout;
using std::endl;
using std::vector;


static const size_t MAX_BYTES = 256 * 1024;
static const size_t NFREELIST = 208;
static const size_t NPAGES = 129;
static const size_t PAGE_SHIFT = 13;

#ifdef  _WIN32
	#include <windows.h>
#else
	#include <sys/mman.h>//linux
#endif 


#ifdef _WIN64
	typedef unsigned long long PAGE_ID;
#elif _WIN32
	typedef size_t PAGE_ID;
#elif __x86_64__
	typedef unsigned long long PAGE_ID;
#elif __i386__
	typedef size_t PAGE_ID;
	
#endif // 0


struct Span;
// ֱ��ȥ���ϰ�ҳ����ռ�
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else// linux��brk mmap��
	void* ptr = mmap(NULL, kpage << PAGE_SHIFT, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}
//ֱ���ڶ��ϰ�ҳ�ͷſռ�
inline static void SystemFree(void* ptr, size_t length) {
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, length);//Linux
#endif // _WIN32

}



// ��������С�Ķ���ӳ�����
class SizeClass
{
public:
	// ������������10%���ҵ�����Ƭ�˷�
	// [1,128]					8byte����	    freelist[0,16)
	// [128+1,1024]				16byte����	    freelist[16,72)
	// [1024+1,8*1024]			128byte����	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte����     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte����   freelist[184,208)

	/*size_t _RoundUp(size_t size, size_t alignNum)
	{
		size_t alignSize;
		if (size % alignNum != 0)
		{
			alignSize = (size / alignNum + 1)*alignNum;
		}
		else
		{
			alignSize = size;
		}

		return alignSize;
	}*/
	// 1-8 
	static inline size_t _RoundUp(size_t bytes, size_t alignNum)
	{
		return ((bytes + alignNum - 1) & ~(alignNum - 1));
	}

	static inline size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			return _RoundUp(size, 1 << PAGE_SHIFT);
		}
	}

	/*size_t _Index(size_t bytes, size_t alignNum)
	{
	if (bytes % alignNum == 0)
	{
	return bytes / alignNum - 1;
	}
	else
	{
	return bytes / alignNum;
	}
	}*/

	// 1 + 7  8
	// 2      9
	// ...
	// 8      15

	// 9 + 7 16
	// 10
	// ...
	// 16    23
	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	// ����ӳ�����һ����������Ͱ
	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		// ÿ�������ж��ٸ���
		static int group_array[4] = { 16, 56, 56, 56 };
		if (bytes <= 128) {
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024) {
			return _Index(bytes - 128, 4) + group_array[0];
		}
		else if (bytes <= 8 * 1024) {
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (bytes <= 64 * 1024) {
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (bytes <= 256 * 1024) {
			return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else {
			assert(false);
		}

		return -1;
	}

	static inline size_t NumMoveSize(size_t size) {
		assert(size > 0);
		assert(size <= MAX_BYTES);
		size_t num = MAX_BYTES / size;
		//����num��[2,512]
		if (num < 2)num = 2;
		if (num > 512)num = 512;
		return num;
	}

	static inline size_t NumMovePage(size_t size) {
		size_t num = NumMoveSize(size);
		size_t npage = num * size;
		npage >>= PAGE_SHIFT;
		if (npage < 1)npage = 1;
		return npage;
	}

};


//��ȡ��һ���ڵ�
static void*& NextObj(void* obj) { return *(void**)obj; }

//��������
class FreeList
{
public:
	void Push(void* obj) {//ͷ��ڵ�
		assert(obj != nullptr);
		NextObj(obj) = _freeList;
		_freeList = obj;
		_size++;
	}

	void PushRange(void* start, void* end,size_t n) {
		assert(start);
		assert(end);
		NextObj(end) = _freeList;
		_freeList = start;
		_size += n;
	}

	void PopRange(void*& start, void*& end, size_t n) {
		assert(n <= _size);
		start = _freeList;
		end = start;
		for (size_t i = 0; i < n - 1; i++) {
			end = NextObj(end);
		}
		_freeList = NextObj(end);
		NextObj(end) = nullptr;
		_size -= n;
	}

	void* Pop() {//ͷɾ�ڵ㲢����
		assert(_freeList != nullptr);
		void* ret = _freeList;
		_freeList = NextObj(_freeList);
		_size--;
		return ret;
	}

	bool Empty() { return _freeList == nullptr; }

	const size_t Size() { return _size; }

	size_t& MaxSize() { return _maxSize; }

private:
	void* _freeList = nullptr;
	size_t _maxSize = 1;
	size_t _size = 0;
};



struct Span
{
	PAGE_ID _pageId = 0;				//��ҳ�ڴ���ʼҳ��
	size_t _n = 0;						//ҳ������

	Span* _next = nullptr;			//˫��ѭ��
	Span* _prev = nullptr;			//��ͷ����

	size_t _objSize = 0;			//�кõ�С����Ĵ�С
	size_t _useCount = 0; 			//���ü���
	void* _freeList = nullptr;		//�к�С���ڴ����������
	bool _isUse = false;			//�ж��Ƿ�����ʹ��
};

class SpanList
{
public:
	SpanList() {	
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	void Insert(Span* pos, Span* node) {
		assert(node);
		assert(pos);
		node->_prev = pos->_prev;
		pos->_prev->_next = node;
		node->_next = pos;
		pos->_prev = node;
		
	}

	void PushFront(Span* span) {
		Insert(Begin(), span);
	}

	Span* PopFront() {
		Span* front = _head->_next;
		Erase(front);
		return front;
	}

	bool Empty() { return _head->_next == _head; }

	void Erase(Span* pos) {
		assert(pos);
		assert(pos != _head);
		pos->_prev->_next = pos->_next;
		pos->_next->_prev = pos->_prev;
	}

	Span* Begin() { return _head->_next; }
	Span* End() { return _head; }

	std::mutex& Mtx() { return _mtx; }//����Ͱ��

private:
	Span* _head;		//˫��ѭ����ͷ����
	std::mutex _mtx;			//Ͱ��
};