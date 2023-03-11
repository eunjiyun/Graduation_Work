//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer()
{
	m_pCamera = NULL;

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

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

		if (dwDirection & DIR_FORWARD)
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);

		if (dwDirection & DIR_BACKWARD)
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);

		if (dwDirection & DIR_RIGHT)
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);

		if (dwDirection & DIR_LEFT)
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

		//if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		//if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);


		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);

	}
}

void CPlayer::Rotate(float x, float y, float z)
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
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
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

void CPlayer::Update(float fTimeElapsed)
{
	if (onAttack || onCollect || onDie) SetMaxVelocityXZ(0.0f);
	else if (onRun) SetMaxVelocityXZ(100.0f);
	else SetMaxVelocityXZ(10.0f);

	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}

	//float fMaxVelocityY = m_fMaxVelocityY;
	//fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	//if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);


	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

	Rotate(cxDelta, cyDelta, czDelta);
	Move(xmf3Velocity, false);
	// 
	//if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	//if (m_xmf3Position.y > SECOND_FLOOR && m_xmf3Position.y < FLOOR_SIZE * 2)
	//{
	//	if (m_xmf3Position.y < SECOND_FLOOR)
	//	{
	//		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
	//		xmf3PlayerVelocity.y = 0.0f;
	//		SetVelocity(xmf3PlayerVelocity);
	//		m_xmf3Position.y = SECOND_FLOOR;
	//		SetPosition(m_xmf3Position);
	//	}
	//}
	//else if (m_xmf3Position.y < FIRST_FLOOR)
	//{
	//	XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
	//	xmf3PlayerVelocity.y = 0.0f;
	//	SetVelocity(xmf3PlayerVelocity);
	//	m_xmf3Position.y = FIRST_FLOOR;
	//	SetPosition(m_xmf3Position);
	//}

}

void CPlayer::Deceleration(float fTimeElapsed)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//if (m_pCameraUpdatedContext)
	//	OnCameraUpdateCallback(fTimeElapsed);

	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	float fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
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
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, ID3D12PipelineState* m_pd3dPipelineState, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//cout << c_id << endl;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
}

void CPlayer::OnUpdateTransform()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void CPlayer::UpdateBoundingBox()
{
	m_xmOOBB.Center = m_xmf3Position;
}

void CPlayer::boundingAnimate(float fElapsedTime)
{
	OnUpdateTransform();
	UpdateBoundingBox();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void* pCallbackData, float fTrackPosition)
{
	_TCHAR* pWavName = (_TCHAR*)pCallbackData;
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
	PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
	PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}

CTerrainPlayer::CTerrainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int choosePl)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);


	pAngrybotModels[0] = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/body26.bin", NULL, 7);
	pAngrybotModels[1] = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/body27.bin", NULL, 7);
	pAngrybotModels[2] = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/body28.bin", NULL, 7);
	
	for (int i = 0; i < 3; i++) {
		AnimationControllers[i] = new CAnimationController(pd3dDevice, pd3dCommandList, 6, pAngrybotModels[i]);
	}

	if (1 == choosePl)
	{
		SetChild(pAngrybotModels[0]->m_pModelRootObject, true);
		m_pSkinnedAnimationController = AnimationControllers[0];
	}
	else if (2 == choosePl)
	{
		SetChild(pAngrybotModels[1]->m_pModelRootObject, true);
		m_pSkinnedAnimationController = AnimationControllers[1];
	}
	else if (3 == choosePl)
	{
		SetChild(pAngrybotModels[2]->m_pModelRootObject, true);
		m_pSkinnedAnimationController = AnimationControllers[2];
	}

	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
	//23.02.20
	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
	m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);
	//
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	//23.02.20
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(0, true);


	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.2f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.5f, _T("Sound/Footstep02.wav"));
	//	m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_xmOOBB = BoundingBox(XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(10, 3, 10));

	SetScale(XMFLOAT3(1.0f, 1.0f, 1.0f));
}

CTerrainPlayer::~CTerrainPlayer()
{
}

CCamera* CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(20.0f);
		SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
		SetMaxVelocityXZ(10.0f);
		SetMaxVelocityY(100.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 40.0f, -100.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}


void CTerrainPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	//m_pSkinnedAnimationController->SetTrackEnable(0, false);

	//m_pSkinnedAnimationController->SetTrackEnable(1, true);
	//m_pSkinnedAnimationController->SetTrackEnable(2, false);
	//m_pSkinnedAnimationController->SetTrackEnable(3, false);
	//m_pSkinnedAnimationController->SetTrackEnable(4, false);
	//m_pSkinnedAnimationController->SetTrackEnable(5, false);


	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}

void CTerrainPlayer::playerAttack(int whatPlayer, CGameObject* pLockedObject, CGameObject*** bulletTmp)
{
	CGameObject* pBulletObject = NULL;

	if (true == onAttack)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, true);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);

		//cout << "2번 트루" << endl;

		if (2 == whatPlayer)
		{

			//for (int i = 0; i < BULLETS; i++)
			for (int i = 0; i < 1; i++)
			{
				if (!(*bulletTmp)[i]->m_bActive)
				{
					pBulletObject = (*bulletTmp)[i];
					break;
				}
			}

			if (pBulletObject)
			{
				XMFLOAT3 xmf3Position = GetPosition();
				//XMFLOAT3 xmf3Direction = GetUp();
				XMFLOAT3 xmf3Direction = GetLook();
				XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, false));

				pBulletObject->m_xmf4x4World = m_xmf4x4World;
				pBulletObject->SetFirePosition(XMFLOAT3(xmf3FirePosition.x, xmf3FirePosition.y + 15, xmf3FirePosition.z));
				pBulletObject->SetMovingDirection(xmf3Direction);
				pBulletObject->SetActive(true);

				if (pLockedObject)
				{
					pBulletObject->m_pLockedObject = pLockedObject;
				}
			}
		}
	}
}
void CTerrainPlayer::playerRun()
{
	if (true == onRun)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, true);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);

		cout << "3번 트루" << endl;
	}
}
void CTerrainPlayer::playerDie()
{
	if (true == onDie)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, true);
		m_pSkinnedAnimationController->SetTrackEnable(5, false);

		//cout << "4번 트루" << endl;
	}
}

void CTerrainPlayer::playerCollect()
{

	if (true == onCollect)
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_pSkinnedAnimationController->SetTrackEnable(1, false);
		m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_pSkinnedAnimationController->SetTrackEnable(4, false);
		m_pSkinnedAnimationController->SetTrackEnable(5, true);

		//cout << "5번 트루" << endl;
	}
}


void CTerrainPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);

	//if (m_pSkinnedAnimationController)
	//{
	//	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	//	if (::IsZero(fLength))//플레이어 좌표에 변화가 없을 때
	//	{
	//		if (false == m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable &&
	//			false ==onRun && //run 키를 떼면 idle 모드로 전환
	//			false == m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable &&
	//			false == m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable)
	//			//if (!onAttack && !onRun && !onDie &&!onCollect)//플레이어가 공격 모드가 아닐 때
	//		{
	//			m_pSkinnedAnimationController->SetTrackEnable(0, true);
	//			m_pSkinnedAnimationController->SetTrackEnable(1, false);
	//			m_pSkinnedAnimationController->SetTrackEnable(2, false);
	//			m_pSkinnedAnimationController->SetTrackEnable(3, false);
	//			m_pSkinnedAnimationController->SetTrackEnable(4, false);
	//			m_pSkinnedAnimationController->SetTrackEnable(5, false);

	//			m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);

	//			//cout << "0번 트루" << endl;
	//		}
	//	}
	//}
}

void CTerrainPlayer::otherPlayerUpdate()//0226
{
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (::IsZero(fLength))//플레이어 좌표에 변화가 없을 때
		{
			if (false == m_pSkinnedAnimationController->m_pAnimationTracks[2].m_bEnable &&
				false == m_pSkinnedAnimationController->m_pAnimationTracks[3].m_bEnable &&
				false == m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable &&
				false == m_pSkinnedAnimationController->m_pAnimationTracks[5].m_bEnable)
				//if (!onAttack && !onRun && !onDie &&!onCollect)//플레이어가 공격 모드가 아닐 때
			{

				if (false == m_pSkinnedAnimationController->m_pAnimationTracks[0].m_bEnable)
					m_pSkinnedAnimationController->SetTrackEnable(0, true);

				m_pSkinnedAnimationController->SetTrackEnable(1, false);
				m_pSkinnedAnimationController->SetTrackEnable(2, false);
				m_pSkinnedAnimationController->SetTrackEnable(3, false);
				m_pSkinnedAnimationController->SetTrackEnable(4, false);
				m_pSkinnedAnimationController->SetTrackEnable(5, false);

				m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);

				//cout << "0번 트루00000" << endl;
			}
		}
	}
}
