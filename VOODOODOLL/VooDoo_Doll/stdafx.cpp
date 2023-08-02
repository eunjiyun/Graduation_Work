
#include "stdafx.h"

#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"

UINT gnCbvSrvDescriptorIncrementSize = 0;
UINT gnRtvDescriptorIncrementSize = 0;
UINT gnDsvDescriptorIncrementSize = 0;
UINT gnCbvSrvUavDescriptorIncrementSize = 0;

void WaitForGpuComplete(ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, UINT64 nFenceValue, HANDLE hFenceEvent)
{
	HRESULT hResult = pd3dCommandQueue->Signal(pd3dFence, nFenceValue);

	if (pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = pd3dFence->SetEventOnCompletion(nFenceValue, hFenceEvent);
		::WaitForSingleObject(hFenceEvent, INFINITE);
	}
}

void ExecuteCommandList(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, UINT64 nFenceValue, HANDLE hFenceEvent)
{
	pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { pd3dCommandList };
	pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	::WaitForGpuComplete(pd3dCommandQueue, pd3dFence, nFenceValue, hFenceEvent);
}

void SynchronizeResourceTransition(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12Resource* pd3dResource, D3D12_RESOURCE_STATES d3dStateBefore, D3D12_RESOURCE_STATES d3dStateAfter)
{
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dResource;
	d3dResourceBarrier.Transition.StateBefore = d3dStateBefore;
	d3dResourceBarrier.Transition.StateAfter = d3dStateAfter;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
}

ID3D12Resource* CreateTextureResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nBytes, D3D12_RESOURCE_DIMENSION d3dResourceDimension, UINT nWidth, UINT nHeight, UINT nDepthOrArraySize, UINT nMipLevels, D3D12_RESOURCE_FLAGS d3dResourceFlags, DXGI_FORMAT dxgiFormat, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, ID3D12Resource** ppd3dUploadBuffer)
{
	ID3D12Resource* pd3dBuffer = NULL;

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = d3dHeapType;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = d3dResourceDimension; //D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_DIMENSION_TEXTURE1D, D3D12_RESOURCE_DIMENSION_TEXTURE2D
	d3dResourceDesc.Alignment = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT : 0;
	d3dResourceDesc.Width = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? nBytes : nWidth;
	d3dResourceDesc.Height = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? 1 : nHeight;
	d3dResourceDesc.DepthOrArraySize = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? 1 : nDepthOrArraySize;
	d3dResourceDesc.MipLevels = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? 1 : nMipLevels;
	d3dResourceDesc.Format = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? DXGI_FORMAT_UNKNOWN : dxgiFormat;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = (d3dResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = d3dResourceFlags; //D3D12_RESOURCE_FLAG_NONE

	switch (d3dHeapType)
	{
	case D3D12_HEAP_TYPE_DEFAULT:
	{
		D3D12_RESOURCE_STATES d3dResourceInitialStates = (ppd3dUploadBuffer && pData) ? D3D12_RESOURCE_STATE_COPY_DEST : d3dResourceStates;
		HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceInitialStates, NULL, __uuidof(ID3D12Resource), (void**)&pd3dBuffer);

		if (hResult == DXGI_ERROR_DEVICE_REMOVED)
		{
			// GPU 장치 인스턴스가 중단되었으므로, 정확한 이유를 얻기 위해 GetDeviceRemovedReason 함수 사용
			HRESULT removedReason = pd3dDevice->GetDeviceRemovedReason();

			// removedReason을 사용하여 적절한 작업 수행
			// 예: 오류 메시지 출력, 오류 로그 기록, 재시작 등

			printf("Error1: removedReason 0x%X\n", removedReason);
			// 또는 printf("Error: %s\n", DXGetErrorString(hResult)); 와 같이 DXGetErrorString 함수를 사용하여 HRESULT를 문자열로 변환할 수도 있습니다.
		}
		else if (FAILED(hResult))
		{
			// 그 외의 오류 처리

			printf("Error2: HRESULT 0x%X\n", hResult);
			// 또는 printf("Error: %s\n", DXGetErrorString(hResult)); 와 같이 DXGetErrorString 함수를 사용하여 HRESULT를 문자열로 변환할 수도 있습니다.
		}

		if (ppd3dUploadBuffer && pData)
		{
			d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;

			d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			d3dResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			d3dResourceDesc.Width = nBytes;
			d3dResourceDesc.Height = 1;
			d3dResourceDesc.DepthOrArraySize = 1;
			d3dResourceDesc.MipLevels = 1;
			d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			d3dResourceDesc.SampleDesc.Count = 1;
			d3dResourceDesc.SampleDesc.Quality = 0;
			d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);
#ifdef _WITH_MAPPING
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			(*ppd3dUploadBuffer)->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			(*ppd3dUploadBuffer)->Unmap(0, NULL);

			pd3dCommandList->CopyResource(pd3dBuffer, *ppd3dUploadBuffer);
#else
			D3D12_SUBRESOURCE_DATA d3dSubResourceData;
			::ZeroMemory(&d3dSubResourceData, sizeof(D3D12_SUBRESOURCE_DATA));
			d3dSubResourceData.pData = pData;
			d3dSubResourceData.SlicePitch = d3dSubResourceData.RowPitch = nBytes;
			::UpdateSubresources<1>(pd3dCommandList, pd3dBuffer, *ppd3dUploadBuffer, 0, 0, 1, &d3dSubResourceData);
#endif
			D3D12_RESOURCE_BARRIER d3dResourceBarrier;
			::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
			d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			d3dResourceBarrier.Transition.pResource = pd3dBuffer;
			d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
			d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
		}
		break;
	}
	case D3D12_HEAP_TYPE_UPLOAD:
	{
		d3dResourceStates |= D3D12_RESOURCE_STATE_GENERIC_READ;
		HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceStates, NULL, __uuidof(ID3D12Resource), (void**)&pd3dBuffer);
		if (pData)
		{
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pd3dBuffer->Unmap(0, NULL);
		}
		break;
	}
	case D3D12_HEAP_TYPE_READBACK:
	{
		d3dResourceStates |= D3D12_RESOURCE_STATE_COPY_DEST;
		HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, d3dResourceStates, NULL, __uuidof(ID3D12Resource), (void**)&pd3dBuffer);
		if (pData)
		{
			D3D12_RANGE d3dReadRange = { 0, 0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pd3dBuffer->Unmap(0, NULL);
		}
		break;
	}
	}
	return(pd3dBuffer);
}

ID3D12Resource* CreateBufferResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, ID3D12Resource** ppd3dUploadBuffer)
{
	return(CreateTextureResource(pd3dDevice, pd3dCommandList, pData, nBytes, D3D12_RESOURCE_DIMENSION_BUFFER, nBytes, 1, 1, 1, D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT_UNKNOWN, d3dHeapType, d3dResourceStates, ppd3dUploadBuffer));
}

ID3D12Resource* CreateTextureResourceFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates)
{
	ID3D12Resource* pd3dTexture = NULL;
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> vSubresources;
	DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
	bool bIsCubeMap = false;

	HRESULT hResult = DirectX::LoadDDSTextureFromFileEx(pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &pd3dTexture, ddsData, vSubresources, &ddsAlphaMode, &bIsCubeMap);

	if (NULL != pd3dTexture)
	{
		D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
		::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
		d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
		d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		d3dHeapPropertiesDesc.CreationNodeMask = 1;
		d3dHeapPropertiesDesc.VisibleNodeMask = 1;

		//D3D12_RESOURCE_DESC d3dTextureResourceDesc = pd3dTexture->GetDesc();
		//UINT nSubResources = d3dTextureResourceDesc.DepthOrArraySize * d3dTextureResourceDesc.MipLevels;
		UINT nSubResources = (UINT)vSubresources.size();
		//UINT64 nBytes = 0;
		//pd3dDevice->GetCopyableFootprints(&d3dResourceDesc, 0, nSubResources, 0, NULL, NULL, NULL, &nBytes);
		UINT64 nBytes = GetRequiredIntermediateSize(pd3dTexture, 0, nSubResources);

		D3D12_RESOURCE_DESC d3dUploadResourceDesc;
		::ZeroMemory(&d3dUploadResourceDesc, sizeof(D3D12_RESOURCE_DESC));
		d3dUploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap���� �ؽ��ĸ� ������ �� ����
		d3dUploadResourceDesc.Alignment = 0;
		d3dUploadResourceDesc.Width = nBytes;
		d3dUploadResourceDesc.Height = 1;
		d3dUploadResourceDesc.DepthOrArraySize = 1;
		d3dUploadResourceDesc.MipLevels = 1;
		d3dUploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		d3dUploadResourceDesc.SampleDesc.Count = 1;
		d3dUploadResourceDesc.SampleDesc.Quality = 0;
		d3dUploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		d3dUploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


		pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dUploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

		//UINT nSubResources = (UINT)vSubresources.size();
		//D3D12_SUBRESOURCE_DATA *pd3dSubResourceData = new D3D12_SUBRESOURCE_DATA[nSubResources];
		//for (UINT i = 0; i < nSubResources; i++) pd3dSubResourceData[i] = vSubresources.at(i);

		//	std::vector<D3D12_SUBRESOURCE_DATA>::pointer ptr = &vSubresources[0];

		::UpdateSubresources(pd3dCommandList, pd3dTexture, *ppd3dUploadBuffer, 0, 0, nSubResources, &vSubresources[0]);


		D3D12_RESOURCE_BARRIER d3dResourceBarrier;
		::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
		d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		d3dResourceBarrier.Transition.pResource = pd3dTexture;
		d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
		d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

		//	delete[] pd3dSubResourceData;
	}

	return(pd3dTexture);
}

ID3D12Resource* CreateTexture2DResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue)
{
	ID3D12Resource* pd3dTexture = NULL;

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dTextureResourceDesc;
	::ZeroMemory(&d3dTextureResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dTextureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dTextureResourceDesc.Alignment = 0;
	d3dTextureResourceDesc.Width = nWidth;
	d3dTextureResourceDesc.Height = nHeight;
	d3dTextureResourceDesc.DepthOrArraySize = nElements;
	d3dTextureResourceDesc.MipLevels = nMipLevels;
	d3dTextureResourceDesc.Format = dxgiFormat;
	d3dTextureResourceDesc.SampleDesc.Count = 1;
	d3dTextureResourceDesc.SampleDesc.Quality = 0;
	d3dTextureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dTextureResourceDesc.Flags = d3dResourceFlags;

	HRESULT hResult = pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dTextureResourceDesc, d3dResourceStates, pd3dClearValue, __uuidof(ID3D12Resource), (void**)&pd3dTexture);

	return(pd3dTexture);
}

ID3D12Resource* CreateTextureResourceFromWICFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, ID3D12Resource** ppd3dUploadBuffer, D3D12_RESOURCE_STATES d3dResourceStates)
{
	ID3D12Resource* pd3dTexture = NULL;
	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA d3dSubresource;

	HRESULT hResult = DirectX::LoadWICTextureFromFileEx(pd3dDevice, pszFileName, 0, D3D12_RESOURCE_FLAG_NONE, WIC_LOADER_DEFAULT, &pd3dTexture, decodedData, d3dSubresource);

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	UINT64 nBytes = GetRequiredIntermediateSize(pd3dTexture, 0, 1);

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //Upload Heap���� �ؽ��ĸ� ������ �� ����
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nBytes;
	d3dResourceDesc.Height = 1;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	pd3dDevice->CreateCommittedResource(&d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

	::UpdateSubresources(pd3dCommandList, pd3dTexture, *ppd3dUploadBuffer, 0, 0, 1, &d3dSubresource);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = pd3dTexture;
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	d3dResourceBarrier.Transition.StateAfter = d3dResourceStates;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	return(pd3dTexture);
}


CGameObject** LoadGameObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName, int* pnGameObjects)
{
	FILE* pFile = NULL;

	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);


	char		strEmissionTextureName[64] = { '\0' };
	BYTE	nEmissionTextureStrLength = 0;

	char pstrToken1[64] = { '\0' };
	char pstrToken2[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };
	char pstrFilePath[64] = { '\0' };

	BYTE nStrLength = 0, nObjectNameLength = 0;
	UINT nReads = 0, nMaterials = 0;
	size_t nConverted = 0;

	int		iObjectID = 0;
	int		nTextureNumber = 0;

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken1, sizeof(char), nStrLength, pFile); //"<GameObjects>:"
	nReads = (UINT)::fread(pnGameObjects, sizeof(int), 1, pFile);

	CGameObject** ppGameObjects = new CGameObject * [*pnGameObjects];

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken2, sizeof(char), nStrLength, pFile); //"<Textures>:"
	nReads = (UINT)::fread(&nTextureNumber, sizeof(int), 1, pFile);


	CGameObject* pGameObject = NULL, * pObjectFound = NULL;

	for (int i{}; i < *pnGameObjects; ++i)
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
		pGameObject = new CGameObject(nMaterials);
		strcpy_s(pGameObject->m_pstrName, 64, pstrGameObjectName);

		pGameObject->m_ppMaterials = new CMaterial * [nMaterials];

		for (unsigned m{}; m < nMaterials; ++m)
			pGameObject->m_ppMaterials[m] = NULL;

		CGameObject* pObjectFound = NULL;
		for (int j{}; j < i; ++j)
		{
			if (!strcmp(pstrGameObjectName, ppGameObjects[j]->m_pstrName))
			{
				pObjectFound = ppGameObjects[j];

				pGameObject->SetMesh(0, ppGameObjects[j]->m_ppMeshes[0]);

				for (UINT k{}; k < nMaterials; ++k)
				{
					pGameObject->SetMaterial(k, ppGameObjects[j]->m_ppMaterials[k]);
				}

				break;
			}
		}


		XMFLOAT4 xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		for (UINT k{}; k < nMaterials; ++k)
		{

			if (!pObjectFound) pGameObject->SetMaterial(k, rand() % MAX_Scene_MATERIALS);

			nReads = (UINT)::fread(&xmf4AlbedoColor, sizeof(float), 4, pFile);
			if (!pObjectFound) pGameObject->SetAlbedoColor(k, xmf4AlbedoColor);

			nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
			nReads = (UINT)::fread(pstrToken5, sizeof(char), nStrLength, pFile); //"<<AlbedoTextureName>:"
			if (0 == strcmp(pstrToken5, "<AlbedoTextureName>:"))
			{
				char		strAlbedoTextureName[64] = { '\0' };
				BYTE	nAlbedoTextureStrLength = 0;
				size_t		nConverted = 0;

				nReads = (UINT)::fread(&nAlbedoTextureStrLength, sizeof(BYTE), 1, pFile);
				nReads = (UINT)::fread(strAlbedoTextureName, sizeof(char), nAlbedoTextureStrLength, pFile);


				pGameObject->m_ppMaterials[k] = new CMaterial(1);

				char			pstrFilePath1[64] = { '\0' };
				TCHAR	pwstrTextureName[64] = { '/0' };

				strcpy_s(pstrFilePath1, 64, "Models/Texture/");
				strcpy_s(pstrFilePath1 + 15, 64 - 15, strAlbedoTextureName);
				strcpy_s(pstrFilePath1 + 15 + strlen(strAlbedoTextureName), 64 - 15 - strlen(strAlbedoTextureName), ".dds");
				mbstowcs_s(&nConverted, pGameObject->m_ppMaterials[k]->m_ppstrTextureNames[0], pstrFilePath1, _TRUNCATE);


				//cout << pGameObject->m_pstrName << "	|	" << strAlbedoTextureName << endl;

			}


			nReads = (UINT)::fread(&xmf4EmissionColor, sizeof(float), 4, pFile);
			if (!pObjectFound) pGameObject->SetEmissionColor(k, xmf4EmissionColor);


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

		strcpy_s(pstrFilePath, 64, "Models/");
		strcpy_s(pstrFilePath + 7, 64 - 7, pstrGameObjectName);
		strcpy_s(pstrFilePath + 7 + nObjectNameLength, 64 - 7 - nObjectNameLength, ".bin");
		pGameObject->m_iObjID = iObjectID;
		CMesh* pMesh = new CMesh(pd3dDevice, pd3dCommandList, pstrFilePath, i);


		pGameObject->SetMesh(0, pMesh);


		//cout << "Name: " << pGameObject->m_pstrName << " : "m_pstrTextureName;


		/*printf("Orientation: (%f, %f, %f, %f)\n", pGameObject->m_ppMeshes[0]->OBBox.Orientation.x,
			pGameObject->m_ppMeshes[0]->OBBox.Orientation.y, pGameObject->m_ppMeshes[0]->OBBox.Orientation.z, pGameObject->m_ppMeshes[0]->OBBox.Orientation.w);*/

			//	cout << "\nCenter: " << pGameObject->m_ppMeshes[0]->OBBox.Center.x << ", " << pGameObject->m_ppMeshes[0]->OBBox.Center.y << ", " << pGameObject->m_ppMeshes[0]->OBBox.Center.z <<
			//		"\nExtents: " << pGameObject->m_ppMeshes[0]->OBBox.Extents.x << ", " << pGameObject->m_ppMeshes[0]->OBBox.Extents.y << ", " << pGameObject->m_ppMeshes[0]->OBBox.Extents.z << endl << endl << endl;
			//

		ppGameObjects[i] = pGameObject;
		++iObjectID;
	}


	::fclose(pFile);

	return(ppGameObjects);
}