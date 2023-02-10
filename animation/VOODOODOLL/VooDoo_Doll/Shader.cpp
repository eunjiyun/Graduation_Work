//-----------------------------------------------------------------------------
// File: Shader.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Shader.h"

//23.01.30
#define _WITH_Scene_ROOT_SIGNATURE
//

CShader::CShader()
{
}

CShader::~CShader()
{
	ReleaseShaderVariables();

	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader()
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pd3dErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);
	char *pErrorString = NULL;
	if (pd3dErrorBlob) pErrorString = (char *)pd3dErrorBlob->GetBufferPointer();

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

#define _WITH_WFOPEN
//#define _WITH_STD_STREAM

#ifdef _WITH_STD_STREAM
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#endif

D3D12_SHADER_BYTECODE CShader::ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob)
{
	UINT nReadBytes = 0;
#ifdef _WITH_WFOPEN
	FILE *pFile = NULL;
	::_wfopen_s(&pFile, pszFileName, L"rb");
	::fseek(pFile, 0, SEEK_END);
	int nFileSize = ::ftell(pFile);
	BYTE *pByteCode = new BYTE[nFileSize];
	::rewind(pFile);
	nReadBytes = (UINT)::fread(pByteCode, sizeof(BYTE), nFileSize, pFile);
	::fclose(pFile);
#endif
#ifdef _WITH_STD_STREAM
	std::ifstream ifsFile;
	ifsFile.open(pszFileName, std::ios::in | std::ios::ate | std::ios::binary);
	nReadBytes = (int)ifsFile.tellg();
	BYTE *pByteCode = new BYTE[*pnReadBytes];
	ifsFile.seekg(0);
	ifsFile.read((char *)pByteCode, nReadBytes);
	ifsFile.close();
#endif

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	if (ppd3dShaderBlob)
	{
		*ppd3dShaderBlob = NULL;
		HRESULT hResult = D3DCreateBlob(nReadBytes, ppd3dShaderBlob);
		memcpy((*ppd3dShaderBlob)->GetBufferPointer(), pByteCode, nReadBytes);
		d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
		d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
	}
	else
	{
		d3dShaderByteCode.BytecodeLength = nReadBytes;
		d3dShaderByteCode.pShaderBytecode = pByteCode;
	}

	return(d3dShaderByteCode);
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	//	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = TRUE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

//23.01.30
////void CShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
//void CShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
////
//{
//	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
//	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
//	m_d3dPipelineStateDesc.VS = CreateVertexShader();
//	m_d3dPipelineStateDesc.PS = CreatePixelShader();
//	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
//	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
//	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
//	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
//	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
//	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//	m_d3dPipelineStateDesc.NumRenderTargets = 1;
//	m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//	m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
//	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
//
//	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void **)&m_pd3dPipelineState);
//
//	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
//	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();
//
//	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
//}
void CShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
	//ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;
	//D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;

	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader();
	m_d3dPipelineStateDesc.PS = CreatePixelShader();
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	m_d3dPipelineStateDesc.NumRenderTargets = nRenderTargets;
	for (UINT i = 0; i < nRenderTargets; i++) 
		m_d3dPipelineStateDesc.RTVFormats[i] = (pdxgiRtvFormats) ? pdxgiRtvFormats[i] : DXGI_FORMAT_R8G8B8A8_UNORM;

	m_d3dPipelineStateDesc.DSVFormat = dxgiDsvFormat;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);
	int a = 0;

	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}
//

void CShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList,bool choose, int nPipelineState)
{
	if(true==choose)
		if (m_pd3dGraphicsRootSignature) 
			pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);//23.02.06 오류

	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	//if (true == choose)
	//	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);//23.02.06 오류
		//pd3dCommandList->SetDescriptorHeaps(1, handle);//23.02.06 오류
}

//23.02.06
//void CShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, bool choose,void* pContext)
//
{
	OnPrepareRender(pd3dCommandList,choose);
	//23.02.06
	UpdateShaderVariables(pd3dCommandList, pContext);
	//
}

//23.01.30
void CShader::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	m_d3dSrvCPUDescriptorNextHandle = m_d3dSrvCPUDescriptorStartHandle;
	m_d3dSrvGPUDescriptorNextHandle = m_d3dSrvGPUDescriptorStartHandle;

	cout << "shader create heap" << endl;
	cout << "descriptor heap : " << m_pd3dCbvSrvDescriptorHeap << endl;//사실 힙 주소는 필요없네 핸들에 다 있다
	cout << "CPUDescriptorNextHandle ptr: " << m_d3dSrvCPUDescriptorNextHandle.ptr << endl;
	cout << "GPUDescriptorNextHandle ptr: " << m_d3dSrvGPUDescriptorNextHandle.ptr << endl << endl << endl;

	//원래 있는 값을 참조해서 쓰나?
	//아니면 프로그램에서 자동 생성인가
	//m_pd3dCbvSrvDescriptorHeap 이것만 생성하면 여기서부터 알아서 다른 변수들 값을 셋하니까
}

void CShader::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}

void CShader::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	int nTextures = pTexture->GetTextures();
	UINT nTextureType = pTexture->GetTextureType();
	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource* pShaderResource = pTexture->GetResource(i);
		if (pShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int i = 0; i < nRootParameters; i++) pTexture->SetRootParameterIndex(i, nRootParameterStartIndex + i);
}

void CShader::CreateShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats)
{
	for (int i = 0; i < nResources; i++)
	{
		if (ppd3dResources[i])
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
			d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			d3dShaderResourceViewDesc.Format = pdxgiSrvFormats[i];
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = 1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			pd3dDevice->CreateShaderResourceView(ppd3dResources[i], &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
}
//

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSTerrain", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTerrain", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkyBoxShader::CSkyBoxShader()
{
}

CSkyBoxShader::~CSkyBoxShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkyBox", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSkyBox", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardShader::CStandardShader()
{
}

CStandardShader::~CStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CStandardShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSStandard", "ps_5_1", &m_pd3dPixelShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationStandardShader::CSkinnedAnimationStandardShader()
{
}

CSkinnedAnimationStandardShader::~CSkinnedAnimationStandardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CStandardObjectsShader::CStandardObjectsShader()
{
}

CStandardObjectsShader::~CStandardObjectsShader()
{
}

void CStandardObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
}

void CStandardObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CStandardObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CStandardObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CStandardObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CStandardShader::Render(pd3dCommandList, pCamera,false);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->UpdateTransform(NULL);
			m_ppObjects[j]->Render(pd3dCommandList,  m_pd3dGraphicsRootSignature, m_pd3dPipelineState,pCamera);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHellicopterObjectsShader::CHellicopterObjectsShader()
{
}

CHellicopterObjectsShader::~CHellicopterObjectsShader()
{
}

float Random(float fMin, float fMax)
{
	float fRandomValue = (float)rand();
	if (fRandomValue < fMin) fRandomValue = fMin;
	if (fRandomValue > fMax) fRandomValue = fMax;
	return(fRandomValue);
}

float Random()
{
	return(rand() / float(RAND_MAX));
}

XMFLOAT3 RandomPositionInSphere(XMFLOAT3 xmf3Center, float fRadius, int nColumn, int nColumnSpace)
{
    float fAngle = Random() * 360.0f * (2.0f * 3.14159f / 360.0f);

	XMFLOAT3 xmf3Position;
    xmf3Position.x = xmf3Center.x + fRadius * sin(fAngle);
    xmf3Position.y = xmf3Center.y - (nColumn * float(nColumnSpace) / 2.0f) + (nColumn * nColumnSpace) + Random();
    xmf3Position.z = xmf3Center.z + fRadius * cos(fAngle);

	return(xmf3Position);
}

void CHellicopterObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	m_nObjects = 40;
	m_ppObjects = new CGameObject*[m_nObjects];

	CLoadedModelInfo *pSuperCobraModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/SuperCobra.bin", this);
	CLoadedModelInfo *pGunshipModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Gunship.bin", this);

	int nColumnSpace = 5, nColumnSize = 30;           
    int nFirstPassColumnSize = (m_nObjects % nColumnSize) > 0 ? (nColumnSize - 1) : nColumnSize;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	int nObjects = 0;
    for (int h = 0; h < nFirstPassColumnSize; h++)
    {
        for (int i = 0; i < floor(float(m_nObjects) / float(nColumnSize)); i++)
        {
			if (nObjects % 2)
			{
				m_ppObjects[nObjects] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pSuperCobraModel->m_pModelRootObject, true);
			}
			else
			{
				m_ppObjects[nObjects] = new CGunshipObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pGunshipModel->m_pModelRootObject, true);
			}
			float fHeight = pTerrain->GetHeight(390.0f, 670.0f);
			XMFLOAT3 xmf3Position = RandomPositionInSphere(XMFLOAT3(390.0f, fHeight + 35.0f, 670.0f), Random(20.0f, 100.0f), h - int(floor(nColumnSize / 2.0f)), nColumnSpace);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z) + Random(0.0f, 25.0f);
			m_ppObjects[nObjects]->SetPosition(xmf3Position);
			m_ppObjects[nObjects]->Rotate(0.0f, 90.0f, 0.0f);
			m_ppObjects[nObjects++]->OnPrepareAnimate();
		}
    }

    if (nFirstPassColumnSize != nColumnSize)
    {
        for (int i = 0; i < m_nObjects - int(floor(float(m_nObjects) / float(nColumnSize)) * nFirstPassColumnSize); i++)
        {
			if (nObjects % 2)
			{
				m_ppObjects[nObjects] = new CSuperCobraObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pSuperCobraModel->m_pModelRootObject, true);
			}
			else
			{
				m_ppObjects[nObjects] = new CGunshipObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
				m_ppObjects[nObjects]->SetChild(pGunshipModel->m_pModelRootObject, true);
			}
			m_ppObjects[nObjects]->SetPosition(RandomPositionInSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), Random(20.0f, 100.0f), nColumnSize - int(floor(nColumnSize / 2.0f)), nColumnSpace));
			m_ppObjects[nObjects]->Rotate(0.0f, 90.0f, 0.0f);
			m_ppObjects[nObjects++]->OnPrepareAnimate();
        }
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (pSuperCobraModel) delete pSuperCobraModel;
	if (pGunshipModel) delete pGunshipModel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CSkinnedAnimationObjectsShader::CSkinnedAnimationObjectsShader()
{
}

CSkinnedAnimationObjectsShader::~CSkinnedAnimationObjectsShader()
{
}

void CSkinnedAnimationObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
}

void CSkinnedAnimationObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->Release();
		delete[] m_ppObjects;
	}
}

void CSkinnedAnimationObjectsShader::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
}

void CSkinnedAnimationObjectsShader::ReleaseUploadBuffers()
{
	for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
}

void CSkinnedAnimationObjectsShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, void* pContext)
{
	CSkinnedAnimationStandardShader::Render(pd3dCommandList, pCamera,false);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(m_fElapsedTime);
			m_ppObjects[j]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAngrybotObjectsShader::CAngrybotObjectsShader()
{
}

CAngrybotObjectsShader::~CAngrybotObjectsShader()
{
}

void CAngrybotObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	int xObjects = 3, zObjects = 3, i = 0;

	m_nObjects = (xObjects * 2 + 1) * (zObjects * 2 + 1);

	m_ppObjects = new CGameObject*[m_nObjects];

	float fxPitch = 7.0f * 2.5f;
	float fzPitch = 7.0f * 2.5f;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	CLoadedModelInfo *pAngrybotModel = pModel;
	if (!pAngrybotModel) pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Angrybot.bin", NULL);

	int nObjects = 0;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int z = -zObjects; z <= zObjects; z++)
		{
			m_ppObjects[nObjects] = new CAngrybotObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pAngrybotModel, 1);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, (nObjects % 2));
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackSpeed(0, (nObjects % 2) ? 0.25f : 1.0f);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackPosition(0, (nObjects % 3) ? 0.85f : 0.0f);
			XMFLOAT3 xmf3Position = XMFLOAT3(fxPitch*x + 390.0f, 0.0f, 730.0f + fzPitch * z);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
			m_ppObjects[nObjects]->SetPosition(xmf3Position);
			m_ppObjects[nObjects++]->SetScale(2.0f, 2.0f, 2.0f);
		}
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pAngrybotModel) delete pAngrybotModel;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CEthanObjectsShader::CEthanObjectsShader()
{
}

CEthanObjectsShader::~CEthanObjectsShader()
{
}

void CEthanObjectsShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext)
{
	int xObjects = 3, zObjects = 3, i = 0;

	m_nObjects = (xObjects * 2 + 1) * (zObjects * 2 + 1);

	m_ppObjects = new CGameObject*[m_nObjects];

	float fxPitch = 7.0f * 2.5f;
	float fzPitch = 7.0f * 2.5f;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;

	CLoadedModelInfo *pEthanModel = pModel;
	if (!pEthanModel) pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Ethan.bin", NULL);

	int nObjects = 0;
	for (int x = -xObjects; x <= xObjects; x++)
	{
		for (int z = -zObjects; z <= zObjects; z++)
		{
			m_ppObjects[nObjects] = new CEthanObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pEthanModel, 1);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, (nObjects % 4));
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.25f);
			m_ppObjects[nObjects]->m_pSkinnedAnimationController->SetTrackPosition(0, (nObjects % 10) * 0.35f);
			XMFLOAT3 xmf3Position = XMFLOAT3(fxPitch*x + 290.0f, 0.0f, 750.0f + fzPitch * z);
			xmf3Position.y = pTerrain->GetHeight(xmf3Position.x, xmf3Position.z);
			m_ppObjects[nObjects++]->SetPosition(xmf3Position);
		}
    }

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (!pModel && pEthanModel) delete pEthanModel;
}

//23.01.30
CObjectsShader::CObjectsShader()
{
	//if (m_nObjects > 0)
	//{
	//	/*m_ppd3dTextures = new ID3D12Resource * [m_nTextures];
	//	for (int i = 0; i < m_nTextures; i++)m_ppd3dTextures[i] = NULL;
	//	m_ppd3dTextureUploadBuffers = new ID3D12Resource * [m_nTextures];
	//	for (int i = 0; i < m_nTextures; i++)m_ppd3dTextureUploadBuffers[i] = NULL;*/


	//	m_ppObjects[0]->m_d3dCbvGPUDescriptorHandle = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nObjects];
	//	for (int i = 0; i < m_nObjects; i++)
	//		m_d3dCbvGPUDescriptorHandle[i].ptr = NULL;

	//	m_pnResourceTypes = new UINT[m_nTextures];
	//	for (int i = 0; i < m_nTextures; i++)m_pnResourceTypes[i] = 0;

	//	m_pdxgiBufferFormats = new DXGI_FORMAT[m_nTextures];
	//	for (int i = 0; i < m_nTextures; i++)m_pnResourceTypes[i] = DXGI_FORMAT_UNKNOWN;
	//	m_pnBufferElements = new int[m_nTextures];
	//	for (int i = 0; i < m_nTextures; i++)m_pnBufferElements[i] = 0;
	//}
	//m_nRootParameters = nRootParameters;
	//if (nRootParameters > 0)m_pnRootParameterIndices = new int[nRootParameters];
	//for (int i = 0; i < m_nRootParameters; i++)m_pnRootParameterIndices[i] = -1;

	//m_nSamplers = nSamplers;
	//if (m_nSamplers > 0)m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CObjectsShader::~CObjectsShader()
{
}

void CObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)
{
#ifdef _WITH_Scene_ROOT_SIGNATURE
	m_pd3dGraphicsRootSignature = pd3dGraphicsRootSignature;
	m_pd3dGraphicsRootSignature->AddRef();
#else
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
#endif

	CShader::CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature , nRenderTargets, pdxgiRtvFormats, dxgiDsvFormat);
}

//23.01.13
D3D12_INPUT_LAYOUT_DESC CObjectsShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CObjectsShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSLighting", "vs_5_1", &m_pd3dVertexShaderBlob));
}

//D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
//{
//	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", ppd3dShaderBlob));
//}

D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSLighting", "ps_5_1", &m_pd3dVertexShaderBlob));
}
//


void CObjectsShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void**)&m_pcbMappedGameObjects);
}

void CObjectsShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)//여기서 부터 오브젝트 값이 생기는데요
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	for (int j = 0; j < m_nObjects; j++)
	{
		CB_GAMEOBJECT_INFO* pbMappedcbGameObject = (CB_GAMEOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppObjects[j]->m_xmf4x4World)));
#ifdef _WITH_BATCH_MATERIAL
		if (m_pMaterial) pbMappedcbGameObject->m_nMaterialID = m_pMaterial->m_nReflection;
		if (m_pMaterial) pbMappedcbGameObject->m_nObjectID = j;
#endif
	}
}

void CObjectsShader::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	//23.01.13
	//CIlluminatedTexturedShader::ReleaseShaderVariables();
	CShader::ReleaseShaderVariables();
	//
}

vector<XMFLOAT3> CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvGPUDescriptorHandle, ID3D12DescriptorHeap* heap,
	char* pstrFileName, void* pContext)
{
	int nSceneTextures = 0;

	m_ppObjects = ::LoadGameObjectsFromFile(pd3dDevice, pd3dCommandList, pstrFileName, &m_nObjects);

	//23.02.08
	//m_d3dCbvGPUDescriptorStartHandle = m_d3dCbvGPUDescriptorHandle;
	//m_pd3dCbvSrvDescriptorHeap = heap;

	

	/*m_ppObjects[0]-> m_d3dCbvGPUDescriptorHandle =m_d3dCbvGPUDescriptorHandle;
	m_ppObjects[0]->m_pd3dCbvSrvDescriptorHeap = heap;*/
	//

	//23.01.13
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 1);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);

	

	//23.02.09
	//m_d3dCbvGPUDescriptorHandle.ptr = 9223373419818686205 + 152;
	//

	//23.02.09
	//m_d3dCbvGPUDescriptorStartHandle
	//m_ppObjects[0]-> m_d3dCbvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	//m_ppObjects[0]->m_d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorStartHandle;


	
	


	//cout << "유니티 맵의 디스크립터 핸들 ptr : " << m_ppObjects[0]->m_d3dCbvGPUDescriptorHandle.ptr << endl;
	//

	//23.02.08
	//SetCbvGPUDescriptorHandle(m_d3dCbvGPUDescriptorStartHandle);

	//m_ppObjects[0]->m_d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorStartHandle;
	//m_ppObjects[0]->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;
	//

	//CreateShaderResourceViews(pd3dDevice, pTexture, 0, 5);


	/*CTexture* pTexture = new CTexture(1, RESOURCE_TEXTURE2DARRAY, 0, 1);
	pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/StonesArray.dds", RESOURCE_TEXTURE2DARRAY, 0);
	CreateShaderResourceViews(pd3dDevice, pTexture, 0, 5);*/
	//

	

	int num = 0;

	if (0 == strcmp("Models/Scene.bin", pstrFileName))
	{
		for (int i = 0; i < m_nObjects; ++i)
		{
			//23.02.09
			m_ppObjects[i]->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;
			//

			//canale
			//if (0 == strcmp(m_ppObjects[i]->m_pstrName, "c4_lamp"))
				//if (0 == strcmp(m_ppObjects[i]->m_pstrName, "c4_pillar"))
				//if (0 == strcmp(m_ppObjects[i]->m_pstrName, "canale"))
			if (0 == strcmp(m_ppObjects[i]->m_pstrName, "CubeLamp1")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "CubeLamp2")

				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "StandingLamp2")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "StandingLamp2-1")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "StandingLamp2-2")

				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "StandingLamp1")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "StandingLamp1-1")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "StandingLamp1-2")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "StandingLamp1-3")

				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "CubeLamp3")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "CubeLamp4")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "CubeLamp5")
				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "CubeLamp6")

				|| 0 == strcmp(m_ppObjects[i]->m_pstrName, "CubeLamp2_(1)"))
			{
				tmp = m_ppObjects[i]->GetPosition();
				mpObjVec.push_back(tmp);


			/*	++num;
				cout << "여기 가로등 들어있어요m_ppObjects[i] : " << i << endl;*/
			}

			/*m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.x += m_ppObjects[i]->GetPosition().x;
			m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.y += m_ppObjects[i]->GetPosition().y;
			m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.z += m_ppObjects[i]->GetPosition().z;

			cout <<i<< "번째 m_xmBoundingBox x : " << m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.x << endl;
			cout << i << "번째 m_xmBoundingBox y : " << m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.y << endl;
			cout << i << "번째 m_xmBoundingBox z : " << m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.z << endl;*/

			//23.01.27
			//CreateCbvSrvDescriptorHeaps(pd3dDevice, 2, 0);
			//SetCbvGPUDescriptorHandle(GetGPUCbvDescriptorStartHandle());
			//m_ppObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));//여기
			//m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			//m_ppObjects[i]->SetCbvCPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));//여기


			m_ppObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));//여기
			//여기서 애니메이션 핸들값을 넘겨주나?


			//m_d3dCbvGPUDescriptorHandle m_d3dSrvGPUDescriptorNextHandle
			//m_ppObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dSrvGPUDescriptorNextHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));//여기
			//

			//SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());//0207

			//cout << "obj file name : " << i << " " << m_ppObjects[i]->m_pstrName << endl;

		}
	}
	

	//			++num;
	//			cout << "여기 가로등 들어있어요m_ppObjects[i] : " << i << endl;
	//		}

	//		/*m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.x += m_ppObjects[i]->GetPosition().x;
	//		m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.y += m_ppObjects[i]->GetPosition().y;
	//		m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.z += m_ppObjects[i]->GetPosition().z;

	//		cout <<i<< "번째 m_xmBoundingBox x : " << m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.x << endl;
	//		cout << i << "번째 m_xmBoundingBox y : " << m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.y << endl;
	//		cout << i << "번째 m_xmBoundingBox z : " << m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Center.z << endl;*/

	//	}
	//}
	//cout << "몇 개? " << num << endl;

	//cout << "벡터는 몇 개? " << mpObjVec.size() << endl;


	//tmp._41 = -150.f;//고정
	//tmp._42 = 0.0f;//고정
	//tmp._43 = m_pPlayer->GetPosition().z + 160;


	//m_d3dSrvGPUDescriptorNextHandle
	cout << "유니티 맵의 디스크립터 힙 : " << m_ppObjects[0]->m_pd3dCbvSrvDescriptorHeap << endl;
	cout << "유니티 맵의 디스크립터 힙 : " << m_ppObjects[1]->m_pd3dCbvSrvDescriptorHeap << endl;
	cout << "유니티 맵의 디스크립터 힙 : " << m_ppObjects[2]->m_pd3dCbvSrvDescriptorHeap << endl;
	cout << "유니티 맵 스타트 핸들 : " << m_ppObjects[0]->m_d3dCbvGPUDescriptorHandle.ptr << endl;
	cout<< "유니티 맵 스타트 핸들2 : " << m_d3dCbvGPUDescriptorStartHandle.ptr << endl;



	return mpObjVec;
}

void CObjectsShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) delete m_ppObjects[j];
		delete[] m_ppObjects;
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->Release();
#endif
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}

void CObjectsShader::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++) if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->ReleaseUploadBuffers();
#endif
}

void CObjectsShader::Render2(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, D3D12_GPU_DESCRIPTOR_HANDLE handle)// , void* pContext)
{
	//23.01.11
	/*CIlluminatedTexturedShader::Render(pd3dCommandList, pCamera, pContext);

#ifdef _WITH_BATCH_MATERIAL
	if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);
#endif

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}*/

	CShader::Render(pd3dCommandList, pCamera,true);

	UINT ncbGameObjectBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbGameObjectGpuVirtualAddress = m_pd3dcbGameObjects->GetGPUVirtualAddress();

	//23.02.09	
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);//23.02.06 오류

	if (m_pd3dPipelineState)
		pd3dCommandList->SetPipelineState(m_pd3dPipelineState);

	cout << "셋할 디스크립터힙 주소 : " << m_pd3dCbvSrvDescriptorHeap << endl;

	//m_d3dCbvGPUDescriptorHandle
	if (m_pd3dCbvSrvDescriptorHeap)
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);//23.02.06 오류 //중단점 무시
	//pd3dCommandList->SetDescriptorHeaps(1, handle);//23.02.06 오류

//D3D12_GPU_DESCRIPTOR_HANDLE tmp = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
//cout << "디스크립터 힙 핸들스타트 : " <<tmp<< endl;

	//cout << "set descriptorheap : " << m_pd3dCbvSrvDescriptorHeap << endl;
	//


	//D3D12_GPU_DESCRIPTOR_HANDLE desHandle=m_pd
	for (int j = 0; j < m_nObjects; j++)
	{
		//23.02.09
		//pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_OBJECT, m_ppObjects[j]->m_d3dCbvGPUDescriptorHandle);//주석치면
		//
		//cout << "디스크립터 핸들 ptr : " << m_ppObjects[j]->m_d3dCbvGPUDescriptorHandle.ptr << endl << endl << endl;


		//pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_OBJECT, d3dcbGameObjectGpuVirtualAddress + (ncbGameObjectBytes * j));
		pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_OBJECT, m_ppObjects[j]->m_d3dCbvGPUDescriptorHandle);
		//애니메이션 핸들이 아닌데
		


		//pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_OBJECT, handle);//
		//if (m_pMaterial) m_pMaterial->UpdateShaderVariables(pd3dCommandList);

		//23.02.01
		/*cbuffer cbGameObjectInfo : register(b2)
		{
			matrix					gmtxGameObject : packoffset(c0);
			MATERIAL				gMaterial : packoffset(c4);
			uint					gnTexturesMask : packoffset(c8);
		};

		pd3dCommandList->SetGraphicsRoot32BitConstant(ROOT_PARAMETER_OBJECT, m_ppMaterials[i]->m_nMaterial, 0);
		pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_OBJECT, 4, &m_ppMaterials[i]->m_xmf4AlbedoColor, 4);
		pd3dCommandList->SetGraphicsRoot32BitConstants(ROOT_PARAMETER_OBJECT, 4, &m_ppMaterials[i]->m_xmf4EmissionColor, 8);

		pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);*/
		//

		//m_ppObjects[j]->m_pcbMappedGameObject = (CB_GAMEOBJECT_INFO*)((UINT8*)m_pcbMappedGameObjects + (j * ncbGameObjectBytes));

		if (m_ppObjects[j]) 
			m_ppObjects[j]->Render(pd3dCommandList,  m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera,false);//
	}

	cout << "과연 애니메이션 핸들? " << m_ppObjects[0]->m_d3dCbvGPUDescriptorHandle.ptr << endl << endl;
	//
}
//
