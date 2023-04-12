//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"

CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}
CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName)
{
	if (pstrFileName)
		LoadMeshFromFile(pd3dDevice, pd3dCommandList, pstrFileName);
}
CMesh::CMesh(const CMesh& other)
{
	m_nReferences = other.m_nReferences;
	strcpy_s(m_pstrMeshName, other.m_pstrMeshName);
	m_xmBoundingBox = other.m_xmBoundingBox;
	*m_pxmf3Positions = *other.m_pxmf3Positions;
	*m_pxmf3Normals = *other.m_pxmf3Normals;
	*m_pnIndices = *other.m_pnIndices;
	m_nSubsets = other.m_nSubsets;
	*m_pnSubSetIndices2 = *other.m_pnSubSetIndices2;
	*m_pnSubSetStartIndices = *other.m_pnSubSetStartIndices;
	**m_ppnSubSetIndices = **other.m_ppnSubSetIndices;
	m_pd3dPositionBuffer = other.m_pd3dPositionBuffer;
	m_pd3dPositionUploadBuffer = other.m_pd3dPositionUploadBuffer;
	m_pd3dNormalBuffer = other.m_pd3dNormalBuffer;
	m_pd3dNormalUploadBuffer = other.m_pd3dNormalUploadBuffer;
	m_nVertexBufferViews = other.m_nVertexBufferViews;
	m_pd3dVertexBufferViews = other.m_pd3dVertexBufferViews;
	m_ppd3dIndexBuffers = other.m_ppd3dIndexBuffers;
	m_ppd3dIndexUploadBuffers = other.m_ppd3dIndexUploadBuffers;
	m_pd3dIndexBufferViews = other.m_pd3dIndexBufferViews;
	//strcpy_s(pstrFileName, other.pstrFileName);
	m_nIndices = other.m_nIndices;
	*m_pnSubSetIndices = *other.m_pnSubSetIndices;
	m_nType = other.m_nType;
	m_xmf3AABBCenter = other.m_xmf3AABBCenter;
	m_xmf3AABBExtents = other.m_xmf3AABBExtents;
	m_d3dPrimitiveTopology = other.m_d3dPrimitiveTopology;
	m_nSlot = other.m_nSlot;
	m_nOffset = other.m_nOffset;
	m_nVertices = other.m_nVertices;
	m_d3dPositionBufferView = other.m_d3dPositionBufferView;
	m_nSubMeshes = other.m_nSubMeshes;
	m_ppd3dSubSetIndexBuffers = other.m_ppd3dSubSetIndexBuffers;
	m_ppd3dSubSetIndexUploadBuffers = other.m_ppd3dSubSetIndexUploadBuffers;
	m_pd3dSubSetIndexBufferViews = other.m_pd3dSubSetIndexBufferViews;
}

CMesh::~CMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
			if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
		if (m_ppnSubSetIndices) delete[] m_ppnSubSetIndices;
	}

	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf2TextureCoords) delete[] m_pxmf2TextureCoords;
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
}

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet, UINT nSubset)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	if (m_pnSubSetIndices)
	{
		UpdateShaderVariables(pd3dCommandList);
		OnPreRender(pd3dCommandList, NULL);

		if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
		{
			pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
			pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
		}
		else
		{
			pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
		}
	}
	else
	{
		pd3dCommandList->IASetVertexBuffers(m_nSlot, m_nVertexBufferViews, m_pd3dVertexBufferViews);

		if (m_nSubsets > 0)
		{
			pd3dCommandList->IASetIndexBuffer(&m_pd3dIndexBufferViews[nSubset]);
			pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices2[nSubset], 1, 0, 0, 0);
		}
		else
		{
			pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
		}
	}
}

void CMesh::OnPostRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
}


void CMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName)
{
#ifdef _WITH_TEXT_MESH
	ifstream InFile(pstrFileName);

	char pstrToken[64] = { '\0' };

	for (; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!strcmp(pstrToken, "<Vertices>:"))
		{
			InFile >> m_nVertices;
			m_pxmf3Positions = new XMFLOAT3[m_nVertices];
			for (UINT i = 0; i < m_nVertices; i++) InFile >> m_pxmf3Positions[i].x >> m_pxmf3Positions[i].y >> m_pxmf3Positions[i].z;
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			InFile >> pstrToken;
			m_pxmf3Normals = new XMFLOAT3[m_nVertices];
			for (UINT i = 0; i < m_nVertices; i++) InFile >> m_pxmf3Normals[i].x >> m_pxmf3Normals[i].y >> m_pxmf3Normals[i].z;
		}
		else if (!strcmp(pstrToken, "<Indices>:"))
		{
			InFile >> m_nIndices;
			m_pnIndices = new UINT[m_nIndices];
			for (UINT i = 0; i < m_nIndices; i++) InFile >> m_pnIndices[i];
		}
	}
#else
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
			nReads = (UINT)::fread(&OBBox.Center, sizeof(float), 3, pFile);
			nReads = (UINT)::fread(&OBBox.Extents, sizeof(float), 3, pFile);
			nReads = (UINT)::fread(&OBBox.Orientation, sizeof(float), 4, pFile);

			if (0 == strncmp(pstrFileName, "Bedroom_wall", 12))
			{
				nReads = (UINT)::fread(&m_xmBoundingBox.Center, sizeof(float), 3, pFile);
				nReads = (UINT)::fread(&m_xmBoundingBox.Extents, sizeof(float), 3, pFile);
			}

			/*OBBox.Center = m_xmBoundingBox.Center;
			OBBox.Extents = m_xmBoundingBox.Extents;*/
		}
		else if (!strcmp(pstrToken, "<Vertices>:"))
		{
			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pFile);
			m_pxmf3Positions = new XMFLOAT3[m_nVertices];
			nReads = (UINT)::fread(m_pxmf3Positions, sizeof(float), 3 * m_nVertices, pFile);
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pFile);
			m_pxmf3Normals = new XMFLOAT3[m_nVertices];
			nReads = (UINT)::fread(m_pxmf3Normals, sizeof(float), 3 * m_nVertices, pFile);
		}
		else if (!strcmp(pstrToken, "<TextureCoords>:"))
		{
			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pFile);
			m_pxmf2TextureCoords = new XMFLOAT2[m_nVertices];
			nReads = (UINT)::fread(m_pxmf2TextureCoords, sizeof(float), 2 * m_nVertices, pFile);
		}
		else if (!strcmp(pstrToken, "<Indices>:"))
		{
			nReads = (UINT)::fread(&m_nIndices, sizeof(int), 1, pFile);
			m_pnIndices = new UINT[m_nIndices];
			nReads = (UINT)::fread(m_pnIndices, sizeof(UINT), m_nIndices, pFile);
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&m_nSubsets, sizeof(int), 1, pFile);

			m_pnSubSetIndices2 = new UINT[m_nSubsets];
			m_pnSubSetStartIndices = new UINT[m_nSubsets];
			m_ppnSubSetIndices = new UINT * [m_nSubsets];

			for (UINT i = 0; i < m_nSubsets; i++)
			{
				nReads = (UINT)::fread(&m_pnSubSetStartIndices[i], sizeof(UINT), 1, pFile);
				nReads = (UINT)::fread(&m_pnSubSetIndices2[i], sizeof(UINT), 1, pFile);
				nReads = (UINT)::fread(&m_nIndices, sizeof(int), 1, pFile);
				m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices2[i]];
				nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices2[i], pFile);
			}

			break;
		}
	}


	::fclose(pFile);
#endif

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);
	m_pd3dTextureCoordsBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoordUploadBuffer);

	m_nVertexBufferViews = 3;
	m_pd3dVertexBufferViews = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferViews];

	m_pd3dVertexBufferViews[0].BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[0].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[0].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dVertexBufferViews[1].BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[1].StrideInBytes = sizeof(XMFLOAT3);
	m_pd3dVertexBufferViews[1].SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dVertexBufferViews[2].BufferLocation = m_pd3dTextureCoordsBuffer->GetGPUVirtualAddress();
	m_pd3dVertexBufferViews[2].StrideInBytes = sizeof(XMFLOAT2);
	m_pd3dVertexBufferViews[2].SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_ppd3dIndexBuffers = new ID3D12Resource * [m_nSubsets];
	m_ppd3dIndexUploadBuffers = new ID3D12Resource * [m_nSubsets];
	m_pd3dIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubsets];

	for (UINT i = 0; i < m_nSubsets; i++)
	{
		m_ppd3dIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices2[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dIndexUploadBuffers[i]);

		m_pd3dIndexBufferViews[i].BufferLocation = m_ppd3dIndexBuffers[i]->GetGPUVirtualAddress();
		m_pd3dIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
		m_pd3dIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices2[i];
	}
}



void CMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, m_nVertexBufferViews, m_pd3dVertexBufferViews);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardMesh::CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CStandardMesh::~CStandardMesh()
{
	if (m_pd3dTextureCoord0Buffer) m_pd3dTextureCoord0Buffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dTangentBuffer) m_pd3dTangentBuffer->Release();
	if (m_pd3dBiTangentBuffer) m_pd3dBiTangentBuffer->Release();

	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;
	if (m_pxmf3Tangents) delete[] m_pxmf3Tangents;
	if (m_pxmf3BiTangents) delete[] m_pxmf3BiTangents;
	if (m_pxmf2TextureCoords0) delete[] m_pxmf2TextureCoords0;
	if (m_pxmf2TextureCoords1) delete[] m_pxmf2TextureCoords1;
}

void CStandardMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) m_pd3dTextureCoord0UploadBuffer->Release();
	m_pd3dTextureCoord0UploadBuffer = NULL;

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;

	if (m_pd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	m_pd3dTangentUploadBuffer = NULL;

	if (m_pd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	m_pd3dBiTangentUploadBuffer = NULL;
}

void CStandardMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	int nPositions = 0, nColors = 0, nNormals = 0, nTangents = 0, nBiTangents = 0, nTextureCoords = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	UINT nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nReads = (UINT)::fread(&nPositions, sizeof(int), 1, pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);

				m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

				m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
				m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nReads = (UINT)::fread(&nColors, sizeof(int), 1, pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_pxmf2TextureCoords0 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords0, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord0UploadBuffer);

				m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_pxmf2TextureCoords1 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_pxmf2TextureCoords1, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTextureCoord1UploadBuffer);

				m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nReads = (UINT)::fread(&nNormals, sizeof(int), 1, pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBiTangentUploadBuffer);

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			nReads = (UINT)::fread(&(m_nSubMeshes), sizeof(int), 1, pInFile);
			if (m_nSubMeshes > 0)
			{
				m_pnSubSetIndices = new int[m_nSubMeshes];
				m_ppnSubSetIndices = new UINT * [m_nSubMeshes];

				m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
				m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
				m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

				for (int i = 0; i < m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);
					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);
						if (m_pnSubSetIndices[i] > 0)
						{
							m_ppnSubSetIndices[i] = new UINT[m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_ppnSubSetIndices[i], sizeof(UINT), m_pnSubSetIndices[i], pInFile);

							m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

							m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

void CStandardMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[5]
		= { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 5, pVertexBufferViews);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedMesh::CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList)
{
}
CSkinnedMesh::CSkinnedMesh(const CSkinnedMesh& other)
{
	m_nBonesPerVertex = other.m_nBonesPerVertex;
	*m_pxmn4BoneIndices = *other.m_pxmn4BoneIndices;
	*m_pd3dBoneIndexBuffer = *other.m_pd3dBoneIndexBuffer;
	*m_pd3dBoneIndexUploadBuffer = *other.m_pd3dBoneIndexUploadBuffer;
	m_d3dBoneIndexBufferView = other.m_d3dBoneIndexBufferView;
	*m_pd3dBoneWeightBuffer = *other.m_pd3dBoneWeightBuffer;
	*m_pd3dBoneWeightUploadBuffer = *other.m_pd3dBoneWeightUploadBuffer;
	m_d3dBoneWeightBufferView = other.m_d3dBoneWeightBufferView;
	m_nSkinningBones = other.m_nSkinningBones;
	strcpy_s(*m_ppstrSkinningBoneNames, *(other.m_ppstrSkinningBoneNames));
	m_ppSkinningBoneFrameCaches = new CGameObject * (*other.m_ppSkinningBoneFrameCaches);
	*m_pxmf4x4BindPoseBoneOffsets = *other.m_pxmf4x4BindPoseBoneOffsets;
	*m_pd3dcbBindPoseBoneOffsets = *other.m_pd3dcbBindPoseBoneOffsets;
	*m_pcbxmf4x4MappedBindPoseBoneOffsets = *other.m_pcbxmf4x4MappedBindPoseBoneOffsets;
	*m_pcbxmf4x4MappedBindPoseBoneOffsets = *other.m_pcbxmf4x4MappedBindPoseBoneOffsets;
	*m_pd3dcbSkinningBoneTransforms = *other.m_pd3dcbSkinningBoneTransforms;
	*m_pcbxmf4x4MappedSkinningBoneTransforms = *other.m_pcbxmf4x4MappedSkinningBoneTransforms;
}

CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pxmn4BoneIndices) delete[] m_pxmn4BoneIndices;
	if (m_pxmf4BoneWeights) delete[] m_pxmf4BoneWeights;

	if (m_ppSkinningBoneFrameCaches) delete[] m_ppSkinningBoneFrameCaches;
	if (m_ppstrSkinningBoneNames) delete[] m_ppstrSkinningBoneNames;

	if (m_pxmf4x4BindPoseBoneOffsets) delete[] m_pxmf4x4BindPoseBoneOffsets;
	if (m_pd3dcbBindPoseBoneOffsets) m_pd3dcbBindPoseBoneOffsets->Release();

	if (m_pd3dBoneIndexBuffer) m_pd3dBoneIndexBuffer->Release();
	if (m_pd3dBoneWeightBuffer) m_pd3dBoneWeightBuffer->Release();

	ReleaseShaderVariables();
}

void CSkinnedMesh::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dcbBindPoseBoneOffsets)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = m_pd3dcbBindPoseBoneOffsets->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_BONEOFFSET, d3dcbBoneOffsetsGpuVirtualAddress); //Skinned Bone Offsets
	}

	if (m_pd3dcbSkinningBoneTransforms)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbSkinningBoneTransforms->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_BONETRANSFORM, d3dcbBoneTransformsGpuVirtualAddress); //Skinned Bone Transforms

		for (int j = 0; j < m_nSkinningBones; j++)
		{
			XMStoreFloat4x4(&m_pcbxmf4x4MappedSkinningBoneTransforms[j], XMMatrixTranspose(XMLoadFloat4x4(&m_ppSkinningBoneFrameCaches[j]->m_xmf4x4World)));
		}
	}
}

void CSkinnedMesh::ReleaseShaderVariables()
{
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CStandardMesh::ReleaseUploadBuffers();

	if (m_pd3dBoneIndexUploadBuffer) m_pd3dBoneIndexUploadBuffer->Release();
	m_pd3dBoneIndexUploadBuffer = NULL;

	if (m_pd3dBoneWeightUploadBuffer) m_pd3dBoneWeightUploadBuffer->Release();
	m_pd3dBoneWeightUploadBuffer = NULL;
}

void CSkinnedMesh::PrepareSkinning(CGameObject* pModelRootObject)
{
	for (int j = 0; j < m_nSkinningBones; j++)
	{
		m_ppSkinningBoneFrameCaches[j] = pModelRootObject->FindFrame(m_ppstrSkinningBoneNames[j]);//본네임에 ???값이 들어있음 0218오류
	}
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)//여기서 본네임 초기화 0218
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);
		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			m_nBonesPerVertex = ::ReadIntegerFromFile(pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))//본네임 0218
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
				m_ppSkinningBoneFrameCaches = new CGameObject * [m_nSkinningBones];
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					::ReadStringFromFile(pInFile, m_ppstrSkinningBoneNames[i]);
					m_ppSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			m_nSkinningBones = ::ReadIntegerFromFile(pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(XMFLOAT4X4), m_nSkinningBones, pInFile);

				UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256의 배수
				m_pd3dcbBindPoseBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
				m_pd3dcbBindPoseBoneOffsets->Map(0, NULL, (void**)&m_pcbxmf4x4MappedBindPoseBoneOffsets);

				for (int i = 0; i < m_nSkinningBones; i++)
				{
					XMStoreFloat4x4(&m_pcbxmf4x4MappedBindPoseBoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneIndices>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmn4BoneIndices = new XMINT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmn4BoneIndices, sizeof(XMINT4), m_nVertices, pInFile);
				m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmn4BoneIndices, sizeof(XMINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

				m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMINT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			m_nVertices = ::ReadIntegerFromFile(pInFile);
			if (m_nVertices > 0)
			{
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
				m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

				m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[7] = { m_d3dPositionBufferView, m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dBoneIndexBufferView, m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 7, pVertexBufferViews);
}