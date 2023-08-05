#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	//慢开始反馈算法
	//1、最开始不会一次想CentralCache一次批量要太多，因为要太多了可能用不完
	//2、如果你不要这个size大小的内存需求，那么batchNum就会不断增长，直到上限
	//3、size越大，一次向CentralCache要的batchNum就越小
	//4、size越小，一次向CentralCache要的batchNum就越大
	size_t batchNum = min(_freeLists[index].Maxsize(), Sizeclass::NumMoveSize(size));
	//size_t batchNum = (((_freeLists[index].Maxsize()) < (Sizeclass::NumMoveSize(size))) ? (_freeLists[index].Maxsize()) : (Sizeclass::NumMoveSize(size)));
	if (_freeLists[index].Maxsize() == batchNum)
	{
		_freeLists[index].Maxsize() += 1;
	}

	void* start = nullptr;
	void* end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum > 0);

	if (actualNum == 1)
	{
		assert(start == end);
		return start;
	}
	else
	{
		_freeLists[index].PushRange(NextObj(start), end, actualNum - 1);
		return start;
	}
}

void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	size_t alignSize = Sizeclass::RoundUp(size);
	size_t index = Sizeclass::Index(size);

	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	else
	{
		return FetchFromCentralCache(index, alignSize);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	//找到映射的自由链表桶，对象插入进入
	size_t index = Sizeclass::Index(size);
	_freeLists[index].Push(ptr);

	//当链表长度大于一次批量申请的内存时就开始还一段list给central cache
	if (_freeLists[index].Size() >= _freeLists[index].Maxsize())
	{
		ListTooLong(_freeLists[index], size);
	}
}

void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr;
	void* end = nullptr;
	list.PopRange(start, end, list.Maxsize());

	CentralCache::GetInstance()->ReleaseListToSpans(start, size);
}