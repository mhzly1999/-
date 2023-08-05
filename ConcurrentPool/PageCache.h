#pragma once

#include "Common.h"
#include "ObjectPool.h"
#include "PageMap.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	//��ȡ����span��ӳ��
	Span* MapObjTospan(void* obj);

	//�ͷſ���span�ص�Pagecache, ���ϲ����ڵ�span
	void ReleaseSpanToPageCache(Span* span);

	//��ȡһ��Kҳ��span
	Span* Newspan(size_t k);

	std::mutex _pageMtx;
private:
	SpanList _spanLists[NPAGES];
	ObjectPool<Span> _spanpool;

	//std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	//std::map<PAGE_ID, Span*> _idSpanMap;
	TCMalloc_PageMap1<32 - PAGE_SHIFT> _idSpanMap;

	PageCache()
	{}
	PageCache(const PageCache&) = delete;


	static PageCache _sInst;
};