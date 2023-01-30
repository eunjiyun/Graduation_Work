#pragma once

#include <iostream>
#include <cassert>
#include <memory>
#include <vector>

using namespace std;
typedef unsigned char UCHAR;
typedef unsigned int UINT;

template <class T, size_t MEMORY_BLOCK_SIZE = 50 >
class CMemoryPool {

private:
    static UCHAR* mPoolPointer;  
    static vector<UCHAR*> m_pointerForRelease;    
protected:
    ~CMemoryPool()
    {
    }

private:
    static void AllocBlock(size_t _size = MEMORY_BLOCK_SIZE) // 메모리 할당 함수
    {
        mPoolPointer = new UCHAR[sizeof(T) * _size];
        m_pointerForRelease.push_back(mPoolPointer);

        UCHAR** curr = reinterpret_cast<UCHAR**>(mPoolPointer);
        UCHAR* next = mPoolPointer;

        for (int i = 0; i < _size - 1; ++i)
        {
            next += sizeof(T);  
            *curr = next;
            curr = reinterpret_cast<UCHAR**>(next);
        }
        *curr = nullptr;
    }

public:

    static void* operator new(size_t _allocSize)
    {
        assert(sizeof(T) >= sizeof(UCHAR*));
        assert(sizeof(T) == _allocSize);

        if (!mPoolPointer)
            AllocBlock();

        UCHAR* returnPointer = mPoolPointer;
        mPoolPointer = *reinterpret_cast<UCHAR**>(returnPointer);

        return returnPointer;
    }

    static void operator delete(void* deletePointer)
    {
        *reinterpret_cast<UCHAR**>(deletePointer) = mPoolPointer;
        mPoolPointer = static_cast<UCHAR*>(deletePointer);
    }

    static void ReleasePool()
    {
        for (auto i = m_pointerForRelease.begin(); i < m_pointerForRelease.end(); i++)
            delete[] * i;
    }
};


template <class T, size_t MEMORY_DEFAULT_SIZE>
UCHAR* CMemoryPool<T, MEMORY_DEFAULT_SIZE>::mPoolPointer = nullptr;

template <class T, size_t MEMORY_DEFAULT_SIZE>
vector<UCHAR*> CMemoryPool<T, MEMORY_DEFAULT_SIZE>::m_pointerForRelease;