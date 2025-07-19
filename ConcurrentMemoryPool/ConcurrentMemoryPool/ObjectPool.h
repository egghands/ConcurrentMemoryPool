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
	// �����ͷŵ��ִ�
	const size_t Rounds = 5;

	// ÿ�������ͷŶ��ٴ�
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