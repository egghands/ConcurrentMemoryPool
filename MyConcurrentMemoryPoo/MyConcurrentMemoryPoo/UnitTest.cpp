#include "ObjectPool.h"
#include "ConcurrentAlloc.h"
#include <thread>
#include <vector>
void Alloc1(size_t size) {
	std::vector<void*>v;
	for (size_t i = 0; i < 2; i++) {
		void* ptr = ConcurrentAlloc(size);
		cout << std::this_thread::get_id() << ":" << ptr << endl;
		v.push_back(ptr);
	}
	for (size_t i = 0; i < 2; i++) {
		ConcurrentFree(v[i], size);
	}
}

void Alloc2(size_t size) {
	std::vector<void*>v;
	for (size_t i = 0; i < 2; i++) {
		void* ptr = ConcurrentAlloc(size);
		cout << std::this_thread::get_id() << ":" << ptr << endl;
		v.push_back(ptr);
	}
	for (size_t i = 0; i < 2; i++) {
		ConcurrentFree(v[i], size);
	}
	void* p1 = ConcurrentAlloc(259);
	cout << std::this_thread::get_id() << ":" << p1 << endl;
												 
	void* p2 = ConcurrentAlloc(256);			 
	cout << std::this_thread::get_id() << ":" << p2 << endl;
	void* p3 = ConcurrentAlloc(256);			 
	cout << std::this_thread::get_id() << ":" << p3 << endl;
								
	ConcurrentFree(p1, 256);
	ConcurrentFree(p2, 256);
	ConcurrentFree(p2, 256);

	void* p4 = ConcurrentAlloc(256);			 
	cout << std::this_thread::get_id() << ":" << p4 << endl;

	void* p5 = ConcurrentAlloc(256);
	cout << std::this_thread::get_id() << ":" << p5 << endl;

}

void Alloc3(size_t size) {
	std::vector<void*>v;
	for (size_t i = 0; i < 2; i++) {
		void* ptr = ConcurrentAlloc(size);
		cout << std::this_thread::get_id() << ":" << ptr << endl;
		v.push_back(ptr);
	}
	for (size_t i = 0; i < 2; i++) {
		ConcurrentFree(v[i], size);
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

}

void TestAlloc1() {
	for (size_t i = 0; i < 1024; i++) {
		void* ptr = ConcurrentAlloc(6);
		cout << ptr << endl;
	}
	void* p = ConcurrentAlloc(6);
	cout << p << endl;
}


int main() {
	//TestObjectPool();
	TestConAlloc();
	TestAlloc();
	TestAlloc1();

}