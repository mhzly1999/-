#pragma once
#include <iostream>
using std::cout;
using std::endl;

//�����ڴ��
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
	    //����ʹ�û��������ڴ����
		if (_freeList)
		{
			void *next = *(void**)_freeList;
			obj = _freeList;
			_freeList = next;
			return obj;
		}
		else
		{
			//ʣ���ڴ治��һ�������Сʱ�������¿�����ڴ�ռ�
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
		//��λnew,��ʾ����T�Ĺ��캯����ʼ��
		new(obj)T;
		return obj;
	}
		void delete(T *obj)
	{
		if (_freeList == nullptr)
		{
			//��ʾ�������������������
			obj-> ~T();
			//ͷ��
			*(void**)obj = _freeList;
			_freeList = obj;
		}
	}

private:
	char * _memory = nullptr;//ÿ�ο���ֻ���ڴ�����һ���ֽڵĿռ�,ָ�����ڴ��ָ��
	size_t _remainBytes = 0;//����ڴ����зֹ�����ʣ����ֽ���
	void * _freeList = nullptr;//���������ӵ����������ͷָ��
};

