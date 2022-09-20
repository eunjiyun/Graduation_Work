#include <windows.h>
#include <iostream>

template<class T, int SIZE = 50>
class MemoryPool
{
public:
	static void* operator new(std::size_t allocLength)
	{
		if (!mFreePointer)
			AllocBlock();
		UCHAR* returnPointer = mFreePointer;

		mFreePointer = *reinterpret_cast<UCHAR**>(returnPointer);

		return returnPointer;
	}

	static void operator delete(void* deletePointer)
	{
		*reinterpret_cast<UCHAR**>(deletePointer) = mFreePointer;

		mFreePointer = static_cast<UCHAR*>(deletePointer);
	}
private:
	static UCHAR* mFreePointer;
	static void AllocBlock()
	{
		mFreePointer = new UCHAR[sizeof(T) * SIZE];
		UCHAR** current = reinterpret_cast<UCHAR**>(mFreePointer);
		UCHAR* next = mFreePointer;
		for (int i = 0; i < SIZE - 1; i++)
		{
			next += sizeof(T);
			*current = next;
			current = reinterpret_cast<UCHAR**>(next);
		}
		*current = 0;
	}
protected:
	~MemoryPool()
	{

	}
};

template<class T, int SIZE>
UCHAR* MemoryPool<T, SIZE>::mFreePointer;
