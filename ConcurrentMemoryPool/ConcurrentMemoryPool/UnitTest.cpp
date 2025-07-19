// ConcurrentMemoryPool.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "ObjectPool.h"
#include "ObjectPool.h"
#include "ConcurrentAlloc.h"

void Alloc1()
{
	for (size_t i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(6);
	}
}

void Alloc2()
{
	for (size_t i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(7);
	}
}


void TLSTest()
{
	std::thread t1(Alloc1);
	std::thread t2(Alloc2);
	t1.join();
	t2.join();
}

int main()
{
	//TestObjectPool();
	TLSTest();

	return 0;
}
//int main()
//{
    //std::cout << "Hello World!\n";
    /*TestObjectPool()*/;
//}
