//-----------------------------------------------------------------------------
// File: Stage.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

#define MAX_LIGHTS						16 

//23.02.06
//#define MAX_LIGHTSS						76
//

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};										
										
struct LIGHTS							
{										
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

////23.02.02
//struct LIGHTSS
//{
//	LIGHT					m_pLights[MAX_LIGHTSS];
//	XMFLOAT4				m_xmf4GlobalAmbient;
//};
////

//23.01.30
struct MATERIAL
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4				m_xmf4Emissive;
};

struct MATERIALS
{
	//23.01.13
	//MATERIAL				m_pReflections[MAX_MATERIALS];
	MATERIAL				m_pReflections[MAX_Scene_MATERIALS];
	//
};
//

class CStage
{
public:
    CStage();
    ~CStage();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();

	//23.01.30
	/*void CMaterial::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature
		, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat)*/
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	//void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
	//	, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);
	//
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void ReleaseUploadBuffers();

	//23.01.30
	void BuildLightsAndMaterials();
	//

	//23.02.07
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	//

	void UpdateBoundingBox();
	void CheckObjectByObjectCollisions();

	CPlayer								*m_pPlayer = NULL;

protected:
	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap			*m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;

public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	static void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNextHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	float								m_fElapsedTime = 0.0f;

	int									m_nGameObjects = 0;
	CGameObject				**m_ppGameObjects = NULL;

	int									m_nHierarchicalGameObjects = 0;
	CGameObject							**m_ppHierarchicalGameObjects = NULL;

	XMFLOAT3							m_xmf3RotatePosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	int									m_nShaders = 0;
	CShader								**m_ppShaders = NULL;

	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain		*m_pTerrain = NULL;

	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	//23.01.30
	//LIGHTSS*				m_pLights2 = NULL;
	MATERIALS*			m_pMaterials = NULL;
	//
	//23.02.01
	ID3D12Resource*		m_pd3dcbMaterials = NULL;
	MATERIAL*				m_pcbMappedMaterials = NULL;

	CShader**					m_ppShaders2 = NULL;
	int								m_nShaders2 = 0;

	ID3D12PipelineState* m_pd3dPipelineState = NULL;
	//

	//23.02.07
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvGPUDescriptorHandle;
	//


	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;

	////23.02.06
	//ID3D12Resource* m_pd3dcbLights2 = NULL;
	//LIGHTSS* m_pcbMappedLights2 = NULL;
	////

	//23.01.05
public:
	vector<XMFLOAT3> mpObjVec;
	XMFLOAT3 tmp;
	//

	//23.01.17
	float						mpTime = 0.f;
	//

	//23.02.12
	bool wakeUp = false;
	//

	BoundingBox			m_xmOOBB = BoundingBox();
	XMFLOAT4X4					m_xmf4x4World = Matrix4x4::Identity();
};
