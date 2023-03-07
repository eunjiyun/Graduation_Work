#pragma once

#include <cassert>
#include <memory>
#include "stdafx.h"


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
    static void AllocBlock(size_t _size = MEMORY_BLOCK_SIZE) // �޸� �Ҵ� �Լ�
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


class MapObject : public CMemoryPool<MapObject>
{
public:
    XMFLOAT4X4 m_xmf4x4World;
    char						m_pstrName[64] = { '\0' };
    BoundingBox			m_xmOOBB = BoundingBox();
  
    MapObject() { m_xmf4x4World = Matrix4x4::Identity(); }
    MapObject(int nMaterials) { m_xmf4x4World = Matrix4x4::Identity(); }
    XMFLOAT3 GetPosition()
    {
        return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
    }
    XMFLOAT3 GetLook()
    {
        return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
    }
    XMFLOAT3 GetUp()
    {
        return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
    }
    XMFLOAT3 GetRight()
    {
        return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
    }
    void SetMaterial(UINT nIndex, UINT nReflection)
    {

    }
    void SetAlbedoColor(UINT nIndex, XMFLOAT4 xmf4Color)
    {

    }
    void SetEmissionColor(UINT nIndex, XMFLOAT4 xmf4Color)
    {

    }
    
};

void Transform_BoundingBox(BoundingBox* _BoundingBox, XMFLOAT4X4 _xmfWorld)
{
    XMVECTOR	xmvCenter = XMLoadFloat3(&_BoundingBox->Center);
    XMVECTOR	xmvExtents = XMLoadFloat3(&_BoundingBox->Extents);

    XMMATRIX xmMatrix = XMMatrixSet(
        _xmfWorld._11, _xmfWorld._12, _xmfWorld._13, _xmfWorld._14,
        _xmfWorld._21, _xmfWorld._22, _xmfWorld._23, _xmfWorld._24,
        _xmfWorld._31, _xmfWorld._32, _xmfWorld._33, _xmfWorld._34,
        _xmfWorld._41, _xmfWorld._42, _xmfWorld._43, _xmfWorld._44
    );


    xmvCenter = XMVector3Transform(xmvCenter, xmMatrix);
    xmvExtents = XMVector3TransformNormal(xmvExtents, xmMatrix);
    XMStoreFloat3(&_BoundingBox->Center, xmvCenter);
    XMStoreFloat3(&_BoundingBox->Extents, xmvExtents);
   
}

void LoadMeshFromFile(MapObject& obj, char* pstrFileName)
{
    FILE* pFile = NULL;
    ::fopen_s(&pFile, pstrFileName, "rb");
    ::rewind(pFile);

    char pstrToken[64] = { '\0' };

    BYTE nStrLength = 0;
    UINT nReads = 0;

    while (!::feof(pFile))
    {
        nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
        if (nReads == 0) break;
        nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile);
        pstrToken[nStrLength] = '\0';

        if (!strcmp(pstrToken, "<BoundingBox>:"))
        {
            nReads = (UINT)::fread(&obj.m_xmOOBB.Center, sizeof(float), 3, pFile);
            nReads = (UINT)::fread(&obj.m_xmOOBB.Extents, sizeof(float), 3, pFile);  
            break;
        }
    }
    

    ::fclose(pFile);
}


MapObject** LoadGameObjectsFromFile(char* pstrFileName, int* pnGameObjects)
{
    FILE* pFile = NULL;
    ::fopen_s(&pFile, pstrFileName, "rb");
    ::rewind(pFile);

    char pstrToken[64] = { '\0' };
    char pstrGameObjectName[64] = { '\0' };
    char pstrFilePath[64] = { '\0' };

    BYTE nStrLength = 0, nObjectNameLength = 0;
    UINT nReads = 0, nMaterials = 0;
    size_t nConverted = 0;

    nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
    nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<GameObjects>:"
    nReads = (UINT)::fread(pnGameObjects, sizeof(int), 1, pFile);

    MapObject** ppGameObjects = new MapObject * [*pnGameObjects];

    MapObject* pGameObject = NULL, * pObjectFound = NULL;
    for (int i = 0; i < *pnGameObjects; i++)
    {
        nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
        nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<GameObject>:"
        nReads = (UINT)::fread(&nObjectNameLength, sizeof(BYTE), 1, pFile);
        nReads = (UINT)::fread(pstrGameObjectName, sizeof(char), nObjectNameLength, pFile);
        pstrGameObjectName[nObjectNameLength] = '\0';

        nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
        nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<Materials>:"
        nReads = (UINT)::fread(&nMaterials, sizeof(int), 1, pFile);


        pGameObject = new MapObject(nMaterials);
        strcpy_s(pGameObject->m_pstrName, 64, pstrGameObjectName);

        //MapObject* pObjectFound = NULL;
        //for (int j = 0; j < i; j++)
        //{
        //	if (!strcmp(pstrGameObjectName, ppGameObjects[j]->m_pstrName))
        //	{
        //		pObjectFound = ppGameObjects[j];
        //		////23.01.03
        //		//ppGameObjects[j]->m_pMesh->m_xmBoundingBox.Center = ppGameObjects[j]->GetPosition();
        //		////

        //		//23.01.11
        //		//pGameObject->SetMesh(ppGameObjects[j]->m_pMesh);
        //		pGameObject->SetMesh(0, ppGameObjects[j]->m_ppMeshes[0]);
        //		//
        //		for (UINT k = 0; k < nMaterials; k++)
        //		{
        //			pGameObject->SetMaterial(k, ppGameObjects[j]->m_ppMaterials[k]);
        //		}

        //		break;
        //	}
        //}


        XMFLOAT4 xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        XMFLOAT4 xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        for (UINT k = 0; k < nMaterials; k++)
        {
           // if (!pObjectFound) pGameObject->SetMaterial(k, rand() % MAX_Scene_MATERIALS);

            nReads = (UINT)::fread(&xmf4AlbedoColor, sizeof(float), 4, pFile);

            //if (!pObjectFound) pGameObject->SetAlbedoColor(k, xmf4AlbedoColor);

            nReads = (UINT)::fread(&xmf4EmissionColor, sizeof(float), 4, pFile);
           // if (!pObjectFound) pGameObject->SetEmissionColor(k, xmf4EmissionColor);
        }

        nReads = (UINT)::fread(&pGameObject->m_xmf4x4World, sizeof(float), 16, pFile);

        if (!pObjectFound)
        {
            strcpy_s(pstrFilePath, 64, "Models/");
            strcpy_s(pstrFilePath + 7, 64 - 7, pstrGameObjectName);
            strcpy_s(pstrFilePath + 7 + nObjectNameLength, 64 - 7 - nObjectNameLength, ".bin");

            LoadMeshFromFile(*pGameObject, pstrFilePath);
        }
        
        pGameObject->m_xmOOBB.Transform(pGameObject->m_xmOOBB, XMLoadFloat4x4(&pGameObject->m_xmf4x4World));

        /*cout << "Name: " << pGameObject->m_pstrName << "\nCenter: " << pGameObject->m_xmOOBB.Center.x << ", " << pGameObject->m_xmOOBB.Center.y << ", " << pGameObject->m_xmOOBB.Center.z <<
            "\nExtents: " << pGameObject->m_xmOOBB.Extents.x << ", " << pGameObject->m_xmOOBB.Extents.y << ", " << pGameObject->m_xmOOBB.Extents.z << endl;*/


        ppGameObjects[i] = pGameObject;
        
    }

    ::fclose(pFile);

    return(ppGameObjects);
}

class Monster : public CMemoryPool<Monster>
{
private:
    XMFLOAT3 Look = { 0, 0, 1 };
    XMFLOAT3 Up = { 0, 1, 0 };
    XMFLOAT3 Right = { 1, 0, 0 };
    short view_range, type, power, speed;
    short target_id = -1; // �����ϴ� �÷��̾� ID
    array<float, MAX_USER_PER_ROOM> distances = { 10000.f };
    short room_num; // �� ���� ��ü�� �����ϴ� ���� �� �ѹ�
public:
    mutex mon_lock;
    short HP;
    BoundingBox BB;
    bool is_alive;
    XMFLOAT3 Pos;
    Monster() {}
    
    void Initialize(short _roomNum, short _type, XMFLOAT3 _pos)
    {
        Pos = _pos;
        room_num = _roomNum;
        is_alive = true;
        BB = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(5, 3, 5));
        switch (_type)
        {
        case 1:
            type = 1;
            HP = 100;
            power = 30;
            view_range = 200;
            speed = 5;
            break;
        case 2:
            type = 2;
            HP = 60;
            power = 30;
            view_range = 400;
            speed = 3;
            break;
        case 3:
            type = 3;
            HP = 10000;
            power = 50;
            view_range = 300;
            speed = 1;
            break;
        case 4:
            type = 4;
            HP = 500;
            power = 70;
            view_range = 1000;
            speed = 2;
            break;
        }
    }
    XMFLOAT3 GetPosition()
    {
        return Pos;
    }
    short getType()
    {
        return type;
    }
    void Move(XMFLOAT3 m_Shift)
    {
        Pos = Vector3::Add(Pos, m_Shift);
        BB.Center = Pos;
    }
    int get_targetID();
    XMFLOAT3 Find_Direction(XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos);
    void Update();


};

class A_star_Node : public CMemoryPool<A_star_Node>
{
public:
    float F = 0;
    float G = 0;
    float H = 0;
    A_star_Node* parent = nullptr;
    XMFLOAT3 Pos;
    A_star_Node(XMFLOAT3 _Pos, XMFLOAT3 _Dest_Pos, float _G = 0, A_star_Node* node = nullptr)
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

struct Comp
{
    bool operator()(A_star_Node* const& A, A_star_Node* const& B) const
    {
        if (A->F > B->F) return true;

        else return false;
    }
};
