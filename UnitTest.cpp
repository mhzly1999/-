//#include "ObjectPool.h"
#include "ConcurrentAlloc.h"
//
////void Alloc1()
////{
////	for (size_t i = 0; i < 5; ++i)
////	{
////		void* ptr = ConcurrentAlloc(6);
////	}
////}
////
////void Alloc2()
////{
////	for (size_t i = 0; i < 5; i++)
////	{
////		void* ptr = ConcurrentAlloc(7);
////	}
////}
////
////void TLSTest()
////{
////	std::thread t1(Alloc1);
////	std::thread t2(Alloc2);
////
////	t1.join();
////	t2.join();
////}
//
////void TestConcurrentAlloc()
////{
////	void* p1 = ConcurrentAlloc(6);
////	void* p2 = ConcurrentAlloc(8);
////	void* p3 = ConcurrentAlloc(1);
////	void* p4 = ConcurrentAlloc(7);
////	void* p5 = ConcurrentAlloc(8);
////
////	cout << p1 << endl;
////	cout << p2<< endl;
////	cout << p3 << endl;
////	cout << p4 << endl;
////	cout << p5 << endl;
////
////	//ConcurrentFree(p1,6);
////	//ConcurrentFree(p2, 8);
////	//ConcurrentFree(p3, 1);
////	//ConcurrentFree(p4, 7);
////	//ConcurrentFree(p5, 8);
////}
////
void TestConcurrentAlloc2()
{
	for (size_t i = 0; i < 5; ++i)
	{
		void* p1 = ConcurrentAlloc(6);
		cout << p1 << endl;
	}

	for (size_t i = 0; i < 5; ++i)
	{
		void* p2 = ConcurrentAlloc(8);
		cout << p2 << endl;
	}
}
//
//void TestAddressShift()
//{
//	PAGE_ID id1 = 2000;
//	PAGE_ID id2 = 2001;
//	char* p1 = (char*)(id1 << PAGE_SHIFT);
//	char* p2 = (char*)(id2 << PAGE_SHIFT);
//	while (p1 < p2)
//	{
//		cout << (void*)p1 << ":" << ((PAGE_ID)p1 >> PAGE_SHIFT) << endl;
//		p1 += 8;
//	}
//}
//
//void bigAlloc()
//{
//	void* p1 = ConcurrentAlloc(257 * 1024);
//	ConcurrentFree(p1);
//
//	void* p2 = ConcurrentAlloc(129 * 8 * 1024);
//	{
//		ConcurrentFree(p2);
//	}
//}
//
//int main()
//{
//	TLSTest();
//	TestConcurrentAlloc2();
//	bigAlloc();
//
//	return 0;
//}
