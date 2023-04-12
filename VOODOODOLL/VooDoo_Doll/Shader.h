//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"
#include "Monster.h"

//23.01.13
extern CGameObject** LoadGameObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName, int* pnGameObjects, int* pnSceneTextures);
//

class CShader
{
public:
	CShader();
	CShader(const CShader& other);
	virtual ~CShader();

private:
	int									m_nReferences = 0;
protected:
	ID3DBlob* m_pd3dVertexShaderBlob = NULL;
	ID3DBlob* m_pd3dPixelShaderBlob = NULL;//

	ID3D12PipelineState* m_pd3dPipelineState = NULL;//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;//
	float								m_fElapsedTime = 0.0f;//

public:
	CGameObject** m_ppObjects = 0;//
	int								m_nObjects = 0;//

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;//
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;//

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCPUDescriptorStartHandle;//
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorStartHandle;//
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;//
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;//

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorNextHandle;//
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorNextHandle;//

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR* pszFileName, ID3DBlob** ppd3dShaderBlob = NULL);
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext = NULL) {}
	virtual void ReleaseShaderVariables() { }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World) { }

	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext = NULL);
	virtual void ReleaseUploadBuffers() { }

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }
	void CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	void CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources, DXGI_FORMAT* pdxgiSrvFormats);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardShader : public CShader
{
public:
	CStandardShader();
	virtual ~CStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardObjectsShader : public CStandardShader
{
public:
	CStandardObjectsShader();
	virtual ~CStandardObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

protected:
	CGameObject** m_ppObjects = 0;
	int								m_nObjects = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkinnedAnimationStandardShader : public CStandardShader
{
public:
	CSkinnedAnimationStandardShader();
	virtual ~CSkinnedAnimationStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkinnedAnimationObjectsShader : public CSkinnedAnimationStandardShader
{
public:
	CSkinnedAnimationObjectsShader();
	virtual ~CSkinnedAnimationObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, void* pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext = NULL);

protected:
	CGameObject** m_ppObjects = 0;
	int								m_nObjects = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CAngrybotObjectsShader : public CSkinnedAnimationObjectsShader
{
public:
	CAngrybotObjectsShader();
	virtual ~CAngrybotObjectsShader();

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel,int choosePl, void* pContext = NULL);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CObjectsShader : public CShader
{
public:
	CObjectsShader();
	virtual ~CObjectsShader();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);
	virtual vector<XMFLOAT3> BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvGPUDescriptorHandle, ID3D12DescriptorHeap* heap,
		char* pstrFileName, void* pContext = NULL);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, void* pContext = NULL);

public:
	vector<XMFLOAT3> mpObjVec;
	XMFLOAT3 tmp;
protected:
	ID3D12Resource* m_pd3dcbGameObjects = NULL;
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObjects = NULL;

#ifdef _WITH_BATCH_MATERIAL
	CMaterial* m_pMaterial = NULL;
#endif
};
