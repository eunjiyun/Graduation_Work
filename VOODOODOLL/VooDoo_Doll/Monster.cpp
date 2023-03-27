#include "stdafx.h"
#include "Monster.h"

CMonster::CMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel,
	int nAnimationTracks)
{
	if (pModel != nullptr) {
		_Model = pModel;

		//23.02.05
		//if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Voodoo5.bin", NULL);
		//switch (whatMonster)
		//{
		//case 1:
		//	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Voodoo19.bin", NULL, 1);
		//	break;
		//case 2:
		//	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Voodoo23.bin", NULL, 2);
		//	break;
		//case 3:
		//	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Voodoo31.bin", NULL, 3);
		//	break;
		//case 4:
		//	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Voodoo41.bin", NULL, 4);
		//	break;
		//case 5:
		//	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Voodoo52.bin", NULL, 5);
		//	break;
		//case 6:
		//	if (!pMonsterModel) pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Voodoo62.bin", NULL, 6);
		//	break;
		//}
		//

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
