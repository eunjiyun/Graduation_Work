#pragma once

#include "Timer.h"
#include "Player.h"
#include "Stage.h"
#include "Login.h"
#include <vector>

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
	void CreateRenderTargetViewsAndDepthStencilView();

	void ChangeSwapChainState();

	void BuildObjects();
	void CreateOtherPlayer(int p_id, XMFLOAT3 Pos, XMFLOAT3 Look, XMFLOAT3 Up, XMFLOAT3 Right);
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//23.01.24
	void Scene_Change(SCENEID _eSceneid);
	SCENEID m_eCurrentScene;
	SCENEID m_ePrevScene;

	//23.01.27
	HWND Get_HWNG() { return m_hWnd; }
	LONG Get_OldCursorPointX() { return m_ptOldCursorPos.x; }
	LONG Get_OldCursorPointY() { return m_ptOldCursorPos.y; }

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd;

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;

	IDXGIFactory4* m_pdxgiFactory;
	IDXGISwapChain3* m_pdxgiSwapChain;
	ID3D12Device* m_pd3dDevice;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource* m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	UINT						m_nDsvDescriptorIncrementSize;

	ID3D12CommandAllocator* m_pd3dCommandAllocator;
	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12GraphicsCommandList* m_pd3dCommandList;

	ID3D12Fence* m_pd3dFence;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

	//23.01.03
public:
	float pfClearColor[4];
	bool wakeUp = true;
	//
	//23.01.03
	bool lookPoint = true;
	bool choose = false;
	//

#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController;
#endif

	CLogin* m_pLogin = NULL;
	CStage* m_pStage = NULL;
	CPlayer* m_pPlayer = NULL;

	vector<CPlayer*> Players;
	CCamera* m_pCamera = NULL;

	POINT						m_ptOldCursorPos;

	CGameTimer					m_GameTimer;
	_TCHAR						m_pszFrameRate[50];
};

