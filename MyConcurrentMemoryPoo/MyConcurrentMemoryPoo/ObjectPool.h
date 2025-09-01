#pragma once
#include "Common.h"
template<class T>
class ObjectPool
{
public:
	T* New() {
		T* obj = nullptr;

		if (_freeList) {//���������������Ѿ����ٵ�����ռ���_freeList��ȡ
			void* next = *(void**)_freeList;
			obj = (T*)_freeList;
			_freeList = next;
		}
		else {//��_memory��ȡ
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

		//��λNew��֮����Ҫ��ʾ������������
		new(obj)T;

		return obj;
	}

	void Delete(T* obj) {
		//��ʾ������������
		obj->~T();

		//ͷ������������
		*(void**)obj = _freeList;
		_freeList = (void*)obj;
	}
	

private:
	char* _memory = nullptr;//���ٵĿռ�
	size_t _remainBytes = 0;//_memory�ռ�Ĵ�С
	void* _freeList = nullptr;//���濪�ٵ�����ռ����������
};

//-------����ObjectPool������--------//
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
	//��¼ObjectPool������
	vector<TreeNode*>v1;
	ObjectPool<TreeNode>objPool;
	v1.reserve(N);
	size_t begin1 = clock();
	for (int i = 0; i < Round; i++) {
		//����
		for (int j = 0; j < N; j++) {
			v1.push_back(objPool.New());
		}
		//ɾ��
		for (int j = 0; j < N; j++) {
			objPool.Delete(v1[j]);
		}
		v1.clear();
	}
	size_t end1 = clock();
	//��¼new������
	vector<TreeNode*>v2;
	v2.reserve(N);
	size_t begin2 = clock();
	for (int i = 0; i < Round; i++) {
		//����
		for (int j = 0; j < N; j++) {
			v2.push_back(new TreeNode);
		}
		//ɾ��
		for (int j = 0; j < N; j++) {
			delete v2[j];
		}
		v2.clear();
	}
	size_t end2 = clock();
	cout << "ObjectPool cost time:" << end1 - begin1<<endl;
	cout << "new cost time:" << end2 - begin2<<endl;
}