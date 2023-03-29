//------------------------------------------------------- ----------------------
// File: Object.hDIR_COLLECT
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_RUN					0x10
#define DIR_JUMP				0x20
#define DIR_ATTACK				0x40
#define DIR_COLLECT				0x80
#define DIR_CHANGESTATE			0x100
#define DIR_DIE					0x200

class CShader;
class CStandardShader;

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
	UINT							m_nObjectID;
	UINT							m_nMaterialID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

class CTexture
{
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters);
	CTexture(const CTexture& other);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;//

	UINT							m_nTextureType;//

	int								m_nTextures = 0;//
	ID3D12Resource** m_ppd3dTextures = NULL;//
	ID3D12Resource** m_ppd3dTextureUploadBuffers;//

	UINT* m_pnResourceTypes = NULL;//

	DXGI_FORMAT* m_pdxgiBufferFormats = NULL;//
	int* m_pnBufferElements = NULL;//

	int								m_nRootParameters = 0;//
	UINT* m_pnRootParameterIndices = NULL;//
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSrvGpuDescriptorHandles = NULL;//

	int								m_nSamplers = 0;//
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;//

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
	void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex);
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);

	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);

	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	ID3D12Resource* GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }

	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP				0x01
#define MATERIAL_SPECULAR_MAP			0x02
#define MATERIAL_NORMAL_MAP				0x04
#define MATERIAL_METALLIC_MAP			0x08
#define MATERIAL_EMISSION_MAP			0x10
#define MATERIAL_DETAIL_ALBEDO_MAP		0x20
#define MATERIAL_DETAIL_NORMAL_MAP		0x40

class CGameObject;

class CMaterial
{
public:
	CMaterial(int nTextures);
	CMaterial(const CMaterial& other);
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;//

public:
	CShader* m_pShader = NULL;//
	static CShader* m_pStandardShader;//
	static CShader* m_pSkinnedAnimationShader;//
	CTexture** m_ppTextures = NULL; //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal //

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);//
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);//
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);//

public:
	UINT							m_nType = 0x00;//

	float							m_fGlossiness = 0.0f;//
	float							m_fSmoothness = 0.0f;//
	float							m_fSpecularHighlight = 0.0f;//
	float							m_fMetallic = 0.0f;//
	float							m_fGlossyReflection = 0.0f;//

public:
	int 							m_nTextures = 0;//
	_TCHAR(*m_ppstrTextureNames)[64] = NULL;
	int								m_nMaterial = 1; //Material Index, CScene::m_pReflections[] //

public:


	XMFLOAT4						m_xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	void SetShader(CShader* pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture* pTexture, UINT nTexture = 0);

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual void ReleaseUploadBuffers();

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nType, UINT nRootParameter,
		_TCHAR* pwstrTextureName, CTexture** ppTexture, CGameObject* pParent, FILE* pInFile, CShader* pShader, int choose, int whatTexture);

	static void PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature
		, UINT nRenderTargets, DXGI_FORMAT* pdxgiRtvFormats, DXGI_FORMAT dxgiDsvFormat);

	void SetStandardShader() { CMaterial::SetShader(m_pStandardShader); }
	void SetSkinnedAnimationShader() { CMaterial::SetShader(m_pSkinnedAnimationShader); }

	//23.01.30
	void SetAlbedoColor(XMFLOAT4 xmf4Color) { m_xmf4AlbedoColor = xmf4Color; }
	void SetEmissionColor(XMFLOAT4 xmf4Color) { m_xmf4EmissionColor = xmf4Color; }
	//
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct CALLBACKKEY
{
	float  							m_fTime = 0.0f;
	void* m_pCallbackData = NULL;
};

#define _WITH_ANIMATION_INTERPOLATION

class CAnimationCallbackHandler
{
public:
	CAnimationCallbackHandler() { }
	CAnimationCallbackHandler(const CAnimationCallbackHandler& other) {}

	~CAnimationCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition) { }
};

//#define _WITH_ANIMATION_SRT

class CAnimationSet
{
public:
	CAnimationSet(float fLength, int nFramesPerSecond, int nKeyFrameTransforms, int nSkinningBones, char* pstrName);
	CAnimationSet(const CAnimationSet& other);
	~CAnimationSet();

public:
	char							m_pstrAnimationSetName[64];

	float							m_fLength = 0.0f;
	int								m_nFramesPerSecond = 0; //m_fTicksPerSecond //

	int								m_nKeyFrames = 0; //
	int m_nAnimatedBones = 0;
	float* m_pfKeyFrameTimes = NULL; //
	XMFLOAT4X4** m_ppxmf4x4KeyFrameTransforms = NULL; //

#ifdef _WITH_ANIMATION_SRT
	int								m_nKeyFrameScales = 0;
	float* m_pfKeyFrameScaleTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameScales = NULL;
	int								m_nKeyFrameRotations = 0;
	float* m_pfKeyFrameRotationTimes = NULL;
	XMFLOAT4** m_ppxmf4KeyFrameRotations = NULL;
	int								m_nKeyFrameTranslations = 0;
	float* m_pfKeyFrameTranslationTimes = NULL;
	XMFLOAT3** m_ppxmf3KeyFrameTranslations = NULL;
#endif

public:
	XMFLOAT4X4 GetSRT(int nBone, float fPosition);
};

class CAnimationSets
{
public:
	CAnimationSets(int nAnimationSets);
	CAnimationSets(const CAnimationSets& other);
	~CAnimationSets();

private:
	int								m_nReferences = 0;//

public:
	int								m_nAnimationSets = 0;//
	CAnimationSet** m_pAnimationSets = NULL;//

	int								m_nAnimatedBoneFrames = 0;//
	CGameObject** m_ppAnimatedBoneFrameCaches = NULL; //[m_nAnimatedBoneFrames] //0308 m_ppAnimatedBoneFrameCaches[j]°¡ null

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
};

class CAnimationTrack
{
public:
	CAnimationTrack() {}
	CAnimationTrack(const CAnimationTrack& other);
	~CAnimationTrack();
	
public:
	BOOL 							m_bEnable = true;//
	float 							m_fSpeed = 1.0f;//
	float 							m_fPosition = -ANIMATION_CALLBACK_EPSILON;//
	float 							m_fWeight = 1.0f;//

	int 							m_nAnimationSet = 0;//

	int 							m_nType = ANIMATION_TYPE_LOOP; //Once, Loop, PingPong //

	int 							m_nCallbackKeys = 0;//
	CALLBACKKEY* m_pCallbackKeys = NULL;

	CAnimationCallbackHandler* m_pAnimationCallbackHandler = NULL;//

public:
	void SetAnimationSet(int nAnimationSet) { m_nAnimationSet = nAnimationSet; }

	void SetEnable(bool bEnable) { m_bEnable = bEnable; }
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	void SetWeight(float fWeight) { m_fWeight = fWeight; }

	void SetPosition(float fPosition) { m_fPosition = fPosition; }
	float UpdatePosition(float fTrackPosition, float fTrackElapsedTime, float fAnimationLength);

	void SetCallbackKeys(int nCallbackKeys);
	void SetCallbackKey(int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(CAnimationCallbackHandler* pCallbackHandler);

	void HandleCallback();
};

class CLoadedModelInfo
{
public:
	CLoadedModelInfo() {}
	CLoadedModelInfo(const CLoadedModelInfo& other);
	~CLoadedModelInfo();

	CAnimationSets* m_pAnimationSets = NULL;
	CGameObject* m_pModelRootObject = NULL;

	int 							m_nSkinnedMeshes = 0;
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache

public:
	void PrepareSkinning();
};

class CAnimationController
{
public:
	CAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	CAnimationController(const CAnimationController& other);
	~CAnimationController();

public:
	float 							m_fTime = 0.0f;//

	short Cur_Animation_Track = 0;//

	int 							m_nAnimationTracks = 0;//
	CAnimationTrack* m_pAnimationTracks = NULL;//

	CAnimationSets* m_pAnimationSets = NULL;//

	int 							m_nSkinnedMeshes = 0;//
	CSkinnedMesh** m_ppSkinnedMeshes = NULL; //[SkinnedMeshes], Skinned Mesh Cache //

	ID3D12Resource** m_ppd3dcbSkinningBoneTransforms = NULL; //[SkinnedMeshes] //
	XMFLOAT4X4** m_ppcbxmf4x4MappedSkinningBoneTransforms = NULL; //[SkinnedMeshes] //
public:
	bool							m_bRootMotion = false;//
	CGameObject* m_pModelRootObject = NULL;//

	CGameObject* m_pRootMotionObject = NULL;//
	XMFLOAT3						m_xmf3FirstRootMotionPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);//

public:
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);

	void SetTrackEnable(int nAnimationTrack, bool bEnable);
	void SetTrackPosition(int nAnimationTrack, float fPosition);
	void SetTrackSpeed(int nAnimationTrack, float fSpeed);
	void SetTrackWeight(int nAnimationTrack, float fWeight);

	void SetCallbackKeys(int nAnimationTrack, int nCallbackKeys);
	void SetCallbackKey(int nAnimationTrack, int nKeyIndex, float fTime, void* pData);
	void SetAnimationCallbackHandler(int nAnimationTrack, CAnimationCallbackHandler* pCallbackHandler);

	void AdvanceTime(float fElapsedTime, short curTrack, CGameObject* pRootGameObject);

	void SetRootMotion(bool bRootMotion) { m_bRootMotion = bRootMotion; }

	virtual void OnRootMotion(CGameObject* pRootGameObject) { }
	virtual void OnAnimationIK(CGameObject* pRootGameObject) { }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
public:
	CGameObject();
	CGameObject(int nMaterials);
	CGameObject(const CGameObject& other);
	virtual ~CGameObject();

public:
	int								m_nReferences = 0;// //

public:
	char							m_pstrName[64] = { '\0' };// //
	CMesh** m_ppMeshes;// //
	int								m_nMeshes;//
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dCbvGPUDescriptorHandle;//
	char							m_pstrFrameName[64];//
	int								m_nMaterials = 0;//
	CMaterial** m_ppMaterials = NULL;//

	XMFLOAT4X4						m_xmf4x4ToParent;//
	XMFLOAT4X4						m_xmf4x4World;//

	CGameObject* m_pParent = NULL;//
	CGameObject* m_pChild = NULL;//
	CGameObject* m_pSibling = NULL;//

	CAnimationController* m_pSkinnedAnimationController = NULL;//

	ID3D12Resource* m_pd3dcbGameObject = NULL;// //
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObject = NULL;//

	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = NULL;//
	BoundingBox			m_xmOOBB = BoundingBox();// //
	BoundingOrientedBox			obBox = BoundingOrientedBox();


	bool						m_bActive = false;//
	bool onAttack = false;//
	bool onRun = false;//
	bool onDie = false;
	bool onCollect = false;//
	bool onAct = false;
	bool dieOnce = false;//
	bool onFloor = false;
	int plWhat = 1;

	int c_id = -1;//monster id //
	int npc_type = -1;//monster type //

	bool isChild = false;

public:
	XMFLOAT3					m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);//
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);//
	XMFLOAT3					m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);//
	CGameObject* m_pLockedObject = NULL;//
	float						m_fMovingSpeed = 0.0f;//
	float						m_fRotationSpeed = 0.0f;//

public:
	void AddRef();
	void Release();

	void SetMesh(int nIndex, CMesh* pMesh);
	void SetShader(CShader* pShader);
	void SetShader(int nMaterial, CShader* pShader);
	void SetMaterial(int nMaterial, CMaterial* pMaterial);

	void SetMaterial(int nIndex, int nReflection);
	void SetAlbedoColor(int nIndex, XMFLOAT4 xmf4Color);
	void SetEmissionColor(int nIndex, XMFLOAT4 xmf4Color);
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

	virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

	virtual void OnPrepareAnimate() { }
	virtual void Animate(float fTimeElapsed,bool onPlayer);

	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, ID3D12PipelineState* m_pd3dPipelineState, CCamera* pCamera = NULL);
	void lightRender(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, ID3D12PipelineState* m_pd3dPipelineState, CCamera* pCamera = NULL);
	void onPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, ID3D12PipelineState* m_pd3dPipelineState);
	virtual void OnLateUpdate() { }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetLook(XMFLOAT3 _in);
	void SetUp(XMFLOAT3 _in);
	void SetRight(XMFLOAT3 _in);

	XMFLOAT3 GetToParentPosition();
	void Move(XMFLOAT3 xmf3Offset);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	void	Boundingbox_Transform();

	CGameObject* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	CGameObject* FindFrame(char* pstrFrameName);

	CTexture* FindReplicatedTexture(_TCHAR* pstrTextureName);

	UINT GetMeshType() { return((m_ppMeshes) ? m_ppMeshes[0]->GetType() : 0x00); }

public:
	CSkinnedMesh* FindSkinnedMesh(char* pstrSkinnedMeshName);
	void FindAndSetSkinnedMesh(CSkinnedMesh** ppSkinnedMeshes, int* pnSkinnedMesh);

	void SetTrackAnimationSet(int nAnimationTrack, int nAnimationSet);
	void SetTrackAnimationPosition(int nAnimationTrack, float fPosition);

	void SetRootMotion(bool bRootMotion) { if (m_pSkinnedAnimationController) m_pSkinnedAnimationController->SetRootMotion(bRootMotion); }

	void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader, int choose);

	static void LoadAnimationFromFile(FILE* pInFile, CLoadedModelInfo* pLoadedModel);
	static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature,
		CGameObject* pParent, FILE* pInFile, CShader* pShader, int* pnSkinnedMeshes, int choose);

	static CLoadedModelInfo* LoadGeometryAndAnimationFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader, int choose);

	static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);

	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection);
	void SetActive(bool bActive) { m_bActive = bActive; }
	void SetRotationAxis(XMFLOAT3& xmf3RotationAxis);
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	virtual void Reset() {}

	void SetFirePosition(XMFLOAT3 xmf3FirePosition)
	{
		m_xmf3FirePosition = xmf3FirePosition;
		SetPosition(xmf3FirePosition);
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CAngrybotAnimationController : public CAnimationController
{
public:
	CAngrybotAnimationController(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int nAnimationTracks, CLoadedModelInfo* pModel);
	~CAngrybotAnimationController();

	virtual void OnRootMotion(CGameObject* pRootGameObject);
};

class CAngrybotObject : public CGameObject
{
public:
	CAngrybotObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CAngrybotObject();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CRootMotionCallbackHandler : public CAnimationCallbackHandler
{
public:
	CRootMotionCallbackHandler() { }
	~CRootMotionCallbackHandler() { }

public:
	virtual void HandleCallback(void* pCallbackData, float fTrackPosition);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//23.02.21
class CBulletObject : public CGameObject
{
public:
	CBulletObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks,int chooseObj);
	virtual ~CBulletObject() {}

public:
	virtual void Animate(float fElapsedTime);

	float						m_fBulletEffectiveRange = 150.0f;
	float						m_fMovingDistance = 0.0f;
	int num = 0;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 4.0f;
	CGameObject* m_pLockedObject = NULL;

	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	virtual void Reset();
};
//
