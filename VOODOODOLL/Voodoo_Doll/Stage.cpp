#include "stdafx.h"
#include "Stage.h"



CStage::CStage()
{
}

CStage::~CStage()
{
}

void CStage::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;


	if (false == choose)
	{
		::ZeroMemory(m_pLights, sizeof(LIGHTS));
	}

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.034f, 0.034f, 0.034f, 1.0f);

	//23.01.03
	//23.01.19
	m_pLights->m_pLights[0].m_bEnable = false;
	//
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	//m_pLights->m_pLights[0].m_fRange = 500.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.148f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(130.0f, 30.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = true;//
	//m_pLights->m_pLights[1].m_bEnable = false;//
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 150.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.14f, 0.14f, 0.14f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights->m_pLights[2].m_bEnable = true;//
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	//23.01.03
	//m_pLights->m_pLights[2].m_fRange = 100.0f;
	//
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.053f, 0.053f, 0.053f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.0f, 0.17f, 0.17f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_bEnable = true;//
	m_pLights->m_pLights[3].m_nType = DIRECTIONAL_LIGHT;
	//23.01.03
	//m_pLights->m_pLights[3].m_fRange = 100.0f;
	//
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.053f, 0.053f, 0.053f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.17f, 0.17f, 0.0f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 1.0f);
	m_pLights->m_pLights[4].m_bEnable = true;//
	m_pLights->m_pLights[4].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[4].m_fRange = 30.0f;
	m_pLights->m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.012f, 0.012f, 0.012f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.24f, 0.14f, 0.24f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Position = XMFLOAT3(0.0f, 5.0f, -7.0f);
	m_pLights->m_pLights[4].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.25f, 0.05f);
	m_pLights->m_pLights[4].m_fFalloff = 1.0f;
	m_pLights->m_pLights[4].m_fPhi = (float)cos(XMConvertToRadians(10.0f));
	m_pLights->m_pLights[4].m_fTheta = (float)cos(XMConvertToRadians(5.0f));


	//23.01.17
	//for (int i = 5; i < 24; ++i)
	for (int i = 5; i < MAX_LIGHTS; ++i)
		//for (int i = 0; i < 1; ++i)
	{
		m_pLights->m_pLights[i].m_bEnable = false;
		//m_pLights->m_pLights[i].m_bEnable =wakeUp;//
		//m_pLights->m_pLights[5].m_nType = SPOT_LIGHT;
		m_pLights->m_pLights[i].m_nType = POINT_LIGHT;

		//23.01.17
		//m_pLights->m_pLights[i].m_fRange = 2.0f;
		m_pLights->m_pLights[i].m_fRange = 140.0f;
		//

		m_pLights->m_pLights[i].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
		m_pLights->m_pLights[i].m_xmf4Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 7.0f);
		m_pLights->m_pLights[i].m_xmf4Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.0f);
		//m_pLights->m_pLights[5].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, -5.0f);
		m_pLights->m_pLights[i].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_pLights->m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
		m_pLights->m_pLights[i].m_fFalloff = 8.0f;
		m_pLights->m_pLights[i].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
		m_pLights->m_pLights[i].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

		//23.01.17
		m_pLights->m_pLights[i].m_xmf3Position = XMFLOAT3(mpObjVec[i - 5].x, mpObjVec[i - 5].y + 5, mpObjVec[i - 5].z);
		//m_pLights->m_pLights[i].m_xmf3Position = XMFLOAT3(mpObjVec[i].x, mpObjVec[i].y + 5, mpObjVec[i].z);
		//
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

void CStage::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	//23.01.16
	m_nShaders = 1;
	//
	m_ppShaders = new CShader * [m_nShaders];

	CObjectsShader* pObjectShader = new CObjectsShader();
	pObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);

	// 23.01.16
	mpObjVec = pObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, "Models/Scene.bin");
	//

	m_ppShaders[0] = pObjectShader;

	for (int i = 0; i < m_ppShaders[0]->m_nObjects; ++i)
	{
		//m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center = m_ppShaders[0]->m_ppObjects[i]->GetPosition();

		m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center.x = m_ppShaders[0]->m_ppObjects[i]->GetPosition().x;
		m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center.y = m_ppShaders[0]->m_ppObjects[i]->GetPosition().y;
		m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center.z = m_ppShaders[0]->m_ppObjects[i]->GetPosition().z;
	}

	//23.01.17
	CAirplaneMonster* pAirplaneMonster = new CAirplaneMonster(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	m_pMonster = pAirplaneMonster;
	//

	BuildLightsAndMaterials();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CStage::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

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

	ReleaseShaderVariables();

	if (m_pLights) delete m_pLights;
	if (m_pMaterials) delete m_pMaterials;

	//23.01.17
	if (m_pMonster)
		delete m_pMonster;
	//
}

void CStage::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nShaders; i++)
		m_ppShaders[i]->ReleaseUploadBuffers();
}

ID3D12RootSignature* CStage::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[5];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Camera: b0
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //GameObject: b1
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 2; //Materials
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = 3; //Lights
	pd3dRootParameters[3].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[4].Constants.Num32BitValues = 12;
	pd3dRootParameters[4].Constants.ShaderRegister = 4; //Constants
	pd3dRootParameters[4].Constants.RegisterSpace = 0;
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 16;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CStage::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbMaterials->Map(0, NULL, (void**)&m_pcbMappedMaterials);
}

void CStage::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));
	//::memcpy(m_pcbMappedLights2, m_pLights2, sizeof(LIGHTS));
	::memcpy(m_pcbMappedMaterials, m_pMaterials, sizeof(MATERIALS));
	//::memcpy(m_pcbMappedMaterials2, m_pMaterials2, sizeof(MATERIALS));
}

void CStage::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
	if (m_pd3dcbMaterials)
	{
		m_pd3dcbMaterials->Unmap(0, NULL);
		m_pd3dcbMaterials->Release();
	}
}

bool CStage::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CStage::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CStage::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CStage::AnimateObjects(float fTimeElapsed)
{
	m_ppShaders[0]->AnimateObjects(fTimeElapsed);

	if (m_pLights)
	{

		//23.01.03
		m_pLights->m_pLights[1].m_xmf3Position = m_pPlayer->GetPosition();
		m_pLights->m_pLights[1].m_xmf3Direction = m_pPlayer->GetLookVector();

		m_pLights->m_pLights[4].m_xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), Vector3::Add(Vector3::ScalarProduct(m_pPlayer->GetLookVector(), -15.0f, false), XMFLOAT3(0.0f, 5.0f, 0.0f)));
		m_pLights->m_pLights[4].m_xmf3Direction = m_pPlayer->GetLookVector();
		if (false == choose)
		{
			//23.01.17
			//for (int i = 5; i < 24; ++i)
			for (int i = 5; i < MAX_LIGHTS; ++i)
				m_pLights->m_pLights[i].m_bEnable = wakeUp;
			//
		}
	}

	//23.01.17
	XMFLOAT3 xmf3Shift
		= XMFLOAT3(m_pPlayer->GetPosition().x - m_pMonster->GetPosition().x, m_pPlayer->GetPosition().y - m_pMonster->GetPosition().y, m_pPlayer->GetPosition().z - m_pMonster->GetPosition().z);
	m_pMonster->Update(xmf3Shift, fTimeElapsed);
	//

	/*cout << "x : " << m_pPlayer->GetPosition().x << endl;
	cout << "y : " << m_pPlayer->GetPosition().y << endl;
	cout << "z : " << m_pPlayer->GetPosition().z << endl;*/

	//23.01.17
	//문 영역
	if (m_pPlayer->GetPosition().x > -50 && m_pPlayer->GetPosition().x < -22
		&& m_pPlayer->GetPosition().y > 1 && m_pPlayer->GetPosition().y < 30
		&& m_pPlayer->GetPosition().z > -20 && m_pPlayer->GetPosition().z < 40)
	{
	}
	else
		CheckObjectByObjectCollisions();//충돌 처리 함수
	// 
	//
}

//23.01.03
void CStage::UpdateBoundingBox()
{
	for (int i = 0; i < m_ppShaders[0]->m_nObjects; ++i)
	{
		if (m_ppShaders[0]->m_ppObjects[i]->m_pMesh)
		{
			m_ppShaders[0]->m_ppObjects[i]->m_pMesh->m_xmBoundingBox.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));//(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
			//XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
		}
	}
}
//


void CStage::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbMaterialsGpuVirtualAddress); //Materials

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(3, d3dcbLightsGpuVirtualAddress); //Lights

	m_ppShaders[0]->Render(pd3dCommandList, pCamera);

	//23.01.17
	if (m_pMonster)
		m_pMonster->Render(pd3dCommandList, pCamera);
	//
}

void CStage::CheckObjectByObjectCollisions()
{
	////23.01.17
	//if (m_pPlayer->GetPosition().x > -50 && m_pPlayer->GetPosition().x < -22
	//	&& m_pPlayer->GetPosition().y > 1 && m_pPlayer->GetPosition().y < 30
	//	&& m_pPlayer->GetPosition().z > -20 && m_pPlayer->GetPosition().z < 40)
	//{
	//	return;
	//}
	//else
	//{
	//	for (int i = 0; i < m_ppShaders[0]->m_nObjects - 1; i++)
	//	{
	//		//m_ppShaders[0]->m_ppObjects[i]->m_pMesh->m_xmBoundingBox
	//		if (m_pPlayer->m_xmOOBB.Intersects(m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB))
	//			//if (m_pPlayer->m_xmOOBB.Intersects(m_ppShaders[0]->m_ppObjects[i]->m_pMesh->m_xmBoundingBox))
	//		{
	//			//if(m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center==XMFLOAT3(0,0,0))

	//			if (!(m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center.x == 0 && m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center.y == 0 &&
	//				m_ppShaders[0]->m_ppObjects[i]->m_xmOOBB.Center.z == 0))
	//			{
	//				m_pPlayer->m_pObjectCollided = m_ppShaders[0]->m_ppObjects[i];
	//				//m_ppShaders[i]->m_pObjectCollided = m_pPlayer;

	//				XMFLOAT3 xmfsub = m_ppShaders[0]->m_ppObjects[i]->GetPosition();
	//				xmfsub = Vector3::Subtract(m_pPlayer->GetPosition(), xmfsub);

	//				xmfsub = Vector3::Normalize(xmfsub);
	//				m_pPlayer->SetPosition(XMFLOAT3(m_pPlayer->GetPosition().x + xmfsub.x, m_pPlayer->GetPosition().y + xmfsub.y, m_pPlayer->GetPosition().z + xmfsub.z));
	//			}
	//		}
	//	}
	//}
}
