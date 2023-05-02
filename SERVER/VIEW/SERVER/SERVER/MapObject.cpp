#include "MapObject.h"

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
            nReads = (UINT)::fread(&obj.m_xmOOBB.Orientation, sizeof(float), 4, pFile);
            break;
        }
    }


    ::fclose(pFile);
}

//int min_x = 0;
//int max_x = 0;
//int min_z = 0;
//int max_z = 0;
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


        XMFLOAT4 xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); XMFLOAT4 xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        for (UINT k = 0; k < nMaterials; k++)
        {



            nReads = (UINT)::fread(&xmf4AlbedoColor, sizeof(float), 4, pFile);


            nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
            nReads = (UINT)::fread(pstrToken5, sizeof(char), nStrLength, pFile); //"<<AlbedoTextureName>:"
            if (0 == strcmp(pstrToken5, "<AlbedoTextureName>:"))
            {
                nReads = (UINT)::fread(&nAlbedoTextureStrLength, sizeof(BYTE), 1, pFile);
                nReads = (UINT)::fread(strAlbedoTextureName, sizeof(char), nAlbedoTextureStrLength, pFile);

            }

            nReads = (UINT)::fread(&xmf4EmissionColor, sizeof(float), 4, pFile);
            // if (!pObjectFound) pGameObject->SetEmissionColor(k, xmf4EmissionColor);


            nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
            nReads = (UINT)::fread(pstrToken6, sizeof(char), nStrLength, pFile); //"<EmissionTextureName>:"

            if (0 == strcmp(pstrToken6, "<EmissionTextureName>:"))
            {
                nReads = (UINT)::fread(&nEmissionTextureStrLength, sizeof(BYTE), 1, pFile);
                nReads = (UINT)::fread(strEmissionTextureName, sizeof(char), nEmissionTextureStrLength, pFile);
                //cout << pstrToken6 << strEmissionTextureName << endl << endl;
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

        //cout << pGameObject->m_pstrName << endl;
        //if (min_x > pGameObject->m_xmOOBB.Center.x - pGameObject->m_xmOOBB.Extents.x) min_x = pGameObject->m_xmOOBB.Center.x - pGameObject->m_xmOOBB.Extents.x;
        //if (max_x < pGameObject->m_xmOOBB.Center.x + pGameObject->m_xmOOBB.Extents.x) max_x = pGameObject->m_xmOOBB.Center.x + pGameObject->m_xmOOBB.Extents.x;
        //if (min_z > pGameObject->m_xmOOBB.Center.z - pGameObject->m_xmOOBB.Extents.z) min_z = pGameObject->m_xmOOBB.Center.z - pGameObject->m_xmOOBB.Extents.z;
        //if (max_z < pGameObject->m_xmOOBB.Center.z + pGameObject->m_xmOOBB.Extents.z) max_z = pGameObject->m_xmOOBB.Center.z + pGameObject->m_xmOOBB.Extents.z;
        //cout << pGameObject->m_pstrName << endl;
        //Vector3::Print(pGameObject->m_xmOOBB.Center);
        //Vector3::Print(pGameObject->m_xmOOBB.Extents);
        //cout << endl;

        ppGameObjects[i] = pGameObject;

    }
    //cout << min_x << ", " << max_x << ", " << min_z << ", " << max_z << endl;
    ::fclose(pFile);

    return(ppGameObjects);
}