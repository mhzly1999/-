#pragma once

#include "Common.h"

class ThreadCache
{
public:
	//������ͷ��ڴ����
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);

	//�����Ļ����ȡ����
	void* FetchFromCentralCache(size_t index, size_t size);

	//�ͷŶ���ʱ���������ʱ�������ڴ�ص����Ļ���
	void ListTooLong(FreeList& list, size_t size);
private:
	FreeList _freeLists[NFREE_LIST];
};

// TLS thread local storage,��ȡ�߳�
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;