#pragma once
#include"Common.h"

//�����ڴ��
//template <size_t N>
//class ObjectPool
//{};

template<class T>
class ObjectPool
{
public:
	T* New()
	{
		T * obj = nullptr;

	    //����ʹ�û��������ڴ����
		if (_freeList)
		{
			void* next = *(void**)_freeList;
			obj = (T*)_freeList;
			_freeList = next;
		}
		else
		{
			//ʣ���ڴ治��һ�������Сʱ�������¿�����ڴ�ռ�
			if (_remainBytes < sizeof(T))
			{
				_remainBytes = 128 * 1024;
				//_memory = (char*)malloc(_remainBytes);
				_memory = (char*)SystemAlloc(_remainBytes >> 13);
				if (_memory == nullptr)
				{
					throw std::bad_alloc();
				}
			}

			obj = (T*)_memory;
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objSize;
			_remainBytes -= objSize;
		}

		//��λnew,��ʾ����T�Ĺ��캯����ʼ��
		new(obj)T;
		assert(obj);

		return obj;
	}

		void Delete(T* obj)
	{
		//��ʾ����T����������
		obj -> ~T();

		//ͷ��    �ó����������ڴ���ǰ�Ļ�˸��ֽڴ������������ĵ�ַ
		*(void**)obj = _freeList;
		_freeList = obj;
	}

private:
	char * _memory = nullptr;//ÿ�ο���ֻ���ڴ�����һ���ֽڵĿռ�,ָ�����ڴ��ָ��
	size_t _remainBytes = 0;//����ڴ����зֹ�����ʣ����ֽ���

	void * _freeList = nullptr;//���������ӵ�����������ͷָ��
};

//struct TreeNode
//{
//	int _val;
//	TreeNode* _left;
//	TreeNode* _right;
//
//	TreeNode()
//		:_val(0)
//		, _left(nullptr)
//		, _right(nullptr)
//	{}
//};
//
//
//void TestObjectPool()
//{
//	// �����ͷŵ��ִ�
//	const size_t Rounds = 5;
//
//	// ÿ�������ͷŶ��ٴ�
//	const size_t N = 100000;
//
//	std::vector<TreeNode*> v1;
//	v1.reserve(N);
//
//	size_t begin1 = clock();
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v1.push_back(new TreeNode);
//		}
//		for (int i = 0; i < N; ++i)
//		{
//			delete v1[i];
//		}
//		v1.clear();
//	}
//
//	size_t end1 = clock();
//
//	std::vector<TreeNode*> v2;
//	v2.reserve(N);
//
//	ObjectPool<TreeNode> TNPool;
//	size_t begin2 = clock();
//	for (size_t j = 0; j < Rounds; ++j)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			v2.push_back(TNPool.New());
//		}
//		for (int i = 0; i < N; ++i)
//		{
//			TNPool.Delete(v2[i]);
//		}
//		v2.clear();
//	}
//	size_t end2 = clock();
//
//	cout << "new cost time:" << end1 - begin1 << endl;
//	cout << "object pool cost time:" << end2 - begin2 << endl;
//}
//