// File: CStage.cpp
//-----------------------------------------------------------------------------

#include <DirectXMath.h>
#include "stdafx.h"
#include "Stage.h"
#include"GameFramework.h"
ID3D12DescriptorHeap* CStage::m_pd3dCbvSrvDescriptorHeap = NULL;

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


double GetDegreeWithTwoVectors(XMFLOAT3& v1, XMFLOAT3& v2)
{
	float dot = Vector3::DotProduct(v1, v2);
	float v1Length = Vector3::Length(v1);
	float v2Length = Vector3::Length(v2);

	double angleRadian = acos(dot / (v1Length * v2Length));

	return XMConvertToDegrees(angleRadian);
}

XMFLOAT3 RotatePointBaseOnPoint(XMFLOAT3& p1, XMFLOAT3& p2, float angle)
{
	XMFLOAT3 translatedP1 = XMFLOAT3(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);

	XMFLOAT4X4 RotationMatrix = Matrix4x4::RotationYawPitchRoll(0, angle, 0);
	XMFLOAT3 rotatedP1 = Vector3::TransformCoord(translatedP1, RotationMatrix);
	XMFLOAT3 finalP1 = XMFLOAT3(rotatedP1.x + p2.x, rotatedP1.y + p2.y, rotatedP1.z + p2.z);

	return finalP1;
}


void CStage::BuildDefaultLightsAndMaterials()
{
	m_nLights = MAX_LIGHTS;

	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	//m_pLights[0].m_bEnable = true;
	//m_pLights[0].m_nType = POINT_LIGHT;
	//m_pLights[0].m_fRange = 300.0f;
	//m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.3f, 0.8f, 1.0f);
	//m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	//m_pLights[0].m_xmf3Position = XMFLOAT3(230.0f, 330.0f, 480.0f);
	//m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	////m_pLights[0].m_xmf3Direction = XMFLOAT3(+1.0f, -1.0f, 0.0f);
	//m_pLights[1].m_bEnable = true;
	//m_pLights[1].m_nType = SPOT_LIGHT;
	//m_pLights[1].m_fRange = 500.0f;
	//m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	//m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	//m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	//m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	//m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	//m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	//m_pLights[1].m_fFalloff = 8.0f;
	//m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	//m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	//m_pLights[2].m_bEnable = true;
	//m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	//m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	//m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	//m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
	//m_pLights[3].m_bEnable = true;
	//m_pLights[3].m_nType = SPOT_LIGHT;
	//m_pLights[3].m_fRange = 600.0f;
	//m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	//m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	//m_pLights[3].m_xmf3Position = XMFLOAT3(550.0f, 330.0f, 530.0f);
	//m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	//m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	//m_pLights[3].m_fFalloff = 8.0f;
	//m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	//m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));


	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_fRange = 2000.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.73f, 0.73f, 0.73f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(512-50, 212.0f, 1280.0f+140);// XMFLOAT3(-(_PLANE_WIDTH * 0.5f), 512.0f, 0.0f);//-562
	m_pLights[0].m_xmf3Direction = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	//m_pLights[0].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 1.0f);

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


	m_pLights[2].m_bEnable = false;
	m_pLights[2].m_nType = SPOT_LIGHT;
	m_pLights[2].m_fRange = 500.0f;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.85f, 0.85f, 0.85f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights[2].m_xmf3Position = XMFLOAT3(0.0f, 256.0f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(+1.0f, -1.0f, 0.0f);
	m_pLights[2].m_xmf3Attenuation = XMFLOAT3(0.5f, 0.01f, 0.0001f);
	m_pLights[2].m_fFalloff = 4.0f;
	m_pLights[2].m_fPhi = (float)cos(XMConvertToRadians(60.0f));
	m_pLights[2].m_fTheta = (float)cos(XMConvertToRadians(30.0f));

	m_pLights[3].m_bEnable = false;
	m_pLights[3].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[3].m_fRange = 1000.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.83f, 0.83f, 0.83f, 1.0f);

	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(0.0f, 128.0f, 0.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(+1.0f, -1.0f, 0.0f);
	//m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);



	{
		/*m_pLights[4].m_bEnable = true;
		m_pLights[4].m_nType = POINT_LIGHT;
		m_pLights[4].m_fRange = 200.0f;
		m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.3f, 0.3f, 1.0f);
		m_pLights[4].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
		m_pLights[4].m_xmf3Position = XMFLOAT3(600.0f, 250.0f, 700.0f);
		m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);*/

		m_pLights[4].m_bEnable = true;
		m_pLights[4].m_nType = SPOT_LIGHT;
		m_pLights[4].m_fRange = 500.0f;
		m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		m_pLights[4].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		//m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		//m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);//0415
		//m_pLights[4].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);


		m_pLights[4].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
		m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
		m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
		m_pLights[4].m_fFalloff = 8.0f;
		m_pLights[4].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
		m_pLights[4].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

		for (int i = 5; i < MAX_LIGHTS; ++i)
		{
			m_pLights[i].m_bEnable = false;
			//m_pLights->m_pLights[i].m_bEnable =wakeUp;//
			//m_pLights->m_pLights[5].m_nType = SPOT_LIGHT;
			m_pLights[i].m_nType = POINT_LIGHT;
			m_pLights[i].m_fRange = 120.0f;

			m_pLights[i].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
			m_pLights[i].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 7.0f);
			m_pLights[i].m_xmf4Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.0f);
			//m_pLights->m_pLights[5].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, -5.0f);
			m_pLights[i].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
			m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
			m_pLights[i].m_fFalloff = 8.0f;
			m_pLights[i].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
			m_pLights[i].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

			//m_pLights[i].m_xmf3Position = XMFLOAT3(mpObjVec[i - 5].x, mpObjVec[i - 5].y + 5, mpObjVec[i - 5].z);
			//m_pLights->m_pLights[5].m_xmf3Position = pos;
		}
	}
}


void CStage::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);


	CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 2000); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()//76

	DXGI_FORMAT pdxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };


	m_pLights = new LIGHT[MAX_LIGHTS];
	BuildDefaultLightsAndMaterials();//Á¶¸í

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);


	pBoxShader = new CBoxShader();
	pBoxShader->BuildObjects(pd3dDevice, pd3dCommandList, NULL);//¹Ù´Ú


	m_nShaders = 1;
	m_ppShaders = new CShader * [m_nShaders];
	CObjectsShader* pObjectShader = new CObjectsShader();
	pObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 5, pdxgiRtvFormats, DXGI_FORMAT_D32_FLOAT);//
	mpObjVec = pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, "Models/Scene.bin", pBoxShader);
	m_ppShaders[0] = pObjectShader;



	CTexture* ppTextures[30];
	ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Wood_missing_low_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	ppTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[1]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Wood_ground_whole_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[2] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[2]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/wall_02_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[3] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[3]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/wall_03_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[4] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[4]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/ceiling_wood_02_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[5] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[5]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Box_Wood_4_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[6] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[6]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Box_Wood_3_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[7] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[7]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Boxes_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[8] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[8]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Wooden_Chair_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[9] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[9]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Door_Frame_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[10] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[10]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/cart_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[11] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[11]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Poster_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[12] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[12]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/dressing_table_02_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[13] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[13]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Pillows_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[14] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[14]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Matress_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[15] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[15]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Cloth_hanging_02_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[16] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[16]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Cloth_hanging_01_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[17] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[17]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/woodalt_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[18] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[18]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Stairs_secondary_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[19] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[19]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/stairs_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[20] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[20]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Box_Wood_2_AlbedoTransparency.dds", RESOURCE_TEXTURE2D, 0);
	
	ppTextures[21] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	ppTextures[21]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Models/Texture/Ceiling_wood_mat_BaseMap.dds", RESOURCE_TEXTURE2D, 0);

	for (int a = 0; a < 22; ++a)
	{
		CreateShaderResourceViews(pd3dDevice, ppTextures[a], 0, 3);
	}

	for (int i = 0; i < m_ppShaders[0]->m_nObjects; ++i)
	{


		for (UINT k = 0; k < m_ppShaders[0]->m_ppObjects[i]->m_nMaterials; k++)
		{
			CMaterial* pMaterial = new CMaterial(1);
			pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);

			if (0 == strcmp("Dense_Floor_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName)	)
			{
				pMaterial->SetTexture(ppTextures[1]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}

			if (0 == strcmp("Dense_wood_missing_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[0]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}

			if (0 == strcmp("WoodBox9", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[2]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}

			if (0 == strcmp("Bedroom_wall_d_02_dense_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName)||
				0 == strcmp("Bedroom_wall_b_01_dense_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName)||
				0 == strcmp("Bedroom_wall_b_06_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[3]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("WoodBox10", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[5]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("WoodBox3", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("WoodBox4", m_ppShaders[0]->m_ppObjects[i]->m_pstrName)
				)
			{
				pMaterial->SetTexture(ppTextures[6]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Chair_03_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[8]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Door_01_Frame_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[9]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Cart_static_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[10]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Poster_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Poster_02_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Poster_03_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Poster_04_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[11]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Dressing_table_drawer_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Dressing_table_mirror_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[12]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Bed_blanked_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) ||
				0 == strcmp("Bed_pillows_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[13]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Matress_mat_BaseMap", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[14]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Cloth_04_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName)&&
				0 == strcmp("Cloth_05_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName)&&
 				0 == strcmp("Cloth_06_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
				0 == strcmp("Cloth_07_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[15]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Cloth_03_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
			0 == strcmp("Cloth_02_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[16]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Beam_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) &&
				0 == strcmp("Beam_02_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[17]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Stair_side_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) )
			{
				pMaterial->SetTexture(ppTextures[18]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Stair_step_01_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName) )
			{
				pMaterial->SetTexture(ppTextures[19]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("WoodBox6", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[20]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			if (0 == strcmp("Ceiling_concrete_base_mesh", m_ppShaders[0]->m_ppObjects[i]->m_pstrName))
			{
				pMaterial->SetTexture(ppTextures[21]);
				m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
			}
			//cout << i << "	|	" << m_ppShaders[0]->m_ppObjects[i]->m_pstrName << endl;
		}
	}

	//for (int i = 0; i < m_ppShaders[0]->m_nObjects; ++i)
	//{
	//	for (UINT k = 0; k < m_ppShaders[0]->m_ppObjects[i]->m_nMaterials; k++)
	//	{
	//		CMaterial* pMaterial = new CMaterial(1);
	//		pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	//		pMaterial->m_ppTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 3);
	//		pMaterial->m_ppTextures[0]->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, m_ppShaders[0]->m_ppObjects[i]->m_ppMaterials[k]->m_ppstrTextureNames[0], RESOURCE_TEXTURE2D, 0);

	//		CreateShaderResourceViews(pd3dDevice, pMaterial->m_ppTextures[0], 0, 3);
	//		// Assign the material to the object's mesh
	//		m_ppShaders[0]->m_ppObjects[i]->SetMaterial(k, pMaterial);
	//		/*m_ppShaders[0]->m_ppObjects[i]->m_ppMaterials[0]->SetTexture(pMaterial->m_ppTextures[0]);*/
	//		cout << i << "	|	" << k << endl;
	//	}
	//}

	for (int i = 0; i < m_ppShaders[0]->m_nObjects; ++i)
	{
		m_ppShaders[0]->m_ppObjects[i]->Boundingbox_Transform();
		/*cout << "Name: " << m_ppShaders[0]->m_ppObjects[i]->m_pstrName << endl;
		cout << "Center: ";
		Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox.Center);
		cout << "Extents: ";
		Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox.Extents);*/

		//if (0 == strcmp(m_ppShaders[0]->m_ppObjects[i]->m_pstrName, "Bedroom_wall_d_02_dense_mesh_(41)"))//041666
		//	for (int j{}; j < m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->m_nVertices; ++j)
		//	{
		//		cout << m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->m_pxmf3Normals[j].x << endl;
		//		cout << m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->m_pxmf3Normals[j].y << endl;
		//		cout << m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->m_pxmf3Normals[j].z << endl << endl << endl;
		//	}
	}


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CStage::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature)
		m_pd3dGraphicsRootSignature->Release();
	if (m_pd3dCbvSrvDescriptorHeap)
		m_pd3dCbvSrvDescriptorHeap->Release();

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
	if (m_pShadowShader)
	{
		m_pShadowShader->ReleaseShaderVariables();
		m_pShadowShader->ReleaseObjects();
		m_pShadowShader->Release();
	}


	ReleaseShaderVariables();

	if (m_pLights) delete[] m_pLights;
}

ID3D12RootSignature* CStage::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[8];

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
	pd3dDescriptorRanges[7].NumDescriptors = MAX_DEPTH_TEXTURES;
	pd3dDescriptorRanges[7].BaseShaderRegister = 2; //Depth Buffer
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER pd3dRootParameters[15];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 17;
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
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[7]; //Depth Buffer
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;


	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[13].Descriptor.ShaderRegister = 6; //ToLight
	pd3dRootParameters[13].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[14].Constants.Num32BitValues = 17;
	pd3dRootParameters[14].Constants.ShaderRegister = 3; //Material
	pd3dRootParameters[14].Constants.RegisterSpace = 0;
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[4];

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

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0.0f;
	pd3dSamplerDescs[2].MaxAnisotropy = 1;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //D3D12_COMPARISON_FUNC_LESS
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;//
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[3].MipLODBias = 0.0f;
	pd3dSamplerDescs[3].MaxAnisotropy = 1;
	pd3dSamplerDescs[3].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	pd3dSamplerDescs[3].MinLOD = 0;
	pd3dSamplerDescs[3].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[3].ShaderRegister = 3;//
	pd3dSamplerDescs[3].RegisterSpace = 0;
	pd3dSamplerDescs[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CStage::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256ï¿½ï¿½ ï¿½ï¿½ï¿?
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void CStage::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
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
}

void CStage::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++)
		m_ppShaders[i]->ReleaseUploadBuffers();

	if (m_pShadowShader)
		m_pShadowShader->ReleaseUploadBuffers();
}

void CStage::CreateCbvSrvDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CStage::CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dConstantBuffers, UINT nStride)
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
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);//0307 pShaderResourceï¿½ï¿½ null
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
			pTexture->SetGpuDescriptorHandle(i, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	int nRootParameters = pTexture->GetRootParameters();
	for (int j = 0; j < nRootParameters; j++)
		pTexture->SetRootParameterIndex(j, nRootParameterStartIndex + j);
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

bool CStage::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CStage::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;
	m_ppShaders[0]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{
		m_pLights[4].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights[4].m_xmf3Position.y = m_pPlayer->GetPosition().y + 10;
		m_pLights[4].m_xmf3Direction = m_pPlayer->GetLookVector();

		/*m_fLightRotationAngle += fTimeElapsed * 0.25f;
		XMMATRIX xmmtxRotation = XMMatrixRotationY(fTimeElapsed * 0.25f);
		XMStoreFloat3(&m_pLights[1].m_xmf3Direction, XMVector3TransformNormal(XMLoadFloat3(&m_pLights[1].m_xmf3Direction), xmmtxRotation));*/
		m_pLights[1].m_bEnable = false;

		for (int i = 5; i < MAX_LIGHTS; ++i)
			m_pLights[i].m_bEnable = wakeUp;
	}

	static float fAngle = 0.0f;
	fAngle += 1.50f;
	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Rotate(0.0f, -fAngle, 0.0f);
	XMFLOAT3 xmf3Position = Vector3::TransformCoord(XMFLOAT3(50.0f, 0.0f, 0.0f), xmf4x4Rotate);

}

void CStage::OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList, LIGHT* light, ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap, vector<CMonster*> Monsters, vector<CPlayer*> Players)
{
	if (m_pDepthRenderShader)
	{
		m_pDepthRenderShader->m_pd3dCbvSrvDescriptorHeap = m_pd3dCbvSrvDescriptorHeap;
		m_pDepthRenderShader->PrepareShadowMap(pd3dCommandList, light, Monsters, Players);
	}
}
void CStage::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	UpdateShaderVariables(pd3dCommandList);

	//if (m_pd3dcbMaterials)
	//{
	//	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	//	pd3dCommandList->SetGraphicsRootConstantBufferView(3, d3dcbMaterialsGpuVirtualAddress); //Materials
	//}
	if (m_pd3dcbLights)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_LIGHT, d3dcbLightsGpuVirtualAddress); //Lights
	}
}


void CStage::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_pd3dCbvSrvDescriptorHeap)
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	if (m_pDepthRenderShader)
		m_pDepthRenderShader->UpdateShaderVariables(pd3dCommandList);
	
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);//Á¶¸í

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_LIGHT, d3dcbLightsGpuVirtualAddress); //Lights



	//m_ppShaders[0]->Render(pd3dCommandList, pCamera);
}

void CStage::UpdateBoundingBox()
{

}

void CStage::CheckObjectByObjectCollisions(float fTimeElapsed, CPlayer*& pl)
{
	XMFLOAT3 Vel = pl->GetVelocity();
	XMFLOAT3 MovVec = Vector3::ScalarProduct(Vel, fTimeElapsed, false);
	BoundingOrientedBox pBox = pl->obBox;


	for (int i = 0; i < m_ppShaders[0]->m_nObjects; i++)
	{
		BoundingOrientedBox oBox = m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox;

		if (pBox.Intersects(oBox))
		{

			if (pBox.Center.y > oBox.Center.y + oBox.Extents.y && Vel.y <= 0) {
				XMFLOAT3 Pos = pl->GetPosition();
				Pos.y = oBox.Center.y + oBox.Extents.y + pBox.Extents.y;
				pl->SetPosition(Pos);
				pl->SetVelocity(XMFLOAT3(Vel.x, 0.0f, Vel.z));
				pl->onFloor = true;
				continue;
			}


			cout << "Name - " << m_ppShaders[0]->m_ppObjects[i]->m_pstrName << endl;
			//cout << "Center - ";
			//Vector3::Print(oBox.Center);
			//cout << "Extents - ";
			//Vector3::Print(oBox.Extents);
			//cout << "Look - ";
			//Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->GetLook());
			//cout << "Right - ";
			//Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->GetRight());
			//cout << "Up - ";
			//Vector3::Print(m_ppShaders[0]->m_ppObjects[i]->GetUp());

			float angle = GetDegreeWithTwoVectors(m_ppShaders[0]->m_ppObjects[i]->GetLook(), XMFLOAT3(0, -m_ppShaders[0]->m_ppObjects[i]->GetLook().y, 1));

			XMFLOAT3 ObjLook = { 0,0,0 };


			if ((int)angle % 90 == 0)
			{
				XMVECTOR xmVector = XMLoadFloat3(&oBox.Extents);
				XMVECTOR xmQuaternion = XMLoadFloat4(&oBox.Orientation);

				// Rotate the vector using the quaternion
				XMVECTOR rotatedVector = XMVector3Rotate(xmVector, xmQuaternion);

				// Convert the rotated vector back to an XMFLOAT3
				XMFLOAT3 realExtents;
				XMStoreFloat3(&realExtents, rotatedVector);

				realExtents.x = sqrtf(realExtents.x * realExtents.x);
				realExtents.y = sqrtf(realExtents.y * realExtents.y);
				realExtents.z = sqrtf(realExtents.z * realExtents.z);

				if (oBox.Center.x - realExtents.x < pBox.Center.x && oBox.Center.x + realExtents.x > pBox.Center.x) {
					if (oBox.Center.z < pBox.Center.z) ObjLook = { 0,0,1 };
					else ObjLook = { 0, 0, -1 };
				}
				else if (oBox.Center.x < pBox.Center.x) ObjLook = { 1,0,0 };
				else ObjLook = { -1, 0, 0 };

			}
			else
			{

				XMFLOAT3 RotatedPos = RotatePointBaseOnPoint(pBox.Center, oBox.Center, -angle);

				if (oBox.Center.x - oBox.Extents.x < RotatedPos.x && oBox.Center.x + oBox.Extents.x > RotatedPos.x) {
					if (oBox.Center.z < RotatedPos.z) ObjLook = m_ppShaders[0]->m_ppObjects[i]->GetLook();
					else ObjLook = Vector3::ScalarProduct(m_ppShaders[0]->m_ppObjects[i]->GetLook(), -1);
				}
				else if (oBox.Center.x < RotatedPos.x) ObjLook = m_ppShaders[0]->m_ppObjects[i]->GetRight();
				else ObjLook = Vector3::ScalarProduct(m_ppShaders[0]->m_ppObjects[i]->GetRight(), -1);
			}
			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			pl->Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			pl->Move(MovVec, false);

		}
	}
}

void CStage::CheckMoveObjectsCollisions(float fTimeElapsed, CPlayer*& pl, vector<CMonster*>& monsters, vector<CPlayer*>& players) {

	XMFLOAT3 Vel = pl->GetVelocity();
	XMFLOAT3 MovVec = Vector3::ScalarProduct(Vel, fTimeElapsed, false);
	BoundingOrientedBox pBox = pl->obBox;

	for (const auto& monster : monsters) {
		if (pBox.Intersects(monster->m_xmOOBB)) {
			XMFLOAT3 ObjLook = { 0,0,0 };


			if (monster->m_xmOOBB.Center.x - monster->m_xmOOBB.Extents.x < pBox.Center.x && monster->m_xmOOBB.Center.x + monster->m_xmOOBB.Extents.x > pBox.Center.x) {
				if (monster->m_xmOOBB.Center.z < pBox.Center.z) ObjLook = { 0,0,1 };
				else ObjLook = { 0, 0, -1 };
			}
			else if (monster->m_xmOOBB.Center.x < pBox.Center.x) ObjLook = { 1,0,0 };
			else ObjLook = { -1, 0, 0 };
			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			pl->Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			pl->Move(MovVec, false);
		}
	}

	for (auto& player : players) {
		if (player->c_id == pl->c_id || player->alive == false) continue;
		if (pBox.Intersects(player->obBox)) {
			XMFLOAT3 ObjLook = { 0,0,0 };


			if (player->obBox.Center.x - player->obBox.Extents.x < pBox.Center.x && player->obBox.Center.x + player->obBox.Extents.x > pBox.Center.x) {
				if (player->obBox.Center.z < pBox.Center.z) ObjLook = { 0,0,1 };
				else ObjLook = { 0, 0, -1 };
			}
			else if (player->obBox.Center.x < pBox.Center.x) ObjLook = { 1,0,0 };
			else ObjLook = { -1, 0, 0 };
			if (Vector3::DotProduct(MovVec, ObjLook) > 0)
				continue;

			XMFLOAT3 ReflectVec = Vector3::ScalarProduct(MovVec, -1, false);

			pl->Move(ReflectVec, false);

			MovVec = GetReflectVec(ObjLook, MovVec);
			pl->Move(MovVec, false);
		}
	}
}
void CStage::CheckCameraCollisions(float fTimeElapsed, CPlayer*& pl, CCamera*& cm)
{
	XMFLOAT4X4 xmf4x4Rotate = Matrix4x4::Identity();
	XMFLOAT3 xmf3Right = m_pPlayer->GetRightVector();
	XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
	XMFLOAT3 xmf3Look = m_pPlayer->GetLookVector();
	xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
	xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
	xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;

	if (cm->GetMode() == THIRD_PERSON_CAMERA) {
		XMFLOAT3 xmf3Offset = Vector3::TransformCoord(cm->GetOffset(), xmf4x4Rotate);
		XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
		XMFLOAT3 ray_castPos = pl->GetPosition();
		BoundingBox test = BoundingBox(ray_castPos, XMFLOAT3(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON));
		XMFLOAT3 dir = Vector3::Normalize(Vector3::Subtract(xmf3Position, pl->GetPosition()));

		bool collide = false;
		while (Vector3::Length(Vector3::Subtract(xmf3Position, ray_castPos)) > 5.f)
		{
			for (int i = 0; i < m_ppShaders[0]->m_nObjects; i++)
			{
				BoundingOrientedBox oBox = m_ppShaders[0]->m_ppObjects[i]->m_ppMeshes[0]->OBBox;
				if (oBox.Contains(XMLoadFloat3(&ray_castPos)))
				{
					collide = true;
					break;
				}
			}
			if (collide) {
				xmf3Position = ray_castPos;
				break;
			}
			ray_castPos = Vector3::Add(ray_castPos, dir);
		}

		cm->Update(xmf3Position, pl->GetPosition(), fTimeElapsed);
	}


	cm->SetLookAt(pl->GetPosition());
	cm->RegenerateViewMatrix();
}
XMFLOAT3 CStage::GetReflectVec(XMFLOAT3 ObjLook, XMFLOAT3 MovVec)
{
	float Dot = Vector3::DotProduct(MovVec, ObjLook);
	XMFLOAT3 Nor = Vector3::ScalarProduct(ObjLook, Dot, false);
	XMFLOAT3 SlidingVec = Vector3::Subtract(MovVec, Nor);
	return SlidingVec;
}


XMFLOAT3 CStage::Calculate_Direction(BoundingBox& pBouningBoxA, BoundingBox& pBouningBoxB)
{
	XMVECTOR xmV1min = XMLoadFloat3(&pBouningBoxA.Center) - XMLoadFloat3(&pBouningBoxA.Extents);
	XMVECTOR xmV1max = XMLoadFloat3(&pBouningBoxA.Center) + XMLoadFloat3(&pBouningBoxA.Extents);
	XMVECTOR xmV2min = XMLoadFloat3(&pBouningBoxB.Center) - XMLoadFloat3(&pBouningBoxB.Extents);
	XMVECTOR xmV2max = XMLoadFloat3(&pBouningBoxB.Center) + XMLoadFloat3(&pBouningBoxB.Extents);

	bool bIntersect = XMVector3GreaterOrEqual(xmV1min, xmV2max) || XMVector3GreaterOrEqual(xmV2min, xmV1max);

	if (bIntersect)
	{
		return XMFLOAT3(0, 0, 0); // ï¿½æµ¹ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½
	}

	XMFLOAT3 xmf3Direction = { 0,0,0 };
	XMFLOAT3 xmf3Subtraction = { 0,0,0 };

	XMStoreFloat3(&xmf3Subtraction, XMVectorSubtract(xmV2max, xmV1min));
	if (fabs(xmf3Subtraction.x) < fabs(xmf3Direction.x) || xmf3Direction.x == 0)
	{
		xmf3Direction.x = xmf3Subtraction.x;
	}
	XMStoreFloat3(&xmf3Subtraction, XMVectorSubtract(xmV1max, xmV2min));
	if (fabs(xmf3Subtraction.x) < fabs(xmf3Direction.x) || xmf3Direction.x == 0)
	{
		xmf3Direction.x = -xmf3Subtraction.x;
	}

	XMStoreFloat3(&xmf3Subtraction, XMVectorSubtract(xmV2max, xmV1min));
	if (fabs(xmf3Subtraction.y) < fabs(xmf3Direction.y) || xmf3Direction.y == 0)
	{
		xmf3Direction.y = xmf3Subtraction.y;
	}
	XMStoreFloat3(&xmf3Subtraction, XMVectorSubtract(xmV1max, xmV2min));
	if (fabs(xmf3Subtraction.y) < fabs(xmf3Direction.y) || xmf3Direction.y == 0)
	{
		xmf3Direction.y = -xmf3Subtraction.y;
	}

	XMStoreFloat3(&xmf3Subtraction, XMVectorSubtract(xmV2max, xmV1min));
	if (fabs(xmf3Subtraction.z) < fabs(xmf3Direction.z) || xmf3Direction.z == 0)
	{
		xmf3Direction.z = xmf3Subtraction.z;
	}
	XMStoreFloat3(&xmf3Subtraction, XMVectorSubtract(xmV1max, xmV2min));
	if (fabs(xmf3Subtraction.z) < fabs(xmf3Direction.z) || xmf3Direction.z == 0)
	{
		xmf3Direction.z = -xmf3Subtraction.z;
	}

	XMStoreFloat3(&xmf3Direction, XMVector3Normalize(XMLoadFloat3(&xmf3Direction)));
	return xmf3Direction;
}

