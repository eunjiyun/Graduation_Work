#pragma once

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480

#include "Timer.h"
#include "Player.h"
#include "Stage.h"
#include "Login.h"
#include <vector>
#include <queue>
#include <array>


class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void BuildObjects();
	void ReleaseObjects();

	// SERVER
	void CreateOtherPlayer(int p_id, XMFLOAT3 Pos, XMFLOAT3 Look, XMFLOAT3 Up, XMFLOAT3 Right);
	void SummonMonster(int npc_id, int type, XMFLOAT3 Pos);

	/*HWND Get_HWNG() { return m_hWnd; }
	LONG Get_OldCursorPointX() { return m_ptOldCursorPos.x; }
	LONG Get_OldCursorPointY() { return m_ptOldCursorPos.y; }*/
	HWND	Get_HWND() { return m_hWnd; }
	void			Change_Scene(SCENEID _eSceneid);
	LONG		Get_OldCursorPointX() { return m_ptOldCursorPos.x; }
	LONG		Get_OldCursorPointY() { return m_ptOldCursorPos.y; }

	void AnimateObjects(float fTimeElapsed);
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void Scene_Change(SCENEID _eSceneid);
	SCENEID m_eCurrentScene;
	SCENEID m_ePrevScene;

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd;

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;

	IDXGIFactory4* m_pdxgiFactory = NULL;
	IDXGISwapChain3* m_pdxgiSwapChain = NULL;
	ID3D12Device* m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;

	ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator* m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue* m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;

	ID3D12Fence* m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

public:
	bool wakeUp = true;
	int whatPlayer = 1;
	CGameObject* m_pLockedObject = NULL;

	queue<CLoadedModelInfo*> pMonsterModel[6];// , pMonsterModel2, pMonsterModel3, pMonsterModel4, pMonsterModel5, pMonsterModel6 = NULL;
	queue<CLoadedModelInfo*> MagiciansHat;
	array<char*, 6> binFileNames = { "Model/Voodoo1.bin", "Model/Voodoo2.bin", "Model/Voodoo3.bin", "Model/Voodoo4.bin",
	"Model/Voodoo5.bin" ,"Model/Voodoo6.bin" };
#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	CStage* m_pStage = NULL;

	CLogin* m_pLogin = NULL;

	CPlayer* m_pPlayer = NULL;
	vector<CPlayer*> Players;
	vector<CMonster*> Monsters;
	int									m_nHierarchicalGameObjects = 0;
	CGameObject** m_ppHierarchicalGameObjects = NULL;

	CCamera* m_pCamera = NULL;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	CGameObject** m_ppBullets = NULL;//총알
	CGameObject** m_ppCap = NULL;//마법모자
	bool onFullScreen = false;
};

