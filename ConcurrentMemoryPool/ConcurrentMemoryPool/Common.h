#pragma once
#include <iostream>
#include <vector>
#include <mutex>
#include <assert.h>
#include <thread>
#include <algorithm>

using std::cout;
using std::endl;
#ifdef _WIN32
	#include <windows.h>
	#include <memoryapi.h>
#else
//...
#endif // _WIN32
static const size_t MAX_BYTES = 256 * 1024;
static const size_t FREElIST_NUM = 208;
static const size_t PAGE_NUM = 129;
static const size_t PAGE_SHIFT = 13;
#ifdef _WIN64
	typedef unsigned long long  PAGE_ID;
#elif _WIN32
	typedef size_t PAGE_ID;
#else
	//...linux
#endif




// ֱ��ȥ���ϰ�ҳ����ռ�
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// linux��brk mmap��
#endif

	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}


static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

class FreeList//�����зֺõ�С�������������
{
public:
	void Push(void* obj)//ͷ��
	{
		assert(obj);
		NextObj(obj) = _freeList;
		_freeList = obj;
		_size++;
	}
	void PushRange(void* start, void* end,size_t n)
	{
		NextObj(end) = _freeList;
		_freeList = start;
		_size += n;
	}

	void PopRange(void*& start, void*& end, size_t n) {
		assert(n >= _size);
		_size -= n;
		start = _freeList;
		end = start;
		for (size_t i = 0; i < n - 1; ++i) {
			end = NextObj(end);
		}
		_freeList = NextObj(end);
		NextObj(end) = nullptr;
	}

	size_t Size() { return _size; }

	void* Pop()//ͷɾ
	{
		assert(_freeList);
		void* obj = _freeList;
		_freeList = NextObj(obj);
		_size--;
		return obj;
	}
	bool Empty()
	{
		return _freeList == nullptr;
	}
	size_t& MaxSize() { return _maxSize; }
private:
	size_t _maxSize = 1;
	void* _freeList = nullptr;
	size_t _size;
};

//����ThreadCache��_freeLists����
class SizeClass
{
public:
// ������������10%���ҵ�����Ƭ�˷�
// [1,128]					8byte����	    freelist[0,16)
// [128+1,1024]				16byte����	    freelist[16,72)
// [1024+1,8*1024]			128byte����	    freelist[72,128)
// [8*1024+1,64*1024]		1024byte����     freelist[128,184)
// [64*1024+1,256*1024]		8*1024byte����   freelist[184,208)

	static inline size_t _RoundUp(size_t size, size_t alignNum) 
	{
		return ((size + alignNum - 1) & ~(alignNum - 1));
	}
	static inline size_t RoundUp(size_t size) 
		//��ȡ�ض������׼��ʵ�ʷ��ص��ڴ��С
	{
		if (size <= 128) {return _RoundUp(size, 8); }
		else if (size <= 1024) { return _RoundUp(size, 16);}
		else if (size <= 8 * 1024) { return _RoundUp(size, 128);}
		else if (size <= 64 * 1024) { return _RoundUp(size, 1024);}
		else if (size <= 256 * 1024) { return _RoundUp(size, 8 * 1024);}
		else{assert(false);return -1;}
	}
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
	// һ��thread cache��central cache��ȡ���ٸ�
	static size_t NumMoveSize(size_t size)
	{
		assert(size > 0);

		// [2, 512]��һ�������ƶ����ٸ������(������)����ֵ
		// С����һ���������޸�
		// С����һ���������޵�
		int num = MAX_BYTES / size;
		if (num < 2)
			num = 2;

		if (num > 512)
			num = 512;

		return num;
	}
	//һ��central cache��page cache��ö��ٸ�
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num * size;

		npage >>= PAGE_SHIFT;
		if (npage == 0)
			npage = 1;

		return npage;
	}
};

/*-------------SPAN-----------------*/
struct Span
{
	PAGE_ID _pageid = 0;        //����ڴ���ʼҳ��ҳ��
	size_t _n = 0;              //ҳ������
	Span* _next = nullptr;		//˫������Ľṹ
	Span* _prev = nullptr;      					  
	size_t _useCount = 0;		//�к�С���ڴ棬�������thread cache�ļ���
	void* _freeList = nullptr;	//�кõ�С���ڴ����������
};

class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}
	void Insert(Span* pos, Span* newSpan) 
	{
		assert(pos);
		assert(newSpan);
		newSpan->_prev = pos;
		newSpan->_next = pos->_next;
		pos->_next->_prev = newSpan;
		pos->_next = newSpan;
	}
	void Erase(Span* pos)
	{
		assert(pos);
		assert(pos != _head);

		Span* next = pos->_next;
		next->_prev = pos->_prev;
		pos->_prev->_next = next;


	}
	void PushFront(Span* newSpan) { Insert(_head->_next, newSpan); }
	Span* PopFront() 
	{
		Span* span = _head->_next;
		Erase(Begin());
		return span;
	}
	Span* Begin() { return _head->_next; }
	Span* End() { return _head; }
	bool Empty() { return _head->_next == _head; }
	std::mutex _mtx;//Ͱ��

private:
	Span* _head;
};