#include "PageCache.h"

PageCache PageCache::_sInst;

//获取一个k页的span 
Span* PageCache::Newspan(size_t k)
{
	assert(k > 0);

	//大于128page 的直接向堆申请
	if (k > NPAGES - 1)
	{
		void* ptr = SystemAlloc(k);
		//Span* span = new Span;
		Span* span = _spanpool.New();

		span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
		span->_n = k;

		//_idSpanMap[span->_pageId] = span;
		_idSpanMap.set(span->_pageId, span);
		return span;
 	}

	//先检查第k个桶里面有没有span
	if (!_spanLists[k].Empty())
	{
		Span* kspan = _spanLists[k].PopFront();

		//建立id和span的映射，方便central cache回收小块内存时，查找对应的span
		for (PAGE_ID i = 0; i < kspan->_n; ++i)
		{
			//_idSpanMap[kspan->_pageId + i] = kspan;
			_idSpanMap.set(kspan->_pageId + i, kspan);
		}

		return kspan;
	}

	//检查一下后面的桶里有没有span ,如果有可以把他进行切分
	for (size_t i = k + 1; i < NPAGES; ++i)
	{
		if (!_spanLists[i].Empty())
		{
			Span* nspan = _spanLists[i].PopFront();
			//Span* kspan = new Span;
			Span* kspan = _spanpool.New();

			//在nspande 头部切一个k页下来
			//k页的span返回
			//nspan再挂到对应映射的位置
			kspan->_pageId = nspan->_pageId;
			kspan->_n = k;

			nspan->_pageId += k;
			nspan->_n -= k;

			_spanLists[nspan->_n].PushFront(nspan);
			//存储nspan的首位页号跟span映射， 方便page cache 回收内存时
			//进行合并查找
			//_idSpanMap[nspan->_pageId] = nspan;
			//_idSpanMap[nspan->_pageId + nspan->_n - 1] = nspan;
			_idSpanMap.set(nspan->_pageId, nspan);
			_idSpanMap.set(nspan->_pageId + nspan->_n - 1, nspan);

			//建立id和span的映射，方便central cache回收小块内存时，查找对应的span
			for (PAGE_ID i = 0; i < kspan->_n; ++i)
			{
				//_idSpanMap[kspan->_pageId + i] = kspan;
				_idSpanMap.set(kspan->_pageId + i, kspan);
			}

			return kspan;
		}
	}

	//走到这个位置就说明没有大页的span了
	//这时就去找堆要一个128页的span
	//Span* bigSpan = new Span;
	Span* bigSpan = _spanpool.New();
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;

	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return Newspan(k); 
}

Span* PageCache::MapObjTospan(void* obj)
{
	PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHIFT);

	//std::unique_lock<std::mutex>lock(_pageMtx);

	//auto ret = _idSpanMap.find(id);
	//if (ret != _idSpanMap.end())
	//{
	//	return ret->second;
	//}
	//else
	//{
	//	assert(false);
	//	return nullptr;
	//}

	auto ret = (Span*)_idSpanMap.get(id);
	assert(ret != nullptr);
	return ret;
}

void PageCache::ReleaseSpanToPageCache(Span* span)
{
	if (span->_n > NPAGES - 1)
	{
		//大于128page 的直接还给堆
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanpool.Delete(span);

		return;
	}

	//对span前后的页，尝试进行合并,缓解内存碎片问题
	while (1)
	{
		PAGE_ID previd = span->_pageId - 1;
		//auto ret = _idSpanMap.find(previd);
		
		////前面的页号没有， 不合并了
		//if (ret == _idSpanMap.end())
		//{
		//	break;
		//}
		auto ret = (Span*)_idSpanMap.get(previd);
		if (ret == nullptr)
		{
			break;
		}

		//前面相邻页的span在使用，不合并了
		Span* prevSpan = ret;
		if (prevSpan->_isUse == true)
		{
			break;
		}

		//合并超过128页的span没办法管理，不合并了
		if (prevSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}

		span->_pageId = prevSpan->_pageId;
		span->_n += prevSpan->_n;

		_spanLists[prevSpan->_n].Erase(prevSpan);
		//delete prevspan;
		_spanpool.Delete(prevSpan);
	}

	//向后合并
	while (1)
	{
		PAGE_ID nextid = span->_pageId + span->_n;
	/*	auto ret = _idSpanMap.find(nextid);
		if (ret == _idSpanMap.end())
		{
			break;
		}*/
		auto ret = (Span*)_idSpanMap.get(nextid);
		if (ret == nullptr)
		{
			break;
		}


		Span* nextspan = ret;
		if (nextspan->_isUse == true)
		{
			break;
		}
		
		if (nextspan->_n + span->_n > NPAGES - 1)
		{
			break;
		}

		span->_n += nextspan->_n;

		_spanLists[nextspan->_n].Erase(nextspan);
		//delete nextspan;
		_spanpool.Delete(nextspan);
	}

	_spanLists[span->_n].PushFront(span);
	span->_isUse = false;
//	_idSpanMap[span->_pageId] = span;
//	_idSpanMap[span->_pageId + span->_n - 1] = span;
	_idSpanMap.set(span->_pageId, span);
	_idSpanMap.set(span->_pageId + span->_n - 1, span);
}