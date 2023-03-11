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
    queue<shared_ptr<T>> objectQueue;
public:
    CObjectPool(size_t MemorySize)
    {
        for (int i = 0; i < MemorySize; ++i) {
            objectQueue.push(make_shared<T>());
        }
    }
    ~CObjectPool()
    {
        objectQueue = queue<shared_ptr<T>>();
    }
    shared_ptr<T> GetMemory()
    {
        if (objectQueue.empty()) {
            cout << "추가요청이 호출됨\n";
            for (int i = 0; i< 50; ++i)
                objectQueue.push(make_shared<T>());
        }

        auto front = objectQueue.front();
        objectQueue.pop();

        return front;
    }
    void ReturnMemory(shared_ptr<T> Mem)
    {
        objectQueue.push(Mem);
    }
};


class MapObject
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

    char		strAlbedoTextureName[64] = { '\0' };
    char		strEmissionTextureName[64] = { '\0' };
    BYTE	nAlbedoTextureStrLength = 0;
    BYTE	nEmissionTextureStrLength = 0;

    char pstrToken1[64] = { '\0' };
    char pstrToken2[64] = { '\0' };
    char pstrGameObjectName[64] = { '\0' };
    char pstrFilePath[64] = { '\0' };

    BYTE nStrLength = 0, nObjectNameLength = 0;
    UINT nReads = 0, nMaterials = 0;
    size_t nConverted = 0;
    int	nTextureNumber = 0;
    int iCount = 0;

    nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
    nReads = (UINT)::fread(pstrToken1, sizeof(char), nStrLength, pFile); //"<GameObjects>:"
    nReads = (UINT)::fread(pnGameObjects, sizeof(int), 1, pFile);

    MapObject** ppGameObjects = new MapObject * [*pnGameObjects];

    nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
    nReads = (UINT)::fread(pstrToken2, sizeof(char), nStrLength, pFile); //"<Textures>:"
    nReads = (UINT)::fread(&nTextureNumber, sizeof(int), 1, pFile);
    cout << *pnGameObjects << endl;
    cout << nTextureNumber << endl;

    MapObject* pGameObject = NULL, * pObjectFound = NULL;
    for (int i = 0; i < *pnGameObjects; i++)
    {
        char pstrToken3[64] = { '\0' };
        char pstrToken4[64] = { '\0' };
        char pstrToken5[64] = { '\0' };
        char pstrToken6[64] = { '\0' };

        nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
        nReads = (UINT)::fread(pstrToken3, sizeof(char), nStrLength, pFile); //"<GameObject>:"
        nReads = (UINT)::fread(&nObjectNameLength, sizeof(BYTE), 1, pFile);
        nReads = (UINT)::fread(pstrGameObjectName, sizeof(char), nObjectNameLength, pFile);
        pstrGameObjectName[nObjectNameLength] = '\0';

        nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
        nReads = (UINT)::fread(pstrToken4, sizeof(char), nStrLength, pFile); //"<Materials>:"
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


        XMFLOAT4 xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); XMFLOAT4 xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        for (UINT k = 0; k < nMaterials; k++)
        {

            //if (!pObjectFound) pGameObject->SetMaterial(k, rand() % MAX_Scene_MATERIALS);

            nReads = (UINT)::fread(&xmf4AlbedoColor, sizeof(float), 4, pFile);
            //if (!pObjectFound) pGameObject->SetAlbedoColor(k, xmf4AlbedoColor);

            nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
            nReads = (UINT)::fread(pstrToken5, sizeof(char), nStrLength, pFile); //"<<AlbedoTextureName>:"
            if (0 == strcmp(pstrToken5, "<AlbedoTextureName>:"))
            {
                nReads = (UINT)::fread(&nAlbedoTextureStrLength, sizeof(BYTE), 1, pFile);
                nReads = (UINT)::fread(strAlbedoTextureName, sizeof(char), nAlbedoTextureStrLength, pFile);
                // pGameObject->Set_AlbedoTexture(k, pd3dDevice, pd3dCommandList, strAlbedoTextureName, nTextureNumber);
            }

            nReads = (UINT)::fread(&xmf4EmissionColor, sizeof(float), 4, pFile);
            // if (!pObjectFound) pGameObject->SetEmissionColor(k, xmf4EmissionColor);


            nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
            nReads = (UINT)::fread(pstrToken6, sizeof(char), nStrLength, pFile); //"<EmissionTextureName>:"

            if (0 == strcmp(pstrToken6, "<EmissionTextureName>:"))
            {
                nReads = (UINT)::fread(&nEmissionTextureStrLength, sizeof(BYTE), 1, pFile);
                nReads = (UINT)::fread(strEmissionTextureName, sizeof(char), nEmissionTextureStrLength, pFile);
                cout << pstrToken6 << strEmissionTextureName << endl << endl;
            }
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

        //cout << "Name: " << pGameObject->m_pstrName << "\nCenter: " << pGameObject->m_xmOOBB.Center.x << ", " << pGameObject->m_xmOOBB.Center.y << ", " << pGameObject->m_xmOOBB.Center.z <<
        //    "\nExtents: " << pGameObject->m_xmOOBB.Extents.x << ", " << pGameObject->m_xmOOBB.Extents.y << ", " << pGameObject->m_xmOOBB.Extents.z << endl;


        ppGameObjects[i] = pGameObject;
        
    }

    ::fclose(pFile);

    return(ppGameObjects);
}

class A_star_Node //: public CMemoryPool<A_star_Node>
{
public:
    float F = 0;
    float G = 0;
    float H = 0;
    A_star_Node* parent = nullptr;
    XMFLOAT3 Pos = { 0,0,0 };
    A_star_Node() {}
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
    void Initialize(XMFLOAT3 _Pos, XMFLOAT3 _Dest_Pos, float _G = 0, A_star_Node* node = nullptr)
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
class Monster
{
private:
    XMFLOAT3 m_xmf3Look = { 0, 0, 1 };
    XMFLOAT3 m_xmf3Up = { 0, 1, 0 };
    XMFLOAT3 m_xmf3Right = { 1, 0, 0 };
    short view_range, type, power, speed;
    array<float, MAX_USER_PER_ROOM> distances = { 10000.f };
    short room_num; // 이 몬스터 객체가 존재하는 게임 룸 넘버
#if USEPOOL == 1
    std::unique_ptr<CObjectPool<A_star_Node>> PoolHandle = make_unique<CObjectPool<A_star_Node>>(500);
#endif
public:
    mutex mon_lock;
    short HP;
    BoundingBox BB;
    bool is_alive = false;
    XMFLOAT3 Pos;
    short cur_animation_track = 0;
    float rotate_Angle = 0.f;
    short target_id = -1; // 추적하는 플레이어 ID
    Monster() { }
    
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
            speed = 10;
            break;
        case 2:
            type = 2;
            HP = 60;
            power = 30;
            view_range = 400;
            speed = 6;
            break;
        case 3:
            type = 3;
            HP = 10000;
            power = 50;
            view_range = 300;
            speed = 2;
            break;
        case 4:
            type = 4;
            HP = 500;
            power = 70;
            view_range = 1000;
            speed = 4;
            break;
        }
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
    void Rotate(float x, float y, float z)
    {
        XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
        m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
        m_xmf3Look = Vector3::Normalize(m_xmf3Look);
        m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
        m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
    }

    int get_targetID();
    XMFLOAT3 Find_Direction(XMFLOAT3 start_Pos, XMFLOAT3 dest_Pos);
    void Update();
    XMFLOAT3 GetPosition() { return Pos; }
    XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
    XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
    XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
    void SetLookVector(XMFLOAT3 _Look) { m_xmf3Look = _Look; }

};



struct Comp
{
    bool operator()(A_star_Node* const& A, A_star_Node* const& B) const
    {
        if (A->F > B->F) return true;

        else return false;
    }
};
