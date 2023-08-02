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

		if (dwDirection & DIR_JUMP && onFloor) {
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance * 15);
			onFloor = false;
			m_pSkinnedAnimationController->SetTrackPosition(5, 1.0f);
			//m_pSkinnedAnimationController->SetTrackEnable(m_pSkinnedAnimationController->Cur_Animation_Track, false);
			//m_pSkinnedAnimationController->SetTrackEnable(5, true);
		}
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
		obBox.Center = m_xmf3Position;
		obBox.Center.y += 10.f;
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
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::processAnimation()
{
	m_pSkinnedAnimationController->SetTrackEnable(m_pSkinnedAnimationController->Cur_Animation_Track, false);
	if (onFloor == false) {
		m_pSkinnedAnimationController->SetTrackEnable(5, true);
		return;
	}
	else m_pSkinnedAnimationController->SetTrackPosition(5, 1.0f);

	if (HP > packet_HP) {
		m_pSkinnedAnimationController->SetTrackPosition(3, 0.0f);
		m_pSkinnedAnimationController->SetTrackEnable(3, true);

		HP = packet_HP;
		return;
	}
	HP = packet_HP;

	if (Vector3::Length(m_xmf3Velocity) > 0.f) {
		m_pSkinnedAnimationController->SetTrackEnable(1, true);
	}
	else
	{
		m_pSkinnedAnimationController->SetTrackEnable(0, true);
		m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
	}
}

void CPlayer::Update(float fTimeElapsed)
{
	//if (onAttack || onCollect || onDie) SetMaxVelocityXZ(0.0f);
	//else SetMaxVelocityXZ(100.0f);

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);

	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}


	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);


	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

	Rotate(cxDelta, cyDelta, czDelta);
	Move(xmf3Velocity, false);

	obBox.Center = m_xmf3Position;
	obBox.Center.y += 10.f;
}

void CPlayer::Deceleration(float fTimeElapsed)
{
	float fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	//XMFLOAT3 Decel = Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true);
	//Decel.y = 0;
	//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Decel);
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

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, ID3D12PipelineState* m_pd3dPipelineState, bool shadow, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;

	if (nCameraMode == THIRD_PERSON_CAMERA || true == shadow)
		CGameObject::Render(pd3dCommandList, m_pd3dGraphicsRootSignature, m_pd3dPipelineState, pCamera);
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
	obBox.Center = m_xmf3Position;
	obBox.Center.y += 10.f;
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


	pAngrybotModels[0] = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/swordPl.bin", NULL, 7);

	pAngrybotModels[1] = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/gunPl.bin", NULL, 7);

	pAngrybotModels[2] = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/fistPl.bin", NULL, 7);


	for (int i = 0; i < 3; i++) {
		AnimationControllers[i] = new CAnimationController(pd3dDevice, pd3dCommandList, 6, pAngrybotModels[i]);
	}

	AnimationControllers[0]->m_pAnimationTracks[2].SetSpeed(2.0f);
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

	m_ppBullet = new CBulletObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, NULL, 1, 3);
	m_ppBullet->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_ppBullet->SetScale(0.1f, 0.1f, 0.1f);

	m_ppBullet->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppBullet->SetRotationSpeed(360.0f);
	m_ppBullet->SetMovingSpeed(120.0f);
	m_ppBullet->SetPosition(XMFLOAT3(5000, 5000, 5000));


	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);

	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
	m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);

	m_pSkinnedAnimationController->SetTrackEnable(1, false);

	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, false);
	m_pSkinnedAnimationController->SetTrackEnable(5, false);
	m_pSkinnedAnimationController->SetTrackEnable(0, true);


	//	AnimationControllers[0]->SetCallbackKeys(1, 1);
	//	//AnimationControllers[0]->SetCallbackKeys(2, 1);
	//	AnimationControllers[0]->SetCallbackKeys(3, 1);
	//	AnimationControllers[0]->SetCallbackKeys(4, 1);
	//	AnimationControllers[0]->SetCallbackKeys(5, 1);
	//
	//	AnimationControllers[1]->SetCallbackKeys(1, 1);
	//	AnimationControllers[1]->SetCallbackKeys(2, 1);
	//	AnimationControllers[1]->SetCallbackKeys(3, 1);
	//	AnimationControllers[1]->SetCallbackKeys(4, 1);
	//	AnimationControllers[1]->SetCallbackKeys(5, 1);
	//
	//	AnimationControllers[2]->SetCallbackKeys(1, 1);
	//	//AnimationControllers[2]->SetCallbackKeys(2, 1);
	//	AnimationControllers[2]->SetCallbackKeys(3, 1);
	//	AnimationControllers[2]->SetCallbackKeys(4, 1);
	//	AnimationControllers[2]->SetCallbackKeys(5, 1);
	//
	//#ifdef _WITH_SOUND_RESOURCE
	//	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	//	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	//	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
	//#else
	//
	//	AnimationControllers[0]->SetCallbackKey(1, 0, 0.2f, _T("Sound/walk.wav"));
	//	//AnimationControllers[0]->SetCallbackKey(2, 0, 0.2f, _T("Sound/swordAttack.wav"));
	//	AnimationControllers[0]->SetCallbackKey(3, 0, 0.0333f, _T("Sound/player_damaged.wav"));
	//	AnimationControllers[0]->SetCallbackKey(4, 0, 0.2f, _T("Sound/death.wav"));
	//	AnimationControllers[0]->SetCallbackKey(5, 0, 0.9666f, _T("Sound/Jump.wav"));
	//
	//
	//	AnimationControllers[1]->SetCallbackKey(1, 0, 0.2f, _T("Sound/walk.wav"));
	//	AnimationControllers[1]->SetCallbackKey(2, 0, 0.2f, _T("Sound/gunAttack.wav"));
	//	AnimationControllers[1]->SetCallbackKey(3, 0, 0.0333f, _T("Sound/player_damaged.wav"));
	//	AnimationControllers[1]->SetCallbackKey(4, 0, 0.2f, _T("Sound/death.wav"));
	//	AnimationControllers[1]->SetCallbackKey(5, 0, 0.9666f, _T("Sound/Jump.wav"));
	//
	//
	//	AnimationControllers[2]->SetCallbackKey(1, 0, 0.2f, _T("Sound/walk.wav"));
	//	//AnimationControllers[2]->SetCallbackKey(2, 0, 0.2f, _T("Sound/attack.wav"));
	//	AnimationControllers[2]->SetCallbackKey(3, 0, 0.0333f, _T("Sound/player_damaged.wav"));
	//	AnimationControllers[2]->SetCallbackKey(4, 0, 0.2f, _T("Sound/death.wav"));
	//	AnimationControllers[2]->SetCallbackKey(5, 0, 0.9666f, _T("Sound/Jump.wav"));
	//
	//#endif
	//	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	//
	//	AnimationControllers[0]->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	//	//AnimationControllers[0]->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	//	AnimationControllers[0]->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	//	AnimationControllers[0]->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);
	//	AnimationControllers[0]->SetAnimationCallbackHandler(5, pAnimationCallbackHandler);
	//
	//	AnimationControllers[1]->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	//	AnimationControllers[1]->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	//	AnimationControllers[1]->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	//	AnimationControllers[1]->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);
	//	AnimationControllers[1]->SetAnimationCallbackHandler(5, pAnimationCallbackHandler);
	//
	//	AnimationControllers[2]->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);
	//	//AnimationControllers[2]->SetAnimationCallbackHandler(2, pAnimationCallbackHandler);
	//	AnimationControllers[2]->SetAnimationCallbackHandler(3, pAnimationCallbackHandler);
	//	AnimationControllers[2]->SetAnimationCallbackHandler(4, pAnimationCallbackHandler);
	//	AnimationControllers[2]->SetAnimationCallbackHandler(5, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//m_xmOOBB = BoundingBox(XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(10, 4, 10));

	obBox = BoundingOrientedBox(XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(15, 10, 15), XMFLOAT4(0, 0, 0, 1));



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
		SetFriction(50.0f);
		SetGravity(XMFLOAT3(0.0f, -5.0f, 0.0f));
		SetMaxVelocityXZ(10.0f);
		SetMaxVelocityY(100.f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 5.0f));
		//m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 12.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, -15.0f, 0.0f));
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
		SetFriction(300.0f);
		SetGravity(XMFLOAT3(0.0f, -20.0f, 0.0f));
		SetMaxVelocityXZ(100.0f);
		SetMaxVelocityY(300.f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.2f);
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
	CPlayer::Move(dwDirection, fDistance, bUpdateVelocity);
}



void CTerrainPlayer::Update(float fTimeElapsed)
{
	CPlayer::Update(fTimeElapsed);
}

SoundPlayer::SoundPlayer()
	: xAudio2_(nullptr), masterVoice_(nullptr), sourceVoice_(nullptr)
{
	ZeroMemory(&waveFormat_, sizeof(waveFormat_));
	ZeroMemory(&buffer_, sizeof(buffer_));
}

SoundPlayer::~SoundPlayer()
{
	Terminate();
}

bool SoundPlayer::Initialize()
{
	HRESULT hr;


	// XAudio2 객체 생성
	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr)) {
		return false;
	}



	// 마스터 보이스 생성
	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
	if (FAILED(hr)) {
		return false;
	}


	waveFormat_.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat_.nChannels = 2;
	waveFormat_.nSamplesPerSec = 48000;
	waveFormat_.nAvgBytesPerSec = 48000 * 4;
	waveFormat_.nBlockAlign = 4;
	waveFormat_.wBitsPerSample = 16;
	waveFormat_.cbSize = 0;


	// 소스 보이스 생성
	hr = xAudio2_->CreateSourceVoice(&sourceVoice_, &waveFormat_);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

void SoundPlayer::Terminate()
{
	if (sourceVoice_ != nullptr) {
		sourceVoice_->DestroyVoice();
		sourceVoice_ = nullptr;
	}


	if (masterVoice_ != nullptr) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}

	if (xAudio2_ != nullptr) {
		xAudio2_->Release();
		xAudio2_ = nullptr;
	}
}

HRESULT SoundPlayer::LoadWaveFile(const wchar_t* filename)
{
	// WAV 파일을 읽기 전용으로 열기
	FILE* file = nullptr;
	errno_t err = _wfopen_s(&file, filename, L"rb");
	if (err != 0)
	{
		return HRESULT_FROM_WIN32(err);
	}

	// WAV 파일 헤더 읽기
	WAVEHEADER header;
	size_t bytesRead = fread(&header, 1, sizeof(WAVEHEADER), file);
	if (bytesRead != sizeof(WAVEHEADER))
	{
		fclose(file);
		return E_FAIL;
	}

	// WAV 파일 검증
	if (memcmp(header.chunkId, "RIFF", 4) != 0 ||//0501
		memcmp(header.format, "WAVE", 4) != 0 ||
		memcmp(header.subchunk1Id, "fmt ", 4) != 0 ||
		memcmp(header.subchunk2Id, "data", 4) != 0)
	{
		fclose(file);
		return E_FAIL;
	}

	// 웨이브 형식 정보 읽기
	WAVEFORMATEX* pWaveFormat = (WAVEFORMATEX*)malloc(header.subchunk1Size);
	if (!pWaveFormat)
	{
		fclose(file);
		return E_OUTOFMEMORY;
	}

	bytesRead = fread(pWaveFormat, 1, header.subchunk1Size, file);
	if (bytesRead != header.subchunk1Size)
	{
		free(pWaveFormat);
		fclose(file);
		return E_FAIL;
	}

	// 버퍼 데이터 읽기
	BYTE* pData = (BYTE*)malloc(header.subchunk2Size);
	if (!pData)
	{
		free(pWaveFormat);
		fclose(file);
		return E_OUTOFMEMORY;
	}

	bytesRead = fread(pData, 1, header.subchunk2Size, file);
	if (bytesRead != header.subchunk2Size)
	{
		//free(pWaveFormat);
		//free(pData);
		//fclose(file);
		//return E_FAIL;//0506
	}

	// 반환값 설정
	waveFormat_ = *pWaveFormat;
	buffer_.pAudioData = pData;
	buffer_.AudioBytes = header.subchunk2Size;

	buffer_.Flags = XAUDIO2_END_OF_STREAM; // 스트림의 끝임을 나타냄
	buffer_.LoopCount = XAUDIO2_LOOP_INFINITE; // 루프 횟수를 무한대로 설정


	// 파일 닫기
	fclose(file);

	return S_OK;
}

bool SoundPlayer::LoadWave(const wchar_t* filename, int type = 0)
{
	HRESULT hr;
	// WAVE 파일 로드
	hr = LoadWaveFile(filename);


	if (FAILED(hr)) {
		return false;
	}

	if (type == 1)
	{
		buffer_.Flags = XAUDIO2_END_OF_STREAM;
		buffer_.LoopCount = 0; // 한 번만 재생하고 멈추기 위해 루프 횟수를 0으로 설정
	}

	if (nullptr == sourceVoice_)
	{
		if (false == Initialize())
		{
			// 소스 보이스 생성
			hr = xAudio2_->CreateSourceVoice(&sourceVoice_, &waveFormat_);
			if (FAILED(hr)) {
				cout << "323호에서 오류나면 소스 보이스 생성" << endl;
				return false;
			}
		}

		//Initialize();
	}

	// 소스 보이스에 버퍼 설정
	hr = sourceVoice_->SubmitSourceBuffer(&buffer_);//0504
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

void SoundPlayer::Play()
{
	// 소스 보이스 재생
	if (sourceVoice_)
		sourceVoice_->Start();
}

void SoundPlayer::Stop()
{
	if (sourceVoice_)
	{
		// 소스 보이스 중지
		sourceVoice_->Stop();
		sourceVoice_->FlushSourceBuffers();
	}
	//Terminate();
}


