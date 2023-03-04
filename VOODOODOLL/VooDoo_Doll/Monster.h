#pragma once
#include "Object.h"

class CMonster : public CGameObject
{
public:
	CMonster(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CLoadedModelInfo* pModel, int nAnimationTracks, int whatMonster);
	virtual ~CMonster();
};

