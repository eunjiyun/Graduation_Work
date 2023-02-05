//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, int nRootParameters)
{
	m_nTextureType = nTextureType;

	m_nTextures = nTextures;
	if (m_nTextures > 0)
	{
		m_ppd3dTextures = new ID3D12Resource * [m_nTextures];
		for (int i = 0; i < m_nTextures; i++)m_ppd3dTextures[i] = NULL;
		m_ppd3dTextureUploadBuffers = new ID3D12Resource * [m_nTextures];
		for (int i = 0; i < m_nTextures; i++)m_ppd3dTextureUploadBuffers[i] = NULL;
		m_pd3dSrvGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		for (int i = 0; i < m_nTextures; i++)m_pd3dSrvGpuDescriptorHandles[i].ptr = NULL;

		m_pnResourceTypes = new UINT[m_nTextures];
		for (int i = 0; i < m_nTextures; i++)m_pnResourceTypes[i] = 0;

		m_pdxgiBufferFormats = new DXGI_FORMAT[m_nTextures];
		for (int i = 0; i < m_nTextures; i++)m_pnResourceTypes[i] = DXGI_FORMAT_UNKNOWN;
		m_pnBufferElements = new int[m_nTextures];
		for (int i = 0; i < m_nTextures; i++)m_pnBufferElements[i] = 0;
	}
	m_nRootParameters = nRootParameters;
	if (nRootParameters > 0)m_pnRootParameterIndices = new int[nRootParameters];
	for (int i = 0; i < m_nRootParameters; i++)m_pnRootParameterIndices[i] = -1;

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0)m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++)if (m_ppd3dTextures[i])m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}
	if (m_pnResourceTypes)delete[] m_pnResourceTypes;
	if (m_pdxgiBufferFormats)delete[] m_pdxgiBufferFormats;
	if (m_pnBufferElements)delete[] m_pnBufferElements;

	if (m_pnRootParameterIndices)delete[] m_pnRootParameterIndices;
	if (m_pd3dSrvGpuDescriptorHandles)delete[] m_pd3dSrvGpuDescriptorHandles;

	if (m_pd3dSamplerGpuDescriptorHandles)delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootParameterIndex(int nIndex, UINT nRootParameterIndex)
{
	m_pnRootParameterIndices[nIndex] = nRootParameterIndex;
}

void CTexture::SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_nRootParameters == m_nTextures)
	{
		for (int i = 0; i < m_nRootParameters; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[i], m_pd3dSrvGpuDescriptorHandles[i]);
		}
	}
	else
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[0], m_pd3dSrvGpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[nParameterIndex], m_pd3dSrvGpuDescriptorHandles[nTextureIndex]);
}

void CTexture::ReleaseShaderVariables()
{
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++)if (m_ppd3dTextureUploadBuffers[i])m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}

ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue, UINT nResourceType, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, nWidth, nHeight, 1, 0, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}

void CTexture::LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex)
{
	m_pnResourceTypes[nIndex] = RESOURCE_BUFFER;
	m_pdxgiBufferFormats[nIndex] = ndxgiFormat;
	m_pnBufferElements[nIndex] = nElements;
	m_ppd3dTextures[nIndex] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, &m_ppd3dTextureUploadBuffers[nIndex]);
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex)
{
	ID3D12Resource* pShaderResource = GetResource(nIndex);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(nIndex);
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = m_pdxgiBufferFormats[nIndex];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_pnBufferElements[nIndex];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}

	return(d3dShaderResourceViewDesc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pTexture)m_pTexture->Release();
	if (m_pShader)m_pShader->Release();
}

void CMaterial::SetTexture(CTexture* pTexture)
{
	if (m_pTexture)m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture)m_pTexture->AddRef();
}

void CMaterial::SetShader(CShader* pShader)
{
	//23.01.08
	//if (nullptr != this)
	//{

	//23.01.13
	if (m_pShader)m_pShader->Release();
	//
	m_pShader = pShader;
	if (m_pShader)m_pShader->AddRef();
	//}
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pTexture)m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader)m_pShader->ReleaseShaderVariables();
	if (m_pTexture)m_pTexture->ReleaseShaderVariables();
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture)m_pTexture->ReleaseUploadBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject(int nMeshes)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh * [m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;
	}

	//23.01.05
	m_nMaterials = nMeshes;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (UINT i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	//m_xmf4x4World = Matrix4x4::Identity();
	//
}

CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Release();

			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}

	//23.01.09
	//if (m_pMaterial)m_pMaterial->Release();
	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i])
				m_ppMaterials[i]->Release();

			m_ppMaterials[i] = NULL;
		}
		delete[] m_ppMaterials;
	}
	//
}

//23.01.05
void CGameObject::SetMesh(int nIndex, CMesh* pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex])
			m_ppMeshes[nIndex]->Release();

		m_ppMeshes[nIndex] = pMesh;

		if (pMesh)
			pMesh->AddRef();
	}
}

//void CGameObject::SetMesh(CMesh* pMesh)
//{
//	if (m_pMesh) m_pMesh->Release();
//	m_pMesh = pMesh;
//	if (pMesh) pMesh->AddRef();
//}
//

//23.01.09
//void CGameObject::SetShader(CShader *pShader)
//{
//	if (!m_pMaterial)
//	{
//		//23.01.08
//		//CMaterial *pMaterial = new CMaterial();
//		m_pMaterial = new CMaterial();
//		m_pMaterial->SetShader(pShader);
//		//
//		
//		//23.01.05
//		//SetMaterial(pMaterial);
//		//SetMaterial(0,pMaterial);
//		//
//	}
//	//23.01.08
//	//if (m_pMaterial)m_pMaterial->SetShader(pShader);
//	//
//}
void CGameObject::SetShader(CShader* pShader)
{
	//23.01.13
	///*if (!m_ppMaterials)
	//{
	//	m_ppMaterials = new CMaterial();
	//	m_ppMaterials->SetShader(pShader);
	//}*/

	////23.01.09
	//if (!m_ppMaterials)
	//{
	//	for (int i = 0; i < m_nMaterials; i++)
	//	{
	//		if (!m_ppMaterials[i])
	//			//m_ppMaterials[i]->Release();
	//		{
	//			m_ppMaterials[i] = new CMaterial();
	//			//23.01.13
	//			SetMaterial(i,m_ppMaterials[i]);
	//			//
	//			m_ppMaterials[i]->SetShader(pShader);
	//		}
	//	}
	//}
	////

	if (!m_ppMaterials[0])
	{
		//23.01.16
		//m_ppMaterials[0] = new CMaterial();
		//SetMaterial(0,m_ppMaterials[0]);
		CMaterial* pMaterial = new CMaterial();
		SetMaterial(0, pMaterial);
		//
	}
	if (m_ppMaterials[0])m_ppMaterials[0]->SetShader(pShader);
	//
}
//

//23.01.05
//void CGameObject::SetMaterial(CMaterial *pMaterial)
//{
//	if (m_pMaterial)m_pMaterial->Release();
//	m_pMaterial = pMaterial;
//	if (m_pMaterial)m_pMaterial->AddRef();
//}
void CGameObject::SetMaterial(UINT nIndex, CMaterial* pMaterial)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->Release();
		m_ppMaterials[nIndex] = pMaterial;
		if (pMaterial) pMaterial->AddRef();
	}
}
void CGameObject::SetMaterial(UINT nIndex, UINT nReflection)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->m_nMaterial = nReflection;
	}
}
//

//23.01.05
void CGameObject::SetAlbedoColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetAlbedoColor(xmf4Color);
	}
}

void CGameObject::SetEmissionColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetEmissionColor(xmf4Color);
	}
}
//

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObject->Map(0, NULL, (void**)&m_pcbMappedGameObject);
}

void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}

	//23.01.09
	//if (m_pMaterial)m_pMaterial->ReleaseShaderVariables();
	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; ++i)
			m_ppMaterials[i]->ReleaseShaderVariables();
	}
	//
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, 
		XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	//23.01.11
	/*if (m_pMaterial)
		m_pcbMappedGameObject->m_nMaterialID = m_pMaterial->m_nReflection;*/

	if (m_ppMaterials)
	{
		//for(int i=0;i<m_nMaterials;++i)
		m_pcbMappedGameObject->m_nMaterialID = m_ppMaterials[0]->m_nReflection;
	}
	//

	//23.01.05
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = 
		m_pd3dcbGameObject->GetGPUVirtualAddress();
	//23.01.27
	//pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dGpuVirtualAddress);
	//pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_OBJECT, d3dGpuVirtualAddress);//gameObject
	
	pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_OBJECT, m_d3dCbvGPUDescriptorHandle);
	//
	//
}

void CGameObject::Animate(float fTimeElapsed)
{
}

void CGameObject::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}

void CGameObject::SetRootParameter(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_OBJECT, m_d3dCbvGPUDescriptorHandle);
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//23.01.28
	OnPrepareRender(pd3dCommandList, pCamera);
	//

	//23.01.11
	//if (m_pMesh && m_ppMaterials)
	if (m_ppMeshes && m_ppMaterials)
		//
	{


		//23.01.13
		//player
		if (m_ppMaterials[0]->m_pShader)
		{
			m_ppMaterials[0]->m_pShader->Render(pd3dCommandList, pCamera);

			if (pCamera)pCamera->SetViewportsAndScissorRects(pd3dCommandList);
			if (pCamera)pCamera->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);

			//23.01.16
			SetRootParameter(pd3dCommandList);
			//

			m_ppMeshes[0]->Render(pd3dCommandList);
		}
		//

		//23.01.28
		else
		{
			//
			if (m_pd3dcbGameObject && m_pcbMappedGameObject)
				UpdateShaderVariables(pd3dCommandList);

			//23.01.11
			//m_pMesh->OnPreRender(pd3dCommandList);
			m_ppMeshes[0]->OnPreRender(pd3dCommandList);
			//

			//23.01.16
			//SetRootParameter(pd3dCommandList);
			//

			for (UINT i = 0; i < m_nMaterials; i++)
			{

				if (m_ppMaterials[i])
				{
					//23.01.06
					/*pd3dCommandList->SetGraphicsRoot32BitConstant(4, m_ppMaterials[i]->m_nMaterial, 0);
					pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_ppMaterials[i]->m_xmf4AlbedoColor, 4);
					pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_ppMaterials[i]->m_xmf4EmissionColor, 8);*/

					pd3dCommandList->SetGraphicsRoot32BitConstant(8, m_ppMaterials[i]->m_nMaterial, 0);
					pd3dCommandList->SetGraphicsRoot32BitConstants(8, 4, &m_ppMaterials[i]->m_xmf4AlbedoColor, 4);
					pd3dCommandList->SetGraphicsRoot32BitConstants(8, 4, &m_ppMaterials[i]->m_xmf4EmissionColor, 8);
					//
				}

				//23.01.13
				//SetRootParameter(pd3dCommandList);
				//

				//23.01.11
				//m_pMesh->Render(pd3dCommandList, i);

				//23.01.13
				m_ppMeshes[0]->Render(pd3dCommandList, i);
				/*if (1 == m_nMaterials)
				{
					for (int i = 0; i < m_nMeshes; ++i)
						m_ppMeshes[i]->Render(pd3dCommandList, i);
				}
				else
					m_ppMeshes[0]->Render(pd3dCommandList, i);*/
					//
			}
		}
		//
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}

	//23.01.11
	//if (m_pMaterial)m_pMaterial->ReleaseUploadBuffers();

	if (m_ppMaterials)
	{
		for (int i = 0; i < m_nMaterials; ++i)
			m_ppMaterials[i]->ReleaseUploadBuffers();
	}
	//
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRotatingObject::CRotatingObject(int nMeshes)
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

void CRotatingObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRevolvingObject::CRevolvingObject(int nMeshes)
{
	m_xmf3RevolutionAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_fRevolutionSpeed = 0.0f;
}

CRevolvingObject::~CRevolvingObject()
{
}

void CRevolvingObject::Animate(float fTimeElapsed)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3RevolutionAxis), XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	m_xmf4x4World = Matrix4x4::Multiply(m_xmf4x4World, mtxRotate);
}

