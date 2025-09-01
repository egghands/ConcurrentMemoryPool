#pragma once
#include "Common.h"
template<class T>
class ObjectPool
{
public:
	T* New() {
		T* obj = nullptr;

		if (_freeList) {//自由链表中有在已经开辟的闲余空间在_freeList中取
			void* next = *(void**)_freeList;
			obj = (T*)_freeList;
			_freeList = next;
		}
		else {//在_memory中取
			if (_remainBytes < sizeof(T)) {
				_remainBytes = 128 * 1024;
				//_memory = (char*)malloc(_remainBytes);
				_memory = (char*)SystemAlloc(_remainBytes >> 13);
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
		*(void**)obj = _freeList;
		_freeList = (void*)obj;
	}
	

private:
	char* _memory = nullptr;//开辟的空间
	size_t _remainBytes = 0;//_memory空间的大小
	void* _freeList = nullptr;//储存开辟的闲余空间的自由链表
};

//-------测试ObjectPool的性能--------//
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

void TestObjectPool() {
	const size_t N = 100000;
	const size_t Round = 3;
	//记录ObjectPool的性能
	vector<TreeNode*>v1;
	ObjectPool<TreeNode>objPool;
	v1.reserve(N);
	size_t begin1 = clock();
	for (int i = 0; i < Round; i++) {
		//创建
		for (int j = 0; j < N; j++) {
			v1.push_back(objPool.New());
		}
		//删除
		for (int j = 0; j < N; j++) {
			objPool.Delete(v1[j]);
		}
		v1.clear();
	}
	size_t end1 = clock();
	//记录new的性能
	vector<TreeNode*>v2;
	v2.reserve(N);
	size_t begin2 = clock();
	for (int i = 0; i < Round; i++) {
		//创建
		for (int j = 0; j < N; j++) {
			v2.push_back(new TreeNode);
		}
		//删除
		for (int j = 0; j < N; j++) {
			delete v2[j];
		}
		v2.clear();
	}
	size_t end2 = clock();
	cout << "ObjectPool cost time:" << end1 - begin1<<endl;
	cout << "new cost time:" << end2 - begin2<<endl;
}