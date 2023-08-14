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

#define DRAW_SCENE_COLOR				   'P'

#define DRAW_SCENE_TEXTURE				'T'
#define DRAW_SCENE_LIGHTING				'Y'
#define DRAW_SCENE_NORMAL				'U'
#define DRAW_SCENE_Z_DEPTH				'I'
#define DRAW_SCENE_DEPTH					'O'


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
	void CreateSwapChainRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void BuildObjects();
	void ReleaseObjects();

	// SERVER
	void CreateOtherPlayer(int p_id, XMFLOAT3 Pos);
	void SummonMonster(int npc_id, int type, XMFLOAT3 Pos);

	HWND	Get_HWND() { return m_hWnd; }
	void			Change_Scene(SCENEID _eSceneid);
	LONG		Get_OldCursorPointX() { return m_ptOldCursorPos.x; }
	LONG		Get_OldCursorPointY() { return m_ptOldCursorPos.y; }
	void		Set_OldCursorPoint(LONG in_x, LONG in_y) { m_ptOldCursorPos.x = in_x; m_ptOldCursorPos.y = in_y; }


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


	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dRtvCPUDescriptorHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSwapChainBackBufferRTVCPUHandles[m_nSwapChainBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDescriptorCPUHandle;

	ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;

	ID3D12CommandAllocator* m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue* m_pd3dCommandQueue = NULL;
	static ID3D12GraphicsCommandList* m_pd3dCommandList;// = NULL;

	ID3D12Fence* m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

public:
	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	vector<char> userId;
	vector<char> userPw;
	bool delUser = false;

	int signIn = -1;
	bool loginSign[2] = { false,false };

	int gameButton = -1;
	bool lobby[3] = { false,false,false };

	vector<CGameObject*> findItem;
	bool idSet = false;

	queue<CLoadedModelInfo*> pMonsterModel[4];// , pMonsterModel2, pMonsterModel3, pMonsterModel4, pMonsterModel5, pMonsterModel6 = NULL;
	queue<CLoadedModelInfo*> MagiciansHat;
	array<char*, 4> binFileNames = { "Model/Voodoo1.bin", "Model/Voodoo2.bin", "Model/Voodoo5.bin", "Model/boss.bin" };

#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;
	SoundPlayer sound[4];
	SoundPlayer hitSound;
	SoundPlayer doorSound;
	SoundPlayer playerSound;
	bool checkDoor[6] = { false,false,false,false,false,false };
	bool checkDoorSound = false;

	int checkJump = 0;
	int curStage = -1;
	bool gameEnd = false;

	const wchar_t* inGame = _T("Sound/inGame.wav");
	const wchar_t* opening = _T("Sound/opening.wav");
	const wchar_t* closing = _T("Sound/closing.wav");
	const wchar_t* win = _T("Sound/win.wav");
	const wchar_t* monster[4] = { _T("Sound/monsterSummon.wav"),_T("Sound/monsterAttack.wav"),_T("Sound/monsterDeath.wav"),_T("Sound/hit_marker.wav") };
	const wchar_t* hit_marker = _T("Sound/hit_marker.wav");
	const wchar_t* door = _T("Sound/door.wav");
	const wchar_t* jump = _T("Sound/jump.wav");

	float monsterSound_CheckTime = 0.f;

	CStage* m_pStage = NULL;
	LIGHT* m_pLights = NULL;


	CLogin* m_pLogin = NULL;

	CPlayer* m_pPlayer = NULL;
	vector<CPlayer*> Players;
	short damagedMon = -1;
	CCamera* m_pCamera = NULL;
	float beforeHp[3] = { 5000,5000,5000 };

	float bloodTime = 0.0f;
	float plTime[3] = { 0.0f,0.0f,0.0f };
	float blurTime = 0.f;
	float popUpTime = 0.0f;
	bool openDoor[7] = { false,false,false,false,false,false,false };

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	CGameObject** m_ppBullets = NULL;
	CGameObject** m_ppCap = NULL;
	CPostProcessingShader* m_pPostProcessingShader = NULL;

	bool onFullScreen = false;

	int curAtt[3] = { 0,0,0 };
};

