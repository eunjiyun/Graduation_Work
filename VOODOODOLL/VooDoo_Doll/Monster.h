#pragma once
#include "Object.h"

class CMonster : public CGameObject
{
public:
	CLoadedModelInfo* _Model;
	CGameObject* m_ppHat;

	CMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks);
	virtual ~CMonster();
};

