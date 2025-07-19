#pragma once
#include <iostream>
#include <vector>
using std::cout;
using std::endl;

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

struct TreeNode
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;

	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};

void TestObjectPool()
{
	// 申请释放的轮次
	const size_t Rounds = 5;

	// 每轮申请释放多少次
	const size_t N = 100000;

	std::vector<TreeNode*> v1;
	v1.reserve(N);

	size_t begin1 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}

	size_t end1 = clock();

	std::vector<TreeNode*> v2;
	v2.reserve(N);

	ObjectPool<TreeNode> TNPool;
	size_t begin2 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v2.push_back(TNPool.New());
		}
		for (int i = 0; i < N; ++i)
		{
			TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();

	cout << "new cost time:" << end1 - begin1 << endl;
	cout << "object pool cost time:" << end2 - begin2 << endl;
}