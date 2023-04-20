#pragma once

#include <cassert>
#include <memory>
#include "stdafx.h"


template<class T>
class CObjectPool {
private:
    concurrent_queue<T*> objectQueue;
public:
    CObjectPool(size_t MemorySize)
    {
        for (int i = 0; i < MemorySize; ++i) {
            objectQueue.push(new T());
        }
    }
    ~CObjectPool()
    {
        T* mem;
        while (objectQueue.try_pop(mem))
        {
            unique_ptr<T> pData(mem);
        }
    }

    T* GetMemory()
    {
        if (objectQueue.empty()) {
            cout << "ObjectPool called add memory request\n";
            for (int i = 0; i < 500; ++i)
                objectQueue.push(new T());
        }
        T* front;
        objectQueue.try_pop(front);
        return front;
    }
    void ReturnMemory(T* Mem)
    {
        objectQueue.push(Mem);
    }
    void PrintSize()
    {
        cout << "CurrentSize - " << objectQueue.unsafe_size() << endl;
    }
};




class A_star_Node
{
public:
    float F = 0;
    float G = 0;
    float H = 0;
   shared_ptr<A_star_Node> parent;
    XMFLOAT3 Pos = { 0,0,0 };
    A_star_Node() {}
    A_star_Node(XMFLOAT3 _Pos, XMFLOAT3 _Dest_Pos, float _G,shared_ptr<A_star_Node> node)
    {
        Pos = _Pos;
        G = _G;
        H = fabs(_Dest_Pos.z - Pos.z) + fabs(_Dest_Pos.x - Pos.x);
        F = G + H;
        if (node) {
            parent = node;
        }
    }
    void Initialize(XMFLOAT3 _Pos, XMFLOAT3 _Dest_Pos, float _G,shared_ptr<A_star_Node> node)
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

class AStar_Pool {
private:
    queue<shared_ptr<A_star_Node>> objectQueue;
    mutex pool_lock;
public:
    AStar_Pool()
    {
        for (int i = 0; i < 4000; ++i) {
            objectQueue.push(make_shared<A_star_Node>());
        }
    }
    ~AStar_Pool() 
    {
    }

   shared_ptr<A_star_Node> GetMemory(XMFLOAT3 _Pos, XMFLOAT3 _Dest_Pos, float _G = 0,shared_ptr<A_star_Node> node = nullptr)
    {
        lock_guard<mutex> ll{ pool_lock };
        if (objectQueue.empty()) {
            cout << "AStar_Pool called add memory request\n";
            for (int i = 0; i < 500; ++i)
                objectQueue.push(make_shared<A_star_Node>());
        }
        if (!objectQueue.empty()) {
            auto& front = objectQueue.front();
            objectQueue.pop();

            front->Initialize(_Pos, _Dest_Pos, _G, node);

            return front;
        }
        else {
            return nullptr;
        }
    }

    void ReturnMemory(shared_ptr<A_star_Node> Mem)
    {
        Mem->parent.reset();
        Mem->Pos = { 0,0,0 };
        Mem->G = Mem->H = Mem->F = 0;
        lock_guard<mutex> ll{ pool_lock };
        objectQueue.push(Mem);
    }
    void PrintSize()
    {
        cout << "CurrentSize - " << objectQueue.size() << endl;
    }
};


