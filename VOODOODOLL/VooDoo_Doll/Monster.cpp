#include "stdafx.h"
#include "Monster.h"

CMonster::CMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel,
	int nAnimationTracks)
{
	if (pModel != nullptr) {
		_Model = pModel;

		CLoadedModelInfo* cap = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Warlock_cap.bin", NULL, 7);
		m_ppHat = new CBulletObject(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, cap, 1, 2);
		m_ppHat->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHat->SetScale(0.8f, 0.8f, 0.8f);

		m_ppHat->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppHat->SetRotationSpeed(360.0f);
		m_ppHat->SetMovingSpeed(120.0f);
		m_ppHat->SetPosition(XMFLOAT3(5000, 5000, 5000));
		if (cap) delete cap;

		SetChild(_Model->m_pModelRootObject, true);
		m_pSkinnedAnimationController = new CAnimationController(pd3dDevice, pd3dCommandList, nAnimationTracks, _Model);
	}
}

CMonster::~CMonster()
{
}

void CMonster::Update(float fTimeElapsed)
{
	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

	Move(xmf3Velocity);
}
