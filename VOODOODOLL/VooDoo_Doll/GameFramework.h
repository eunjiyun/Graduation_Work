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
	void CreateOtherPlayer(int p_id, short type, XMFLOAT3 Pos, XMFLOAT3 Look, XMFLOAT3 Up, XMFLOAT3 Right);
	void SummonMonster(int npc_id, int type, XMFLOAT3 Pos);

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
	static ID3D12GraphicsCommandList* m_pd3dCommandList;// = NULL;

	ID3D12Fence* m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

public:
	bool wakeUp = true;
	char input;
	bool userName[4] = { false,false,false,false };
	vector<char> userId;

	int gameButton = -1;
	bool exit = false;
	bool firstFloor = false;
	CGameObject* temp = nullptr;
	vector<CGameObject*> findItem;

	void RenderText();
	Text* m_Test;
	ComPtr<ID2D1DeviceContext2> d2dDeviceContext;
	ComPtr<IDWriteTextFormat> textFormat;
	ComPtr<ID2D1SolidColorBrush> textBrush;
	


	queue<CLoadedModelInfo*> pMonsterModel[6];// , pMonsterModel2, pMonsterModel3, pMonsterModel4, pMonsterModel5, pMonsterModel6 = NULL;
	queue<CLoadedModelInfo*> MagiciansHat;
	array<char*, 6> binFileNames = { "Model/Voodoo1.bin", "Model/Voodoo2.bin", "Model/Voodoo5.bin", "Model/Voodoo4.bin",
	 "Model/Voodoo3.bin","Model/Voodoo6.bin" };
#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;
	SoundPlayer sound[4];

	SoundPlayer monsterSound;
	SoundPlayer doorSound;
	SoundPlayer playerSound;
	bool checkDoor[6] = { false,false,false,false,false,false };
	bool checkDoorSound = false;
	bool checkDoorSound2 = false;
	int checkJump = 0;
	int curStage = -1;


	const wchar_t* inGame = _T("Sound/inGame.wav");
	const wchar_t* opening = _T("Sound/opening.wav");
	const wchar_t* closing = _T("Sound/closing.wav");
	const wchar_t* win = _T("Sound/win.wav");
	const wchar_t* monster = _T("Sound/monster.wav");
	//const wchar_t* monsterDie = _T("Sound/mob10die.wav");
	const wchar_t* door = _T("Sound/door.wav");
	const wchar_t* jump = _T("Sound/jump.wav");


	CStage* m_pStage = NULL;
	LIGHT* m_pLights = NULL;


	CLogin* m_pLogin = NULL;

	CPlayer* m_pPlayer = NULL;
	vector<CPlayer*> Players;
	vector<CMonster*> Monsters;
	CCamera* m_pCamera = NULL;

	float time = 0.f;
	bool openDoor[7] = { false,false,false,false,false,false,false };


	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];

	CGameObject** m_ppBullets = NULL;//�Ѿ�
	CGameObject** m_ppCap = NULL;//��������
	bool onFullScreen = false;
};

