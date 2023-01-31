#pragma once

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480

#include "Timer.h"
#include "Player.h"
#include "Stage.h"
#include "Login.h"
#include <vector>

#define DRAW_Scene_COLOR				'S'

#define DRAW_Scene_TEXTURE				'T'
#define DRAW_Scene_LIGHTING				'L'
#define DRAW_Scene_NORMAL				'B' //'N'
#define DRAW_Scene_Z_DEPTH				'Z'
#define DRAW_Scene_DEPTH				'G' //'D'

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

	void CreateSwapChainRenderTargetViews();
	void CreateDepthStencilView();

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

	HWND	Get_HWND() { return m_hWnd; }
	void			Change_Scene(SCENEID _eSceneid);
	LONG		Get_OldCursorPointX() { return m_ptOldCursorPos.x; }
	LONG		Get_OldCursorPointY() { return m_ptOldCursorPos.y; }

private:
	SCENEID m_eCurrentScene;
	SCENEID m_ePrevScene;

private:
	HINSTANCE						m_hInstance;
	HWND							m_hWnd;

	int								m_nWndClientWidth;
	int								m_nWndClientHeight;

	IDXGIFactory4* m_pdxgiFactory = NULL;
	IDXGISwapChain3* m_pdxgiSwapChain = NULL;
	ID3D12Device* m_pd3dDevice = NULL;

	bool							m_bMsaa4xEnable = false;
	UINT							m_nMsaa4xQualityLevels = 0;

	static const UINT				m_nSwapChainBuffers = 2;
	UINT							m_nSwapChainBufferIndex = 0;

	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];

	ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;

	ID3D12CommandAllocator* m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue* m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;

	ID3D12Fence* m_pd3dFence = NULL;
	UINT64							m_nFenceValues[m_nSwapChainBuffers];
	HANDLE							m_hFenceEvent;

	CCamera* m_pCamera = NULL;

public:
	CPlayer*							m_pPlayer = NULL;
	CLogin*							m_pLogin = NULL;
	CStage*							m_pScene = NULL;
	vector<CPlayer*>			Players;
	CGameTimer					m_GameTimer;

	CPostProcessingShader* m_pPostProcessingShader = NULL;

	int								m_nDrawOptions = DRAW_Scene_COLOR;

	POINT							m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[50];

	//23.01.28
public:
	bool wakeUp = true;
	//
};

