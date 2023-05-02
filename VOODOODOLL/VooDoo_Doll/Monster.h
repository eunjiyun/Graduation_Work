#pragma once
#include "Object.h"

class CMonster : public CGameObject
{
public:
	CLoadedModelInfo* _Model;
	CGameObject* m_ppHat = nullptr;
	CLoadedModelInfo* Hat_Model;
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_hats_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float speed;

	CMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CMonster();
	virtual void Update(float fTimeElapsed);
};

