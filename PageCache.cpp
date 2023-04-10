#include "PageCache.h"

PageCache PageCache::_sInst;

//��ȡһ��kҳ��span 
Span* PageCache::Newspan(size_t k)
{
	assert(k > 0);

	//����128page ��ֱ���������
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

	//�ȼ���k��Ͱ������û��span
	if (!_spanLists[k].Empty())
	{
		Span* kspan = _spanLists[k].PopFront();

		//����id��span��ӳ�䣬����central cache����С���ڴ�ʱ�����Ҷ�Ӧ��span
		for (PAGE_ID i = 0; i < kspan->_n; ++i)
		{
			//_idSpanMap[kspan->_pageId + i] = kspan;
			_idSpanMap.set(kspan->_pageId + i, kspan);
		}

		return kspan;
	}

	//���һ�º����Ͱ����û��span ,����п��԰��������з�
	for (size_t i = k + 1; i < NPAGES; ++i)
	{
		if (!_spanLists[i].Empty())
		{
			Span* nspan = _spanLists[i].PopFront();
			//Span* kspan = new Span;
			Span* kspan = _spanpool.New();

			//��nspande ͷ����һ��kҳ����
			//kҳ��span����
			//nspan�ٹҵ���Ӧӳ���λ��
			kspan->_pageId = nspan->_pageId;
			kspan->_n = k;

			nspan->_pageId += k;
			nspan->_n -= k;

			_spanLists[nspan->_n].PushFront(nspan);
			//�洢nspan����λҳ�Ÿ�spanӳ�䣬 ����page cache �����ڴ�ʱ
			//���кϲ�����
			//_idSpanMap[nspan->_pageId] = nspan;
			//_idSpanMap[nspan->_pageId + nspan->_n - 1] = nspan;
			_idSpanMap.set(nspan->_pageId, nspan);
			_idSpanMap.set(nspan->_pageId + nspan->_n - 1, nspan);

			//����id��span��ӳ�䣬����central cache����С���ڴ�ʱ�����Ҷ�Ӧ��span
			for (PAGE_ID i = 0; i < kspan->_n; ++i)
			{
				//_idSpanMap[kspan->_pageId + i] = kspan;
				_idSpanMap.set(kspan->_pageId + i, kspan);
			}

			return kspan;
		}
	}

	//�ߵ����λ�þ�˵��û�д�ҳ��span��
	//��ʱ��ȥ�Ҷ�Ҫһ��128ҳ��span
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
		//����128page ��ֱ�ӻ�����
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanpool.Delete(span);

		return;
	}

	//��spanǰ���ҳ�����Խ��кϲ�,�����ڴ���Ƭ����
	while (1)
	{
		PAGE_ID previd = span->_pageId - 1;
		//auto ret = _idSpanMap.find(previd);
		
		////ǰ���ҳ��û�У� ���ϲ���
		//if (ret == _idSpanMap.end())
		//{
		//	break;
		//}
		auto ret = (Span*)_idSpanMap.get(previd);
		if (ret == nullptr)
		{
			break;
		}

		//ǰ������ҳ��span��ʹ�ã����ϲ���
		Span* prevSpan = ret;
		if (prevSpan->_isUse == true)
		{
			break;
		}

		//�ϲ�����128ҳ��spanû�취�������ϲ���
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

	//���ϲ�
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