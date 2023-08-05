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

	//获取对象到span的映射
	Span* MapObjTospan(void* obj);

	//释放空闲span回到Pagecache, 并合并相邻的span
	void ReleaseSpanToPageCache(Span* span);

	//获取一个K页的span
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