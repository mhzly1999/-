#pragma once

#include"Common.h"

//����ģʽ
class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;
	}

	//��ȡһ���ǿյ�span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	//�����Ļ����ȡһ����Ŀ�Ķ����threadcache
	size_t FetchRangeObj(void*& start, void*& end,size_t batchNum, size_t byte_size);

	//��һ�������Ķ����ͷŵ�span���
	void ReleaseListToSpans(void* start, size_t byte_size);
private:
	SpanList _spanLists[NFREE_LIST];

private:
	CentralCache()
	{}

	CentralCache(const CentralCache&) = delete;

	static CentralCache _sInst;
};
