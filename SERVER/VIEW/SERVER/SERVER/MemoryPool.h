#pragma once

#include <cassert>
#include <memory>
#include "stdafx.h"

#define _USE_MYLOCKFREEQUEUE 

template <class T>
class Node {
public:
    T data;
    std::atomic<Node*> next;
    std::atomic<short> ref;
    Node(const T& value) : data(value), next(nullptr), ref(1) {}
};

template<class T>
class LockFreeQueue {
private:
    std::atomic<Node<T>*> head;
    std::atomic<Node<T>*> tail;
    std::atomic<size_t> size;
public:
    LockFreeQueue() : head(new Node<T>(T())), tail(head.load()), size(0) {}
    ~LockFreeQueue() {
        T value;
        while (pop(value));
        delete head.load();
    }

    bool push(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        while (true) {
            Node<T>* oldTail = tail.load();
            Node<T>* oldNext = oldTail->next.load();
            if (oldTail == tail.load()) {
                if (oldNext == nullptr) {
                    if (oldTail->next.compare_exchange_strong(oldNext, newNode)) {
                        tail.compare_exchange_strong(oldTail, newNode);
                        size.fetch_add(1);
                        cout << size.load() << endl;
                        return true;
                    }
                }
                else {
                    tail.compare_exchange_strong(oldTail, oldNext);
                }
            }
        }
    }

    bool pop(T& value) {
        while (true) {
            Node<T>* oldHead = head.load();
            Node<T>* oldTail = tail.load();
            Node<T>* oldNext = oldHead->next.load();
            if (oldHead == head.load()) {
                if (oldHead == oldTail) {
                    if (oldNext == nullptr) {
                        return false;
                    }
                    tail.compare_exchange_strong(oldTail, oldNext);
                }
                else {
                    if (head.compare_exchange_strong(oldHead, oldNext)) {
                        oldHead->ref.fetch_sub(1);
                        if (oldHead->ref.load() == 0) {
                            delete oldHead;
                        }
                        value = oldNext->data;
                        size.fetch_sub(1);
                        cout << size.load() << endl;
                        return true;
                    }
                }
            }
        }
    }

    size_t get_size() const { // 추가된 부분
        return size.load();
    }

    bool empty() const {
        return head.load() == tail.load();
    }
};


template<class T>
class CObjectPool {
#ifdef _USE_MYLOCKFREEQUEUE
private:
    LockFreeQueue<T*> objectQueue;
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
        while (objectQueue.pop(mem))
        {
            delete mem;
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
        if (objectQueue.pop(front))
            return front;
        else
            return nullptr;
    }
    void ReturnMemory(T* Mem)
    {
        objectQueue.push(Mem);
    }
    void PrintSize()
    {
        cout << "CurrentSize - " << objectQueue.get_size() << endl;
    }
#elif
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
            delete mem;
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
        if (objectQueue.try_pop(front))
            return front;
        else
            return nullptr;
    }
    void ReturnMemory(T* Mem)
    {
        objectQueue.push(Mem);
    }
    void PrintSize()
    {
        cout << "CurrentSize - " << objectQueue.unsafe_size() << endl;
    }
#endif
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


