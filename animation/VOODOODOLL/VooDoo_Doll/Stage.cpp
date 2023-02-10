//-----------------------------------------------------------------------------
// File: CStage.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Stage.h"

ID3D12DescriptorHeap *CStage::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CStage::m_d3dSrvGPUDescriptorNextHandle;

CStage::CStage()
{
}

CStage::~CStage()
{
}

void CStage::BuildDefaultLightsAndMaterials()
{
	m_nLights = 5;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 300.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.3f, 0.8f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(230.0f, 330.0f, 480.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	m_pLights[3].m_bEnable = true;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(550.0f, 330.0f, 530.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	m_pLights[4].m_bEnable = true;
	m_pLights[4].m_nType = POINT_LIGHT;
	m_pLights[4].m_fRange = 200.0f;
	m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
	m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
	m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
}


void CStage::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	//23.02.05
	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 76); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()
	//CreateCbvSrvDescriptorHeaps(pd3dDevice, m_ppShaders2[0]->m_nObjects, 77); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()

	DXGI_FORMAT pdxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	//


	//23.02.07
	//SetCbvGPUDescriptorHandle(m_d3dCbvGPUDescriptorStartHandle);

	//m_ppObjects[0]->m_d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorStartHandle;
	//

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature , 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);

	BuildDefaultLightsAndMaterials();//인형이 까맣게 출력

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);//스테이지 핸들에 엑세스
	//23.02.08
	m_pSkyBox->m_d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorStartHandle;//오브젝트 렌더에서 셋 디스크립터를 해야해서 변수 전달은 필요합니다
	m_pSkyBox->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;
	//

	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.3f, 0.0f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		//5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT,
		_T("Terrain/HeightMap.raw"), 257, 257, xmf3Scale, xmf4Color);

	m_nHierarchicalGameObjects = 20;
	m_ppHierarchicalGameObjects = new CGameObject*[m_nHierarchicalGameObjects];

	CLoadedModelInfo *pAngrybotModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, 
		m_pd3dGraphicsRootSignature, "Model/Angrybot.bin", NULL);
	m_ppHierarchicalGameObjects[0] = new CAngrybotObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pAngrybotModel, 1);
	m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[0]->SetPosition(410.0f, m_pTerrain->GetHeight(410.0f, 735.0f), 735.0f);
	m_ppHierarchicalGameObjects[0]->SetScale(10.0f, 10.0f, 10.0f);
	if (pAngrybotModel) delete pAngrybotModel;

	CLoadedModelInfo *pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Monster.bin", NULL);
	m_ppHierarchicalGameObjects[1] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
	m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[1]->SetPosition(430.0f, m_pTerrain->GetHeight(430.0f, 700.0f), 700.0f);
	m_ppHierarchicalGameObjects[1]->SetScale(3.0f, 3.0f, 3.0f);
	m_ppHierarchicalGameObjects[2] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
	m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[2]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 720.0f), 720.0f);
	m_ppHierarchicalGameObjects[2]->SetScale(3.0f, 3.0f, 3.0f);
	m_ppHierarchicalGameObjects[3] = new CMonsterObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pMonsterModel, 1);
	m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[3]->SetPosition(380.0f, m_pTerrain->GetHeight(380.0f, 750.0f), 750.0f);
	m_ppHierarchicalGameObjects[3]->SetScale(3.0f, 3.0f, 3.0f);
	if (pMonsterModel) delete pMonsterModel;

	CLoadedModelInfo *pHumanoidModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Humanoid.bin", NULL);
	m_ppHierarchicalGameObjects[4] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanoidModel, 1);
	m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[4]->SetPosition(400.0f, m_pTerrain->GetHeight(400.0f, 670.0f), 670.0f);
	m_ppHierarchicalGameObjects[4]->Rotate(0.0f, 180.0f, 0.0f);
	m_ppHierarchicalGameObjects[4]->SetScale(5.0f, 5.0f, 5.0f);

	m_ppHierarchicalGameObjects[5] = new CHumanoidObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pHumanoidModel, 1);
	m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[5]->SetPosition(410.0f, m_pTerrain->GetHeight(410.0f, 660.0f), 660.0f);
	m_ppHierarchicalGameObjects[5]->SetScale(5.0f, 5.0f, 5.0f);
	if (pHumanoidModel) delete pHumanoidModel;

	CLoadedModelInfo *pEthanModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Ethan.bin", NULL);
	m_ppHierarchicalGameObjects[6] = new CEthanObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, 1);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
///*
	float* pfData = new float[2];
	pfData[0] = 0.0f;
	pfData[1] = 1.0f;

	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKeys(0, 2);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKey(0, 0, 0.0f, &pfData[0]);
	CAnimationSet* pAnimationSet = m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[1];
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetCallbackKey(0, 1, pAnimationSet->m_fLength, &pfData[1]);

	CRootMotionCallbackHandler* pRootMotionCallbackHandler = new CRootMotionCallbackHandler();
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pRootMotionCallbackHandler);
//*/
	m_ppHierarchicalGameObjects[6]->SetRootMotion(true);
	m_ppHierarchicalGameObjects[6]->SetPosition(350.0f, m_pTerrain->GetHeight(350.0f, 670.0f), 670.0f);
	m_ppHierarchicalGameObjects[6]->Rotate(0.0f, -90.0f, 0.0f);
	m_ppHierarchicalGameObjects[6]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.75f);

	m_ppHierarchicalGameObjects[14] = new CEthanObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, 2);
	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackWeight(0, 0.85f);
	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackSpeed(0, 0.5f);
	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackWeight(1, 0.15f);
	m_ppHierarchicalGameObjects[14]->m_pSkinnedAnimationController->SetTrackSpeed(1, 0.025f);
	m_ppHierarchicalGameObjects[14]->SetPosition(380.0f, m_pTerrain->GetHeight(380.0f, 680.0f), 680.0f);

	CLoadedModelInfo *pZebraModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Voodoo1.bin", NULL);
	m_ppHierarchicalGameObjects[7] = new CZebraObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel, 1,1);//칼든 애
	m_ppHierarchicalGameObjects[7]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[7]->SetPosition(280.0f, m_pTerrain->GetHeight(280.0f, 620.0f), 620.0f);
	m_ppHierarchicalGameObjects[7]->SetScale(1.1f, 1.1f, 1.1f);
	if (pZebraModel) delete pZebraModel;

	//23.02.05
	CLoadedModelInfo* pZebraModel2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Voodoo2.bin", NULL);
	m_ppHierarchicalGameObjects[15] = new CZebraObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel2, 1,2);//뼈다귀 다리
	m_ppHierarchicalGameObjects[15]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[15]->SetPosition(310.0f, m_pTerrain->GetHeight(310.0f, 700.0f), 700.0f);//
	m_ppHierarchicalGameObjects[15]->SetScale(0.51f, 0.51f, 0.51f);
	if (pZebraModel2) delete pZebraModel2;

	CLoadedModelInfo* pZebraModel3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Voodoo3.bin", NULL);//
	m_ppHierarchicalGameObjects[16] = new CZebraObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel3, 1,3);//귀신
	m_ppHierarchicalGameObjects[16]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[16]->SetPosition(237.0f, m_pTerrain->GetHeight(237.0f, 667.0f), 667.0f);//
	m_ppHierarchicalGameObjects[16]->SetScale(0.31f, 0.31f, 0.31f);
	if (pZebraModel3) delete pZebraModel3;

	CLoadedModelInfo* pZebraModel4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Voodoo4.bin", NULL);//
	m_ppHierarchicalGameObjects[17] = new CZebraObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel4, 1,4);//펜싱 칼 든 애
	m_ppHierarchicalGameObjects[17]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[17]->SetPosition(226.0f, m_pTerrain->GetHeight(226.0f, 662.0f)-150, 662.0f);//
	m_ppHierarchicalGameObjects[17]->SetScale(2.1f, 2.1f, 2.1f);
	if (pZebraModel4) delete pZebraModel4;

	CLoadedModelInfo* pZebraModel5 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Voodoo5.bin", NULL);
	m_ppHierarchicalGameObjects[18] = new CZebraObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel5, 1,5);//무릎 꿇는 마법사
	m_ppHierarchicalGameObjects[18]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[18]->SetPosition(361.0f, m_pTerrain->GetHeight(361.0f, 641.0f), 641.0f);//
	m_ppHierarchicalGameObjects[18]->SetScale(0.51f, 0.51f, 0.51f);
	if (pZebraModel5) delete pZebraModel5;

	CLoadedModelInfo* pZebraModel6 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Voodoo6.bin", NULL);//
	m_ppHierarchicalGameObjects[19] = new CZebraObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pZebraModel6, 1,6);
	m_ppHierarchicalGameObjects[19]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[19]->SetPosition(251.0f, m_pTerrain->GetHeight(251.0f, 698.0f)-150, 398.0f);//머리에 바늘있는 비실한 애
	m_ppHierarchicalGameObjects[19]->SetScale(2.1f, 2.1f, 2.1f);
	if (pZebraModel6) delete pZebraModel6;
	//


	CLoadedModelInfo *pLionModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Lion.bin", NULL);
	m_ppHierarchicalGameObjects[8] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[8]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[8]->SetPosition(300.0f, m_pTerrain->GetHeight(280.0f, 640.0f), 630.0f);
	m_ppHierarchicalGameObjects[8]->SetScale(1.0f, 1.0f, 1.0f);
	m_ppHierarchicalGameObjects[9] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[9]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 1);
	m_ppHierarchicalGameObjects[9]->SetPosition(310.0f, m_pTerrain->GetHeight(310.0f, 630.0f), 630.0f);
	m_ppHierarchicalGameObjects[9]->SetScale(1.0f, 1.0f, 1.0f);
	m_ppHierarchicalGameObjects[10] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[10]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 2);
	m_ppHierarchicalGameObjects[10]->SetPosition(250.0f, m_pTerrain->GetHeight(250.0f, 600.0f), 600.0f);
	m_ppHierarchicalGameObjects[10]->SetScale(1.0f, 1.0f, 1.0f);
	m_ppHierarchicalGameObjects[11] = new CLionObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pLionModel, 1);
	m_ppHierarchicalGameObjects[11]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 3);
	m_ppHierarchicalGameObjects[11]->SetPosition(270.0f, m_pTerrain->GetHeight(270.0f, 620.0f), 620.0f);
	m_ppHierarchicalGameObjects[11]->SetScale(1.0f, 1.0f, 1.0f);
	m_xmf3RotatePosition = m_ppHierarchicalGameObjects[11]->GetPosition();

	if (pLionModel) delete pLionModel;

	CLoadedModelInfo *pEagleModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Eagle.bin", NULL);
	m_ppHierarchicalGameObjects[12] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
	m_ppHierarchicalGameObjects[12]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[12]->SetRootMotion(true);
	m_ppHierarchicalGameObjects[12]->SetPosition(330.0f, m_pTerrain->GetHeight(330.0f, 590.0f) + 20.0f, 590.0f);
	m_ppHierarchicalGameObjects[13] = new CEagleObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEagleModel, 1);
	m_ppHierarchicalGameObjects[13]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppHierarchicalGameObjects[13]->SetRootMotion(true);
	//23.02.05
	m_ppHierarchicalGameObjects[13]->SetPosition(330.0f, m_pTerrain->GetHeight(330.0f, 750.0f) + 25.0f, 750.0f);
	//m_ppHierarchicalGameObjects[13]->SetPosition(330.0f, m_pTerrain->GetHeight(330.0f, 750.0f) , 750.0f);
	//
	m_ppHierarchicalGameObjects[13]->Rotate(0.0f, 180.0f, 0.0f);

	if (pEagleModel) delete pEagleModel;

	//23.02.08
	m_ppHierarchicalGameObjects[0]->m_d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorStartHandle;
	m_ppHierarchicalGameObjects[0]->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;//여기서 디스크립터힙 주소 넘겨줍니다
	//

///*
	m_nShaders = 1;
	m_ppShaders = new CShader*[m_nShaders];

	CEthanObjectsShader *pEthanObjectsShader = new CEthanObjectsShader();
	pEthanObjectsShader->BuildObjects(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, pEthanModel, m_pTerrain);

	m_ppShaders[0] = pEthanObjectsShader;
//*/
	if (pEthanModel) delete pEthanModel;

	//23.01.30
	m_nShaders2 = 1;
	m_ppShaders2 = new CShader * [m_nShaders2];

	CObjectsShader* pObjectShader = new CObjectsShader();


	//23.02.03
	//DXGI_FORMAT_D24_UNORM_S8_UINT
	//DXGI_FORMAT pdxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	pObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);//
	//pObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 5, pdxgiRtvFormats, DXGI_FORMAT_D24_UNORM_S8_UINT);
	//

	//여기
	mpObjVec = pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, m_d3dCbvGPUDescriptorStartHandle, m_pd3dCbvSrvDescriptorHeap, "Models/Scene.bin");

	cout << "mpObjVec[0] .x : " << mpObjVec[0].x << endl;
	cout << "mpObjVec[0] .y : " << mpObjVec[0].y << endl;
	cout << "mpObjVec[0] .z : " << mpObjVec[0].z << endl;

	cout << "mpObjVec[56] .x : " << mpObjVec[56].x << endl;
	cout << "mpObjVec[56] .y : " << mpObjVec[56].y << endl;
	cout << "mpObjVec[56] .z : " << mpObjVec[56].z << endl;

	cout << "mpObjVec[35] .x : " << mpObjVec[35].x << endl;
	cout << "mpObjVec[35] .y : " << mpObjVec[35].y << endl;
	cout << "mpObjVec[35] .z : " << mpObjVec[35].z << endl;

	m_ppShaders2[0] = pObjectShader;

	//BuildLightsAndMaterials();
	//


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CStage::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
		delete[] m_ppGameObjects;
	}

	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; i++)
		{
			m_ppShaders[i]->ReleaseShaderVariables();
			m_ppShaders[i]->ReleaseObjects();
			m_ppShaders[i]->Release();
		}
		delete[] m_ppShaders;
	}

	if (m_pTerrain) delete m_pTerrain;
	if (m_pSkyBox) delete m_pSkyBox;

	if (m_ppHierarchicalGameObjects)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Release();
		delete[] m_ppHierarchicalGameObjects;
	}

	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature *CStage::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[9];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//pd3dDescriptorRanges[8].NumDescriptors = 1;
	//pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	//pd3dDescriptorRanges[8].RegisterSpace = 0;
	//pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//pd3dDescriptorRanges[9].NumDescriptors = 1;
	//pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	//pd3dDescriptorRanges[9].RegisterSpace = 0;
	//pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//23.02.01
	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 6; //b6: Game Objects2//수정
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//

	D3D12_ROOT_PARAMETER pd3dRootParameters[16];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	/*pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;*/

	/*pd3dRootParameters[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[16].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[16].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[16].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;*/

	//23.02.01
	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[8]; //Game Objects
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[14].Descriptor.ShaderRegister = 5; //Materials
	pd3dRootParameters[14].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//
	

	//23.01.06
	pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[15].Constants.Num32BitValues = 12;
	pd3dRootParameters[15].Constants.ShaderRegister = 3; //Constants
	pd3dRootParameters[15].Constants.RegisterSpace = 0;
	pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//


	//23.02.06
	//pd3dRootParameters[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	//pd3dRootParameters[15].Descriptor.ShaderRegister = 9; //Lights2
	//pd3dRootParameters[15].Descriptor.RegisterSpace = 0;
	//pd3dRootParameters[15].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//


	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CStage::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);

	//23.02.01
	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials); 
	//

	////23.02.06
	//UINT ncbElementBytes2 = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	//m_pd3dcbLights2 = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes2, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	//m_pd3dcbLights2->Map(0, NULL, (void**)&m_pcbMappedLights2);
	////
}

void CStage::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CStage::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}

	////23.02.06
	//if (m_pd3dcbLights2)
	//{
	//	m_pd3dcbLights2->Unmap(0, NULL);
	//	m_pd3dcbLights2->Release();
	//}
	////
}

void CStage::ReleaseUploadBuffers()
{
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();
	//if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();

	for (int i = 0; i < m_nShaders; i++) m_ppShaders[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nHierarchicalGameObjects; i++) m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();
}

void CStage::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);

	cout << "stage create heap" << endl;
	cout << "descriptor heap : " << m_pd3dCbvSrvDescriptorHeap << endl;
	cout << "CPUDescriptorNextHandle ptr: " << m_d3dSrvCPUDescriptorNextHandle.ptr << endl;
	cout << "GPUDescriptorNextHandle ptr: " << m_d3dSrvGPUDescriptorNextHandle.ptr << endl<<endl<<endl;
}

D3D12_GPU_DESCRIPTOR_HANDLE CStage::CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

void CStage::CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex)
{
	m_d3dSrvCPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);
	m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize * nDescriptorHeapIndex);

	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource* pShaderResource = pTexture->GetResource(i);
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = pTexture->GetShaderResourceViewDesc(i);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++) 
		pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);

	cout << "descriptor heap : " << m_pd3dCbvSrvDescriptorHeap << endl;
	//D3D12_GPU_DESCRIPTOR_HANDLE tmp = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	cout << "디스크립터 힙->gpu handle : " << m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr << endl;
	cout << "update heap cpu handle ptr: " << m_d3dSrvCPUDescriptorNextHandle.ptr << endl;
	cout << "update heap gpu handle ptr: " << m_d3dSrvGPUDescriptorNextHandle.ptr << endl<<endl<<endl<<endl;//총 24번 호출 아마도 스카이박스1 하이어아키23
}

bool CStage::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CStage::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		break;
	default:
		break;
	}
	return(false);
}

bool CStage::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CStage::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Animate(fTimeElapsed);
	for (int i = 0; i < m_nShaders; i++) if (m_ppShaders[i]) m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{
		m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();
	}

//**/
	static float fAngle = 0.0f;
	fAngle += 1.50f;
//	XMFLOAT3 xmf3Position = XMFLOAT3(50.0f, 0.0f, 0.0f);
	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Rotate(0.0f, -fAngle, 0.0f);
	XMFLOAT3 xmf3Position = Vector3::TransformCoord(XMFLOAT3(50.0f, 0.0f, 0.0f), xmf4x4Rotate);
//	m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent._41 = m_xmf3RotatePosition.x + xmf3Position.x;
//	m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent._42 = m_xmf3RotatePosition.y + xmf3Position.y;
//	m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent._43 = m_xmf3RotatePosition.z + xmf3Position.z;

	m_ppHierarchicalGameObjects[11]->m_xmf4x4ToParent = Matrix4x4::AffineTransformation(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, -fAngle, 0.0f), Vector3::Add(m_xmf3RotatePosition, xmf3Position));
	m_ppHierarchicalGameObjects[11]->Rotate(0.0f, -1.5f, 0.0f);
//**/
}

void CStage::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature) 
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	//23.02.02
	if (m_pd3dPipelineState) 
		pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
	//

	//23.02.07
	if (m_pd3dCbvSrvDescriptorHeap) 
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);
	//

	cout << "set descriptorheap : " << m_pd3dCbvSrvDescriptorHeap << endl << endl << endl;

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);//인형이 까맣게

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_LIGHT, d3dcbLightsGpuVirtualAddress); //Lights

	
	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList,m_pd3dGraphicsRootSignature,  m_pd3dPipelineState, pCamera);
	//if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState,pCamera);

	/*for (int i = 0; i < m_nShaders; i++) 
		if (m_ppShaders[i]) 
			m_ppShaders[i]->Render(pd3dCommandList, pCamera);*/

	for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			m_ppHierarchicalGameObjects[i]->Animate(m_fElapsedTime);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			m_ppHierarchicalGameObjects[i]->Render(pd3dCommandList, m_pd3dGraphicsRootSignature,  m_pd3dPipelineState, pCamera);
		}
	}

	//23.01.06
	//pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	//pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	//pCamera->UpdateShaderVariables(pd3dCommandList);

	//UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();//
	//23.01.08
	//pd3dCommandList->SetGraphicsRootConstantBufferView(4, d3dcbMaterialsGpuVirtualAddress); //Materials
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_MATERIAL, d3dcbMaterialsGpuVirtualAddress); //Materials//
	//

	//D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress2 = m_pd3dcbLights2->GetGPUVirtualAddress();
	////23.01.08
	////pd3dCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights
	//pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_LIGHT, d3dcbLightsGpuVirtualAddress2); //Lights
	////

	//m_d3dSrvGPUDescriptorNextHandle.ptr += (::gnCbvSrvDescriptorIncrementSize *0);
	m_ppShaders2[0]->Render2(pd3dCommandList, pCamera, m_d3dCbvGPUDescriptorHandle);//
	//m_ppShaders2[0]->Render2(pd3dCommandList, pCamera, m_d3dSrvGPUDescriptorNextHandle);
	//m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

	//cout << "m_ppShaders[0]->m_ppObjects : " << m_ppShaders[0]->m_ppObjects[0]->m_pstrName << endl;
	//
}

//23.01.30
void CStage::BuildLightsAndMaterials()
{
	m_pLights2 = new LIGHTSS;
	//23.01.13
	::ZeroMemory(m_pLights2, sizeof(LIGHTSS));
	//

	m_pLights2->m_xmf4GlobalAmbient = XMFLOAT4(0.034f, 0.034f, 0.034f, 1.0f);

	//23.01.03
	m_pLights2->m_pLights[0].m_bEnable = false;//
	m_pLights2->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights2->m_pLights[0].m_fRange = 100.0f;
	//m_pLights->m_pLights[0].m_fRange = 500.0f;
	m_pLights2->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights2->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.148f, 0.0f, 0.0f, 1.0f);
	m_pLights2->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights2->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights2->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights2->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights2->m_pLights[1].m_bEnable = true;//
	//m_pLights->m_pLights[1].m_bEnable = false;//
	m_pLights2->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights2->m_pLights[1].m_fRange = 150.0f;
	m_pLights2->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights2->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.14f, 0.14f, 0.14f, 1.0f);
	m_pLights2->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights2->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights2->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights2->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights2->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights2->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights2->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights2->m_pLights[2].m_bEnable = true;//
	m_pLights2->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	//23.01.03
	//m_pLights->m_pLights[2].m_fRange = 100.0f;
	//
	m_pLights2->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.053f, 0.053f, 0.053f, 1.0f);
	m_pLights2->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.0f, 0.17f, 0.17f, 1.0f);
	m_pLights2->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights2->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 1.0f);
	m_pLights2->m_pLights[3].m_bEnable = true;//
	m_pLights2->m_pLights[3].m_nType = DIRECTIONAL_LIGHT;
	//23.01.03
	//m_pLights->m_pLights[3].m_fRange = 100.0f;
	//
	m_pLights2->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.053f, 0.053f, 0.053f, 1.0f);
	m_pLights2->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.17f, 0.17f, 0.0f, 1.0f);
	m_pLights2->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights2->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights2->m_pLights[4].m_bEnable = true;//
	m_pLights2->m_pLights[4].m_nType = SPOT_LIGHT;
	m_pLights2->m_pLights[4].m_fRange = 30.0f;
	m_pLights2->m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.012f, 0.012f, 0.012f, 1.0f);
	m_pLights2->m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.24f, 0.14f, 0.24f, 1.0f);
	m_pLights2->m_pLights[4].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights2->m_pLights[4].m_xmf3Position = XMFLOAT3(0.0f, 5.0f, -7.0f);
	m_pLights2->m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights2->m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.25f, 0.05f);
	m_pLights2->m_pLights[4].m_fFalloff = 1.0f;
	m_pLights2->m_pLights[4].m_fPhi = (float)cos(XMConvertToRadians(10.0f));
	m_pLights2->m_pLights[4].m_fTheta = (float)cos(XMConvertToRadians(5.0f));

	for (int i = 5; i < MAX_LIGHTSS; ++i)
	{
		m_pLights2->m_pLights[i].m_bEnable = false;
		//m_pLights->m_pLights[i].m_bEnable =wakeUp;//
		//m_pLights->m_pLights[5].m_nType = SPOT_LIGHT;
		m_pLights2->m_pLights[i].m_nType = POINT_LIGHT;
		m_pLights2->m_pLights[i].m_fRange = 20.0f;

		m_pLights2->m_pLights[i].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
		m_pLights2->m_pLights[i].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 7.0f);
		m_pLights2->m_pLights[i].m_xmf4Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.0f);
		//m_pLights->m_pLights[5].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, -5.0f);
		m_pLights2->m_pLights[i].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_pLights2->m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
		m_pLights2->m_pLights[i].m_fFalloff = 8.0f;
		m_pLights2->m_pLights[i].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
		m_pLights2->m_pLights[i].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

		m_pLights2->m_pLights[i].m_xmf3Position = XMFLOAT3(mpObjVec[i - 5].x, mpObjVec[i - 5].y + 5, mpObjVec[i - 5].z);
		//m_pLights->m_pLights[5].m_xmf3Position = pos;
	}

	m_pMaterials = new MATERIALS;
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[1] = { XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f), XMFLOAT4(0.7f, 0.7f, 0.7f, 5.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[0] = { XMFLOAT4(0.3f, 0.4f, 0.0f, 1.0f), XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(0.0f, 0.6f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(0.5f, 0.0f, 0.6f, 1.0f), XMFLOAT4(0.0f, 0.5f, 0.8f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(0.0f, 0.5f, 0.6f, 1.0f), XMFLOAT4(0.5f, 0.0f, 0.8f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.8f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(0.5f, 0.5f, 0.6f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.8f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 0.5f, 0.6f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.8f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[8] = { XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[9] = { XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.8f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[10] = { XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[11] = { XMFLOAT4(0.4f, 0.0f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.3f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[12] = { XMFLOAT4(0.0f, 0.4f, 0.7f, 1.0f), XMFLOAT4(0.3f, 0.0f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[13] = { XMFLOAT4(0.0f, 0.3f, 0.3f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[14] = { XMFLOAT4(0.3f, 0.3f, 0.6f, 1.0f), XMFLOAT4(0.3f, 0.3f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[15] = { XMFLOAT4(0.7f, 0.5f, 0.7f, 1.0f), XMFLOAT4(0.5f, 0.0f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
}
//
