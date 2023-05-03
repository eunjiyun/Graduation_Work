#pragma once
#include "stdafx.h"


//시작방 - 260 - 1200
//1번방 - 1260 - 2550
//2번방 - 2650 - 3550
//계단방 - 3690 - 4500

class MapObject
{
public:
    XMFLOAT4X4 m_xmf4x4World;
    char		m_pstrName[64] = { '\0' };
    BoundingOrientedBox		m_xmOOBB = BoundingOrientedBox();

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

class Key_Object
{
public:
    BoundingOrientedBox		m_xmOOBB;
    float percent;
    unsigned short obj_id;
};
void Transform_BoundingBox(BoundingBox* _BoundingBox, XMFLOAT4X4 _xmfWorld);

void LoadMeshFromFile(MapObject& obj, char* pstrFileName);
MapObject** LoadGameObjectsFromFile(char* pstrFileName, int* pnGameObjects);


