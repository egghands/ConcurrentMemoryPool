#include "ObjectPool.h"
#include "ConcurrentAlloc.h"
#include <thread>
#include <vector>

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
	{
	}
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
	cout << "ObjectPool cost time:" << end1 - begin1 << endl;
	cout << "new cost time:" << end2 - begin2 << endl;
}
void Alloc1(size_t size) {
	std::vector<void*>v;
	for (size_t i = 0; i < 2; i++) {
		void* ptr = ConcurrentAlloc(size);
		cout << std::this_thread::get_id() << ":" << ptr << endl;
		v.push_back(ptr);
	}
	for (size_t i = 0; i < 2; i++) {
		ConcurrentFree(v[i]);
	}
}

void Alloc2(size_t size) {
	std::vector<void*>v;
	for (size_t i = 0; i < 5; i++) {
		void* ptr = ConcurrentAlloc(size);
		cout << std::this_thread::get_id() << ":" << ptr << endl;
		v.push_back(ptr);
	}
	for (size_t i = 0; i < 5; i++) {
		ConcurrentFree(v[i]);
	}
	void* p1 = ConcurrentAlloc(259);
	cout << std::this_thread::get_id() << ":" << p1 << endl;
												 
	void* p2 = ConcurrentAlloc(256);			 
	cout << std::this_thread::get_id() << ":" << p2 << endl;

	void* p3 = ConcurrentAlloc(256);			 
	cout << std::this_thread::get_id() << ":" << p3 << endl;
								
	void* p4 = ConcurrentAlloc(256);			 
	cout << std::this_thread::get_id() << ":" << p4 << endl;

	void* p5 = ConcurrentAlloc(256);
	cout << std::this_thread::get_id() << ":" << p5 << endl;
	
	ConcurrentFree(p1);
	ConcurrentFree(p2);
	ConcurrentFree(p3);
	ConcurrentFree(p4);
	ConcurrentFree(p5);
}

void Alloc3(size_t size) {
	std::vector<void*>v;
	for (size_t i = 0; i < 2; i++) {
		void* ptr = ConcurrentAlloc(size);
		cout << std::this_thread::get_id() << ":" << ptr << endl;
		v.push_back(ptr);
	}
	for (size_t i = 0; i < 2; i++) {
		ConcurrentFree(v[i]);
	}
}

void TestConAlloc() {
	std::thread t1(Alloc1,2);
	std::thread t2(Alloc2, 9);
	std::thread t3(Alloc3, 1);
	std::thread t4(Alloc2, 17);
	std::thread t5(Alloc2,13);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();

}

void TestAlloc() {
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(8);
	void* p3 = ConcurrentAlloc(1);
	void* p4 = ConcurrentAlloc(7);
	void* p5 = ConcurrentAlloc(8);

	cout << p1 << endl;
	cout << p2 << endl;
	cout << p3 << endl;
	cout << p4 << endl;
	cout << p5 << endl;

	ConcurrentFree(p1);
	ConcurrentFree(p2);
	ConcurrentFree(p3);
	ConcurrentFree(p4);
	ConcurrentFree(p5);
}

void TestAlloc1() {
	for (size_t i = 0; i < 1024; i++) {
		void* ptr = ConcurrentAlloc(6);
		cout << ptr << endl;
	}
	void* p = ConcurrentAlloc(6);
	cout << p << endl;
}


void TestDealloc(size_t n,size_t size) {
	vector<void*>v(n);
	for (size_t i = 0; i < n; i++)
	{
		void* p = ConcurrentAlloc(size);
		v[i] = p;
	}

	for (size_t i = 0; i < n; i++) {
		ConcurrentFree(v[i]);
	}
}

void BigAlloc() {
	void* p1 = ConcurrentAlloc(257 * 1024);
	ConcurrentFree(p1);

	void* p2 = ConcurrentAlloc(129 * 1024 * 8);
	ConcurrentFree(p2);
}

//int main() {
//	//TestObjectPool();
//	//TestConAlloc();
//	//TestAlloc();
//	//TestAlloc1();
//	//Alloc1(6);
//	//TestDealloc(2,6);
//	BigAlloc();
//}