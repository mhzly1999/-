#pragma once
#include <iostream>
using std::cout;
using std::endl;

//定长内存池
//template <size_t N>
//class ObjectPool
//{};


template<class T>
class ObjectPool
{
public:
	T* new()
	{
		T * obj = nullptr;
	    //优先使用还回来的内存对象
		if (_freeList)
		{
			void *next = *(void**)_freeList;
			obj = _freeList;
			_freeList = next;
			return obj;
		}
		else
		{
			//剩余内存不够一个对象大小时，则重新开大块内存空间
			if (_remainBytes < sizeof(T))
			{
				_remainBytes = 128 * 1024;
				_memory = (char*)malloc(_remainBytes);
				if (_memory == nullptr)
				{
					throw bad_alloc();
				}
			}

			obj = (T*)_memory;
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objSize;
			_remainBytes -= objSize;
		}
		//定位new,显示调用T的构造函数初始化
		new(obj)T;
		return obj;
	}
		void delete(T *obj)
	{
		if (_freeList == nullptr)
		{
			//显示调用析构函数清理对象
			obj-> ~T();
			//头插
			*(void**)obj = _freeList;
			_freeList = obj;
		}
	}

private:
	char * _memory = nullptr;//每次可以只向内存申请一个字节的空间,指向大块内存的指针
	size_t _remainBytes = 0;//大块内存在切分过程中剩余的字节数
	void * _freeList = nullptr;//返回来链接的自由链表的头指针
};

