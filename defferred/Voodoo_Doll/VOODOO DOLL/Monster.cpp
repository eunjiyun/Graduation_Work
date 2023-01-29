//-----------------------------------------------------------------------------
// File: Cmonster.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "monster.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cmonster

Cmonster::Cmonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
}

Cmonster::~Cmonster()
{
	ReleaseShaderVariables();

	if (m_pCamera)delete m_pCamera;
}

void Cmonster::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera)m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	UINT ncbElementBytes = ((sizeof(CB_monster_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbmonster = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbmonster->Map(0, NULL, (void**)&m_pcbMappedmonster);
}

void Cmonster::ReleaseShaderVariables()
{
	if (m_pCamera)m_pCamera->ReleaseShaderVariables();

	if (m_pd3dcbmonster)
	{
		m_pd3dcbmonster->Unmap(0, NULL);
		m_pd3dcbmonster->Release();
	}

	CGameObject::ReleaseShaderVariables();
}

void Cmonster::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedmonster->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
}

void Cmonster::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD)xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD)xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT)xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT)xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP)xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN)xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void Cmonster::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//m_pCamera->Move(xmf3Shift);
	}
}

void Cmonster::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f)m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f)m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void Cmonster::Update(XMFLOAT3 xmf3Shift, float fTimeElapsed)
{
	//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));
	//float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	//float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	//if (fLength > m_fMaxVelocityXZ)
	//{
	//	m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
	//	m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	//}
	//float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	//fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	//if (fLength > m_fMaxVelocityY)m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//Move(m_xmf3Velocity, false);

	//if (m_pmonsterUpdatedContext)OnmonsterUpdateCallback(fTimeElapsed);

	//DWORD nCurrentCameraMode = m_pCamera->GetMode();
	//if (nCurrentCameraMode == THIRD_PERSON_CAMERA)m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	//if (m_pCameraUpdatedContext)OnCameraUpdateCallback(fTimeElapsed);
	//if (nCurrentCameraMode == THIRD_PERSON_CAMERA)m_pCamera->SetLookAt(m_xmf3Position);
	////m_pCamera->RegenerateViewMatrix();

	//fLength = Vector3::Length(m_xmf3Velocity);
	//float fDeceleration = (m_fFriction * fTimeElapsed);
	//if (fDeceleration > fLength)fDeceleration = fLength;
	//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	////23.01.19
	//if (m_xmf3Position.y > SECOND_FLOOR - 5 && m_xmf3Position.y < FLOOR_SIZE * 2)
	//{
	//	if (m_xmf3Position.y < SECOND_FLOOR)
	//	{
	//		XMFLOAT3 xmf3monsterVelocity = GetVelocity();
	//		xmf3monsterVelocity.y = 0.0f;
	//		SetVelocity(xmf3monsterVelocity);
	//		m_xmf3Position.y = SECOND_FLOOR;
	//		SetPosition(m_xmf3Position);
	//	}
	//}
	//else if (m_xmf3Position.y < FIRST_FLOOR)
	//{
	//	XMFLOAT3 xmf3monsterVelocity = GetVelocity();
	//	xmf3monsterVelocity.y = 0.0f;
	//	SetVelocity(xmf3monsterVelocity);
	//	m_xmf3Position.y = FIRST_FLOOR;
	//	SetPosition(m_xmf3Position);
	//}
	////

	//23.01.19
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z *
		m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity,
		-fDeceleration, true));
	//

	//23.01.03
	//일정한 시간이 지나면 적이 나를 향해서 오게: 델타 t
	srand((unsigned int)time(NULL));
	mpTime += fTimeElapsed;
	XMFLOAT3 tmp = XMFLOAT3(xmf3Shift.x / 10, xmf3Shift.y / 10, xmf3Shift.z / 10);
	if (mpTime > 0.5f)
	{
		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동한다.

		m_xmf3Position = Vector3::Add(m_xmf3Position, tmp);
		mpTime = 0.f;
	}
	//23.01.19
	if (m_xmf3Position.y > SECOND_FLOOR - 5 && m_xmf3Position.y < FLOOR_SIZE)
	{
		if (m_xmf3Position.y < SECOND_FLOOR)
		{
			XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
			xmf3PlayerVelocity.y = 0.0f;
			SetVelocity(xmf3PlayerVelocity);
			m_xmf3Position.y = SECOND_FLOOR;
			SetPosition(m_xmf3Position);
		}
	}
	else if (m_xmf3Position.y < FIRST_FLOOR)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		m_xmf3Position.y = FIRST_FLOOR;
		SetPosition(m_xmf3Position);
	}
	//
}


CCamera* Cmonster::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f)m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	/*if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->Setmonster(this);
	}*/

	if (m_pCamera)delete m_pCamera;

	return(pNewCamera);
}

void Cmonster::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void Cmonster::SetRootParameter(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//23.01.16
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbmonster->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_PLAYER, d3dGpuVirtualAddress);

	//pd3dCommandList->SetGraphicsRootDescriptorTable(ROOT_PARAMETER_OBJECT, m_d3dCbvGPUDescriptorHandle);
	//
}

void Cmonster::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA)CGameObject::Render(pd3dCommandList, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAirplanemonster

CAirplanemonster::CAirplanemonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes) : Cmonster(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pContext, nMeshes)
{
	//m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CAirplaneMeshDiffused* pAirplaneMesh = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 4.0f, XMFLOAT4(0.5f, 0.0f, 0.5f, 0.0f));
	//23.01.05
	SetMesh(0, pAirplaneMesh);
	//SetMesh(pAirplaneMesh);
	//

	UINT ncbElementBytes = ((sizeof(CB_monster_INFO) + 255) & ~255); //256의 배수

	CmonsterShader* pShader = new CmonsterShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 1, NULL, DXGI_FORMAT_D32_FLOAT);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 0);
	pShader->CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbmonster, ncbElementBytes);

	SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pShader);
}

CAirplanemonster::~CAirplanemonster()
{
}

void CAirplanemonster::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	Cmonster::OnPrepareRender(pd3dCommandList, pCamera);

	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

//CCamera* CAirplanemonster::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
//{
//	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
//	if (nCurrentCameraMode == nNewCameraMode)return(m_pCamera);
//	switch (nNewCameraMode)
//	{
//	case FIRST_PERSON_CAMERA:
//		SetFriction(200.0f);
//		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
//		SetMaxVelocityXZ(125.0f);
//		SetMaxVelocityY(400.0f);
//		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
//		m_pCamera->SetTimeLag(0.0f);
//		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
//		m_pCamera->GenerateProjectionMatrix(10.01f, 5000.0f, ASPECT_RATIO, 60.0f);
//		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
//		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
//		break;
//	case SPACESHIP_CAMERA:
//		SetFriction(125.0f);
//		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
//		SetMaxVelocityXZ(400.0f);
//		SetMaxVelocityY(400.0f);
//		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
//		m_pCamera->SetTimeLag(0.0f);
//		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
//		m_pCamera->GenerateProjectionMatrix(10.01f, 5000.0f, ASPECT_RATIO, 60.0f);
//		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
//		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
//		break;
//	case THIRD_PERSON_CAMERA:
//		SetFriction(250.0f);
//		//23.01.29
//		SetGravity(XMFLOAT3(0.0f, -20.0f, 0.0f));
//		//
//		SetMaxVelocityXZ(125.0f);
//		SetMaxVelocityY(400.0f);
//		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
//		m_pCamera->SetTimeLag(0.25f);
//		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
//		m_pCamera->GenerateProjectionMatrix(10.01f, 5000.0f, ASPECT_RATIO, 60.0f);
//		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
//		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
//		break;
//	default:
//		break;
//	}
//	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
//	Update(fTimeElapsed);
//
//	return(m_pCamera);
//}
