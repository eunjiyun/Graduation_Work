#pragma once

#include <cassert>
#include <memory>
#include "stdafx.h"


using namespace std;
typedef unsigned char UCHAR;
typedef unsigned int UINT;

#define USEPOOL 0

template<class T>
class CObjectPool {
private:
    queue<T*> objectQueue;
    mutex pool_lock;
public:
    CObjectPool(size_t MemorySize)
    {
        for (int i = 0; i < MemorySize; ++i) {
            objectQueue.push(new T());
        }
    }
    ~CObjectPool()
    {
        objectQueue = queue<T*>();
    }
    T* GetMemory()
    {
        if (objectQueue.empty()) {
            cout << "추가요청이 호출됨\n";
            for (int i = 0; i < 500; ++i)
                objectQueue.push(new T());
        }

        lock_guard<mutex> ll{ pool_lock };
        auto front = objectQueue.front();
        objectQueue.pop();
        return front;
    }
    void ReturnMemory(T* Mem)
    {
        lock_guard<mutex> ll{ pool_lock };
        objectQueue.push(Mem);
    }
    void PrintSize()
    {
        cout << "CurrentSize - " << objectQueue.size() << endl;
    }
};




class A_star_Node //: public CMemoryPool<A_star_Node>
{
public:
    float F = 0;
    float G = 0;
    float H = 0;
    shared_ptr<A_star_Node> parent = nullptr;
    XMFLOAT3 Pos = { 0,0,0 };
    A_star_Node() {}
    A_star_Node(XMFLOAT3 _Pos, XMFLOAT3 _Dest_Pos, float _G = 0, shared_ptr<A_star_Node> node = nullptr)
    {
        Pos = _Pos;
        G = _G;
        H = fabs(_Dest_Pos.z - Pos.z) + fabs(_Dest_Pos.x - Pos.x);
        F = G + H;
        if (node) {
            parent = node;
        }
    }
    void Initialize(XMFLOAT3 _Pos, XMFLOAT3 _Dest_Pos, float _G = 0, shared_ptr<A_star_Node> node = nullptr)
    {
        Pos = _Pos;
        G = _G;
        H = fabs(_Dest_Pos.z - Pos.z) + fabs(_Dest_Pos.x - Pos.x);
        F = G + H;
        if (node) {
            parent = node;
        }
    }
};



