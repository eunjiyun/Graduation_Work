//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"



ID3D12GraphicsCommandList* CGameFramework::m_pd3dCommandList = NULL;

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dCommandList = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pStage = NULL;
	m_pLogin = NULL;
	m_pPlayer = NULL;
	m_eCurrentScene = SCENE_STAGE;

	m_ePrevScene = SCENE_LOGIN;
	m_eCurrentScene = SCENE_STAGE;

	_tcscpy_s(m_pszFrameRate, _T("VoodooDoll ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	CoInitialize(NULL);

	BuildObjects();

	return(true);
}

void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;//전체화면 모드에서 바탕화면의 해상도를 스왑체인(후면버퍼)의 크기에 맞게 변경한다. 



	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
#endif

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	//full screen
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);

	IDXGIAdapter1* pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void**)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	HRESULT hResult;

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	::gnRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	::gnDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += ::gnRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, d3dDsvCPUDescriptorHandle);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);


	for (int i = 0; i < m_nSwapChainBuffers; i++)
		if (m_ppd3dSwapChainBackBuffers[i])
			m_ppd3dSwapChainBackBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth,
		m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);


	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pStage) m_pStage->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);


		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pStage) m_pStage->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			exit = true;
			if (onFullScreen)
				ChangeSwapChainState();

			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::Scene_Change(SCENEID _eSceneid)
{
	switch (_eSceneid)
	{
		/*case SCENE_OPEN:
			m_pScene = new CLogo;
			break;*/

	case SCENE_LOGIN:
		m_pLogin = new CLogin;
		break;

	case SCENE_STAGE:
		m_pStage = new CStage;
		break;

		//case SCENE_END:
		//	m_pScene = new CMyEdit;
		//	break;
	}
}

void CGameFramework::Change_Scene(SCENEID _eSceneid)
{
	switch (_eSceneid)
	{
		/*case SCENE_OPEN:
			m_pScene = new CLogo;
			break;*/

	case SCENE_LOGIN:
		m_pLogin = new CLogin;
		break;

	case SCENE_STAGE:
		m_pStage = new CStage;
		break;

		//case SCENE_END:
		//	m_pScene = new CMyEdit;
		//	break;
	}
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

#define _WITH_TERRAIN_PLAYER





void CGameFramework::BuildObjects()
{
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	Scene_Change(m_eCurrentScene);

	m_pStage->hpUi = new CGameObject * [2];
	m_pStage->hpUi[0] = new CGameObject(1);
	m_pStage->hpUi[1] = new CGameObject(1);

	m_pStage->userId = new CGameObject * [10];
	for(int i{};i<10;++i)
		m_pStage->userId[i] = new CGameObject(1);
	

	if (m_pLogin) m_pStage->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
	if (m_pStage)
	{
		m_pStage->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
	}

	m_pStage->hpUi[0]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[94]->m_ppMeshes[0]);
	m_pStage->hpUi[0]->SetScale(0.05f, 0.3f, 0.1f);

	m_pStage->hpUi[1]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[94]->m_ppMeshes[0]);
	m_pStage->hpUi[1]->SetScale(0.05f, 0.3f, 0.7f);

	for (int i{}; i < 10; ++i)
	{
		m_pStage->userId[i]->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[94]->m_ppMeshes[0]);
		m_pStage->userId[i]->SetScale(0.05f, 0.3f, 0.1f);
		m_pStage->userId[i]->SetPosition(50, -55, 178+25*i);
	}
	m_pStage->userId[4]->m_ppMaterials[0] = m_pStage->userId[1]->m_ppMaterials[0];//d
	m_pStage->userId[5]->m_ppMaterials[0] = m_pStage->userId[2]->m_ppMaterials[0];//d
	m_pStage->userId[6]->m_ppMaterials[0] = m_pStage->userId[2]->m_ppMaterials[0];//d
	m_pStage->userId[7]->m_ppMaterials[0] = m_pStage->userId[2]->m_ppMaterials[0];//d
	m_pStage->userId[8]->m_ppMaterials[0] = m_pStage->userId[2]->m_ppMaterials[0];//d
	m_pStage->userId[9]->m_ppMaterials[0] = m_pStage->userId[3]->m_ppMaterials[0];//d

	// Initialize SoundPlayer
	sound[0].Initialize();
	sound[0].LoadWave(inGame);


	sound[1].Initialize();
	sound[1].LoadWave(opening);


	sound[2].Initialize();
	sound[2].LoadWave(closing);


	sound[3].Initialize();
	sound[3].LoadWave(win);

	/*doorSound.Initialize();
	doorSound.LoadWave(door);

	playerSound.Initialize();
	playerSound.LoadWave(jump);

	monsterSound.Initialize();
	monsterSound.LoadWave(monster);*/

	temp = new CGameObject(1);
	temp->m_ppMaterials[0] = m_pStage->m_ppShaders[0]->gameMat[0];
	temp->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[94]->m_ppMeshes[0]);

	temp->SetPosition(84, 75, 108);
	temp->Rotate(270, 0, 0);
	temp->SetScale(0.7f, 0.7f, 0.7f);





	string g_UserName = "VooDooDoll";

	
	//m_Test = new Text(TextureKey::BATTLE_UI_ENGFONT, 1310, 110, g_UserName);

	//// Direct2D 초기화
	//ComPtr<ID2D1Factory3> d2dFactory;
	//HRESULT h = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactory);

	//ComPtr<IDXGIDevice> dxgiDevice;
	//h = m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));

	//if(SUCCEEDED(h))
	//{
	//	// IDXGIDevice 인터페이스 지원됨
	//	// 추가적인 작업 수행
	//	// 예: dxgiDevice->GetAdapter() 등

	//	cout << "성공" << endl;
	//}
	//else
	//{
	//	// IDXGIDevice 인터페이스 지원되지 않음
	//	// 처리 방법 결정

	//	cout << "실패" << endl;
	//}

	//ComPtr<ID2D1Device2> d2dDevice;
	//h = d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice);

	//ComPtr<ID2D1Device> d2dDevice;
	//h = d2dFactory->CreateDevice(m_pd3dDevice, &d2dDevice);

	//
	//h = d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext);


	//// Direct2D 초기화
	//ComPtr<ID2D1Device> d2dDevice;
	//HRESULT h = D2D1CreateDevice(m_pd3dDevice, D2D1_CREATION_PROPERTIES(), &d2dDevice);

	//ComPtr<ID2D1DeviceContext> d2dDeviceContext;
	//d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext);

	//// DWrite 팩토리 생성
	//IDWriteFactory* dwriteFactory;
	//h = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)(&dwriteFactory));

	//
	//h = dwriteFactory->CreateTextFormat(
	//	L"Arial", // 폰트 이름
	//	nullptr, // 폰트 컬렉션
	//	DWRITE_FONT_WEIGHT_NORMAL,
	//	DWRITE_FONT_STYLE_NORMAL,
	//	DWRITE_FONT_STRETCH_NORMAL,
	//	16.0f, // 폰트 크기
	//	L"en-US", // 로케일
	//	&textFormat
	//);

	//// 텍스트 색상 지정
	//D2D1_COLOR_F textColor = D2D1::ColorF(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f));
	//
	//h = d2dDeviceContext->CreateSolidColorBrush(textColor, &textBrush);
	
	m_pStage->text = new CGameObject(1);
	
	m_pStage->text->m_ppMeshes[0] = new CMesh;
	m_pStage->text->m_ppMaterials[0] = m_pStage->texMat;


	//text->SetMesh(0, m_pStage->m_ppShaders[0]->m_ppObjects[94]->m_ppMeshes[0]);


	m_pLights = m_pStage->m_pLights;

	DXGI_FORMAT RtvFormats[5] = { DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_FLOAT };


	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 10; j++) {
			pMonsterModel[i].push(
				CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), binFileNames[i], NULL, i + 1));
		}
	}

	for (int i = 0; i < 10; i++) {
		MagiciansHat.push(CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Warlock_cap.bin", NULL, 7));
	}




#ifdef _WITH_TERRAIN_PLAYER
	m_pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 1);
#else
	CAirplanePlayer* pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL);
	pPlayer->SetPosition(XMFLOAT3(425.0f, 240.0f, 640.0f));
#endif


	m_pStage->m_pPlayer = m_pPlayer;
	m_pCamera = m_pPlayer->GetCamera();


	Players.push_back(m_pPlayer);

	for (int i = 0; i < 2; i++) {
		CTerrainPlayer* pAirplanePlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 1);
		Players.push_back(pAirplanePlayer);
	}
	CGameObject* t = new CBulletObject(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, 1, 1);






	m_pStage->m_pDepthRenderShader = new CDepthRenderShader(m_pStage->pBoxShader, m_pLights);
	//DXGI_FORMAT pdxgiRtvFormats[5] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT };
	m_pStage->m_pDepthRenderShader->CreateShader(m_pd3dDevice, m_pStage->GetGraphicsRootSignature(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 5, RtvFormats, DXGI_FORMAT_D32_FLOAT);
	m_pStage->m_pDepthRenderShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList, NULL);


	m_pStage->m_pShadowShader = new CShadowMapShader(m_pStage->pBoxShader);
	m_pStage->m_pShadowShader->CreateShader(m_pd3dDevice, m_pStage->GetGraphicsRootSignature(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 5, NULL, DXGI_FORMAT_D32_FLOAT);//pipelinestate null
	m_pStage->m_pShadowShader->BuildObjects(m_pd3dDevice, m_pd3dCommandList, m_pStage->m_pDepthRenderShader->GetDepthTexture());

	m_pStage->m_pShadowMapToViewport = new CTextureToViewportShader();
	m_pStage->m_pShadowMapToViewport->CreateShader(m_pd3dDevice, m_pStage->GetGraphicsRootSignature(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, 1, NULL, DXGI_FORMAT_D24_UNORM_S8_UINT);



	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pStage) m_pStage->ReleaseUploadBuffers();
	if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();



	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (!exit)
	{
		if (m_pPlayer) m_pPlayer->Release();


		for (auto& player : Players)
			delete player;
	}

	for (auto& monster : Monsters)
		delete monster;

	if (sound)
	{
		for (int i{}; i < 4; ++i)
			sound[i].~SoundPlayer();
		//delete sound;
	}
	playerSound.~SoundPlayer();
	monsterSound.~SoundPlayer();
	doorSound.~SoundPlayer();



	if (m_pStage) m_pStage->ReleaseObjects();
	if (m_pStage) delete m_pStage;

	if (m_pLogin) m_pLogin->ReleaseObjects();
	if (m_pLogin) delete m_pLogin;
}

void CGameFramework::CreateOtherPlayer(int p_id, short type, XMFLOAT3 Pos, XMFLOAT3 Look, XMFLOAT3 Up, XMFLOAT3 Right)
{
	for (auto& player : Players)
		if (player->c_id < 0) {
			player->c_id = p_id;
			player->SetPosition(Pos);
			player->cur_weapon = type;
			player->SetLookVector(Look);
			player->SetUpVector(Up);
			player->SetRightVector(Right);
			cout << player->c_id << endl;
			break;
		}
}

void CGameFramework::SummonMonster(int npc_id, int type, XMFLOAT3 Pos)
{
	// 이 함수에서 몬스터를 동적 할당하여 소환함
	if (pMonsterModel[type].empty()) {
		cout << "생성실패\n";
		return;
	}
	CMonster* Mon = nullptr;
	CLoadedModelInfo* Hat = nullptr;
	CLoadedModelInfo* Model = pMonsterModel[type].front();


	// Client's monster speed = Server's monster speed * 3 / 10	-> 클라는 30ms, 서버는 100ms 주기로 업데이트하기 때문
	pMonsterModel[type].pop();
	switch (type)
	{
	case 0:
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 4); //손에 칼	
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);
		Mon->speed = 12.f;
		Mon->SetScale(1.0f, 1.0f, 1.0f);

		break;
	case 1:
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 4);//뼈다귀 다리
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);
		Mon->speed = 15.f;
		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;
	case 2:
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 4); // 마술사
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);

		Hat = MagiciansHat.front();
		MagiciansHat.pop();
		Mon->Hat_Model = Hat;
		Mon->m_ppHat = new CBulletObject(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Mon->Hat_Model, 1, 2);
		Mon->m_ppHat->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_ppHat->SetScale(0.8f, 0.8f, 0.8f);
		Mon->speed = 9.f;
		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;
	case 3:
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 5);//손에 바늘
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(4, false);
		Mon->speed = 15.f;
		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;
	case 4:
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 2);//귀신
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, false);
		Mon->speed = 15.f;
		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;
	case 5:
		Mon = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), Model, 4);// 머리에 바늘
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		Mon->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);

		Mon->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		Mon->m_pSkinnedAnimationController->SetTrackEnable(3, false);
		Mon->speed = 15.f;
		Mon->SetScale(1.0f, 1.0f, 1.0f);
		break;
	default:
		break;
	}
	if (Mon != nullptr)
	{
		Mon->c_id = npc_id;
		Mon->npc_type = type;
		Mon->m_xmOOBB = BoundingBox(Pos, XMFLOAT3(15, 20, 12));
		Mon->SetPosition(Pos);
		Monsters.push_back(Mon);
		//cout << Mon->npc_type << "type, " << Mon->c_id << "number Monster SUMMONED - ";
		//Vector3::Print(Mon->GetPosition());

		if (0 == Mon->c_id % 10)
		{
			monsterSound.Initialize();
			monsterSound.LoadWave(monster);
			monsterSound.Play();
		}
	}
}


void CGameFramework::AnimateObjects(float fTimeElapsed)
{
	if (m_pStage) m_pStage->AnimateObjects(fTimeElapsed);

	for (auto& player : Players)
	{
		if (player->c_id > -1) {

			if (5 == player->m_pSkinnedAnimationController->Cur_Animation_Track)
			{
				if (player->m_xmf3Velocity.y <= 0)
				{
					if (0 != checkJump % 2)
					{
						playerSound.Stop();
						playerSound.Terminate();
						++checkJump;
					}
				}
				else
				{
					if (0 == checkJump % 2)
					{
						playerSound.Initialize();
						playerSound.LoadWave(jump);
						playerSound.Play();
						++checkJump;
					}
				}

			}
			player->boundingAnimate(fTimeElapsed);
			player->Animate(fTimeElapsed, true);
		}
	}
	for (auto& monster : Monsters)
	{
		if (monster->c_id > -1)
		{
			/*if (3 == monster->m_pSkinnedAnimationController->Cur_Animation_Track)
			{
				if (curId != monster->c_id)
				{
					monsterSound[1].Initialize();
					monsterSound[1].LoadWave(monsterDie);
					monsterSound[1].Play();
					curId = monster->c_id;
					cout << "몬스터 플레이" << endl;
				}
			}*/

			monster->Animate(fTimeElapsed, false);
			//클라그림자
			/*monster->m_pSkinnedAnimationController->SetTrackEnable(0, false);
			monster->m_pSkinnedAnimationController->SetTrackEnable(2, true);

			time += fTimeElapsed;
			if (time > 3.0f)
			{
				monster->SetPosition(m_pPlayer->GetPosition());
				time = 0.f;
			}*/

			/*cout << "x : " << monster->GetPosition().x << endl;
			cout << "y : " << monster->GetPosition().y << endl;
			cout << "z : " << monster->GetPosition().z << endl << endl << endl;*/
		}
	}

	if (0 == Monsters.size())
	{
		if (monsterSound.sourceVoice_)
		{
			monsterSound.Stop();
			monsterSound.Terminate();
		}
	}

	for (int i{}; i < 7; ++i)
	{
		if (openDoor[i])
		{
			if (2 >= i)
			{
				if (curStage == i - 1)
					curStage = i;

				if (2 != i)
				{
					if (m_pStage->m_ppShaders[0]->door[i]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_fPosition
						== m_pStage->m_ppShaders[0]->door[i]->m_pSkinnedAnimationController->m_pAnimationSets->
						m_pAnimationSets[m_pStage->m_ppShaders[0]->door[i]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_nAnimationSet]->m_fLength
						&& i == curStage)
					{
						if (false == checkDoorSound)
						{
							doorSound.Stop();
							doorSound.Terminate();
							checkDoorSound = true;
						}
					}
					else
					{
						if (false == checkDoor[i])
						{
							doorSound.Initialize();
							doorSound.LoadWave(door);
							doorSound.Play();

							checkDoor[i] = true;
							checkDoorSound = false;
						}
					}

					m_pStage->m_ppShaders[0]->door[i]->Animate(fTimeElapsed, false);
				}
			}
			else
			{

				if (curStage == i - 2)
					curStage = i - 1;

				if (m_pStage->m_ppShaders[0]->door[i - 1]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_fPosition
					== m_pStage->m_ppShaders[0]->door[i - 1]->m_pSkinnedAnimationController->m_pAnimationSets->
					m_pAnimationSets[m_pStage->m_ppShaders[0]->door[i - 1]->m_pSkinnedAnimationController->m_pAnimationTracks[0].m_nAnimationSet]->m_fLength
					&& i == curStage + 1)
				{

					if (false == checkDoorSound2)
					{
						doorSound.Stop();
						doorSound.Terminate();
						checkDoorSound2 = true;
					}
				}
				else
				{
					if (false == checkDoor[i - 1])
					{

						doorSound.Initialize();
						doorSound.LoadWave(door);
						doorSound.Play();

						checkDoor[i - 1] = true;
						checkDoorSound2 = false;
					}
				}


				m_pStage->m_ppShaders[0]->door[i - 1]->Animate(fTimeElapsed, false);
			}
		}
	}
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

//#define _WITH_PLAYER_TOP

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(30.0f);//30프레임

	float fTimeElapsed = m_GameTimer.GetTimeElapsed();
	// Play sound
	sound[1].Play();//오프닝


	// Wait for sound to finish
	//Sleep(10000);

	//if (-200 > m_pPlayer->GetPosition().y && 400 > m_pPlayer->GetPosition().z)
	//{
	//	monsterSound.Stop();//몬스터
	//	monsterSound.Terminate();
	//}

	// else if (-200 > m_pPlayer->GetPosition().y && 400 > m_pPlayer->GetPosition().z)
	// {
	// 	temp->m_ppMaterials[0] = m_pStage->m_ppShaders[0]->gameMat[1];
	// 	temp->SetPosition(880, -70, 800);
	// 	m_pCamera->SetPosition(XMFLOAT3(800, -150, 700));

	// 	monsterSound.Stop();//몬스터
	// 	sound[0].Stop();//인게임
	// 	sound[3].Play();//윈
	// }






	//// 텍스트 출력
	//D2D1_RECT_F textRect = D2D1::RectF(100.0f, 100.0f, 300.0f, 200.0f); // 텍스트 위치 및 크기
	//d2dDeviceContext->DrawText(
	//	L"Hello, World!", // 출력할 텍스트
	//	wcslen(L"Hello, World!"), // 텍스트 길이
	//	textFormat.Get(), // 텍스트 포맷
	//	textRect, // 출력 영역
	//	textBrush.Get() // 텍스트 브러시
	//);

	//// 텍스트 출력
	//d2dDeviceContext->BeginDraw();
	//d2dDeviceContext->SetTransform(D2D1::IdentityMatrix());
	//d2dDeviceContext->DrawText(
	//	L"VooDooDoll",
	//	9,
	//	textFormat,
	//	D2D1::RectF(0, 0, 200, 200),
	//	textBrush);
	//d2dDeviceContext->EndDraw();

	//m_pStage->text->RenderText(m_pd3dCommandList, m_pd3dDevice, m_pCamera);
	//RenderText();




	for (auto& player : Players) {
		if (player->c_id > -1) {
			player->Update(fTimeElapsed);
			m_pStage->CheckMoveObjectsCollisions(fTimeElapsed, player, Monsters, Players);
			// 문과의 충돌처리
			//m_pStage->CheckDoorCollisions( fTimeElapsed, player);
			m_pStage->CheckObjectByObjectCollisions(fTimeElapsed, player);
			m_pStage->Lighthing(player);
			m_pStage->Pushing_Button(player);
			player->Deceleration(fTimeElapsed);
		}
	}



	// hWnd는 게임 창의 윈도우 핸들입니다.
	RECT rcWindow;
	GetWindowRect(Get_HWND(), &rcWindow);

	// rcWindow 변수에는 윈도우 창의 위치와 크기가 저장됩니다.
	int windowX = rcWindow.left;
	int windowY = rcWindow.top;

	if (false == onFullScreen)
	{
		if (526 <= m_ptOldCursorPos.x - windowX && 589 >= m_ptOldCursorPos.x - windowX
			&& 302 <= m_ptOldCursorPos.y - windowY && 335 >= m_ptOldCursorPos.y - windowY)
			gameButton = 1;
		else if (528 <= m_ptOldCursorPos.x - windowX && 587 >= m_ptOldCursorPos.x - windowX
			&& 372 <= m_ptOldCursorPos.y - windowY && 409 >= m_ptOldCursorPos.y - windowY)
		{
			gameButton = 2;
			exit = true;
		}
		else if (499 <= m_ptOldCursorPos.x - windowX && 617 >= m_ptOldCursorPos.x - windowX
			&& 440 <= m_ptOldCursorPos.y - windowY && 474 >= m_ptOldCursorPos.y - windowY)
			gameButton = 3;
	}
	else
	{
		if (518 <= m_ptOldCursorPos.x - windowX && 580 >= m_ptOldCursorPos.x - windowX
			&& 265 <= m_ptOldCursorPos.y - windowY && 304 >= m_ptOldCursorPos.y - windowY)
			gameButton = 1;
		else if (521 <= m_ptOldCursorPos.x - windowX && 577 >= m_ptOldCursorPos.x - windowX
			&& 337 <= m_ptOldCursorPos.y - windowY && 374 >= m_ptOldCursorPos.y - windowY)
		{
			gameButton = 2;
			exit = true;
			ChangeSwapChainState();
		}
		else if (493 <= m_ptOldCursorPos.x - windowX && 611 >= m_ptOldCursorPos.x - windowX
			&& 409 <= m_ptOldCursorPos.y - windowY && 445 >= m_ptOldCursorPos.y - windowY)
			gameButton = 3;
	}

	switch (gameButton)
	{
	case 1://play
		sound[1].Stop();//오프닝
		sound[0].Play();//인게임

		ShowCursor(false);//마우스

		//// 스프라이트 배치를 생성합니다.
		//std::unique_ptr<DirectX::SpriteBatch> spriteBatch(new DirectX::SpriteBatch(m_d3dDevice.Get(), m_commandList.Get()));

		//// 스프라이트 텍스쳐를 로드합니다.
		//Microsoft::WRL::ComPtr<ID3D12Resource> texture;
		//Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;
		//CreateDDSTextureFromFile(m_d3dDevice.Get(), L"ui_sprite.dds", texture.GetAddressOf(), textureUploadHeap.GetAddressOf());

		//// 스프라이트 배치를 시작합니다.
		//spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_states->NonPremultiplied());

		//// UI 스프라이트를 그립니다.
		//const DirectX::XMFLOAT2 position = DirectX::XMFLOAT2(0.0f, m_outputSize.Height - m_spriteSize.y); // 좌측 하단 위치
		//spriteBatch->Draw(texture.Get(), m_spriteSize, position);

		//// 스프라이트 배치를 종료합니다.
		//spriteBatch->End();

		//// 스프라이트를 생성합니다.
		//auto device = m_d3dDevice.Get();
		//auto context = m_d3dContext.Get();
		//auto spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

		//// 스프라이트를 렌더링합니다.
		//spriteBatch->Begin();
		//spriteBatch->Draw(m_texture.Get(), XMFLOAT2(0, 0), nullptr, Colors::White);
		//spriteBatch->End();


		m_pStage->CheckCameraCollisions(fTimeElapsed, m_pPlayer, m_pCamera);

		break;
	case 2://exit
		PostQuitMessage(0);
		break;
	case 3://settings
		break;
	}



	for (auto& monster : Monsters) {
		monster->Update(fTimeElapsed);
	}
	AnimateObjects(fTimeElapsed);
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	m_pStage->OnPrepareRender(m_pd3dCommandList);
	m_pStage->OnPreRender(m_pd3dCommandList, m_pLights, m_pStage->m_pd3dCbvSrvDescriptorHeap, Monsters, Players, firstFloor);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * ::gnRtvDescriptorIncrementSize);

	float pfClearColor[4] = { 0.31f, 0.74f, 0.88f, 1.0f };// 하늘 색깔
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	if (true == wakeUp)
		m_pStage->bLightwakeUp = false;
	else
		m_pStage->bLightwakeUp = true;


	if (4 == m_pPlayer->m_pSkinnedAnimationController->Cur_Animation_Track)//게임오버
	{
		if (m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_fPosition ==
			m_pPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_nAnimationSet]->m_fLength)
		{
			temp->m_ppMaterials[0] = m_pStage->m_ppShaders[0]->gameMat[2];
			temp->SetPosition(880, -65, 1000);
			m_pCamera->SetPosition(XMFLOAT3(800, -150, 900));

			monsterSound.Stop();//몬스터
			monsterSound.Terminate();
			sound[0].Stop();//인게임
			sound[2].Play();//클로징
		}
	}

	if (1 == gameButton && true == m_pPlayer->alive)
	{

		if (-64 > m_pPlayer->GetPosition().y)
		{
			m_pStage->hpUi[0]->SetPosition(50, -225, 178);
			m_pStage->hpUi[1]->SetPosition(50, -225, 346);
		}
		else
		{
			m_pStage->hpUi[0]->SetPosition(50, -55, 178);
			m_pStage->hpUi[1]->SetPosition(50, -55, 346);
			//m_pStage->hpUi[1]->SetPosition(50, -55, 320);
		}


		m_pStage->hpUi[0]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		m_pStage->hpUi[1]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
	}

	if (-1 == gameButton)
	{
		if ('v' == input || true == userName[0])
		{
			m_pStage->userId[0]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
			userName[0] = true;
		}
		if ('d' == input || true == userName[1])
		{
			m_pStage->userId[1]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
			userName[1] = true;
		}
		if ('o' == input || true == userName[2])
		{
			m_pStage->userId[2]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
			userName[2] = true;
		}
		if ('l' == input || true == userName[3])
		{
			m_pStage->userId[3]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
			userName[3] = true;
		}
	}


	if (m_pStage)
		m_pStage->Render(m_pd3dCommandList, m_pCamera);

	if (-70 > m_pPlayer->GetPosition().y)
		firstFloor = true;
	else
		firstFloor = false;

	if (m_pStage->m_pShadowShader)
	{
		m_pStage->m_pShadowShader->Render(m_pd3dCommandList, m_pCamera, Monsters, Players, m_pLights, firstFloor);
	}


	temp->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
	//m_Test->Render()




	if (m_pStage->m_pShadowMapToViewport && 1 == gameButton && true == m_pPlayer->alive)
	{
		if (-1 == m_pStage->m_pShadowMapToViewport->curPl)
		{
			m_pStage->m_pShadowMapToViewport->maxHp = 55500;
			m_pStage->m_pShadowMapToViewport->curPl = 0;
		}

		if (false == m_pStage->m_pShadowMapToViewport->init)
		{
			if (1 != m_pStage->m_pShadowMapToViewport->curPl)
			{
				m_pStage->m_pShadowMapToViewport->maxHp = 55500;

				if (2 != m_pStage->m_pShadowMapToViewport->curPl)
					++m_pStage->m_pShadowMapToViewport->curPl;
				else
					m_pStage->m_pShadowMapToViewport->curPl = 0;
			}
			else
			{
				m_pStage->m_pShadowMapToViewport->maxHp = 55500;
				//m_pPlayer->HP = 55500;
				m_pStage->m_pShadowMapToViewport->curPl = 2;
			}
			m_pStage->m_pShadowMapToViewport->init = true;

		}

		m_pStage->m_pShadowMapToViewport->Render(m_pd3dCommandList, m_pCamera, m_pPlayer->HP);
	};


	/*for (auto& player : Players) {
		if (player->c_id > -1) {
			player->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
			player->m_ppBullet->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		}
	}
	for (const auto& monster : Monsters) {
		if (monster->c_id > -1) {
			monster->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
			monster->m_ppHat->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		}
	}*/

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif


	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	size_t nLength = _tcslen(m_pszFrameRate);
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	_stprintf_s(m_pszFrameRate + nLength, 70 - nLength, _T("(%4f, %4f, %4f)"), xmf3Position.x, xmf3Position.y, xmf3Position.z);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}



//// 텍스트 출력 함수
//void RenderText()
//{
//	// 텍스트를 그리기 위한 정점 데이터
//	struct Vertex
//	{
//		XMFLOAT3 position;
//		XMFLOAT2 texCoord;
//	};
//
//	// 텍스트의 정점 데이터
//	Vertex vertices[] =
//	{
//		// 정점 위치         텍스처 좌표
//		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
//		{ XMFLOAT3(100.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
//		{ XMFLOAT3(0.0f, 100.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
//		{ XMFLOAT3(100.0f, 100.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) }
//	};
//
//	// 정점 버퍼 생성
//	D3D12_HEAP_PROPERTIES vbHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	D3D12_RESOURCE_DESC vbResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
//	ID3D12Resource* vertexBuffer;
//	m_pd3dDevice->CreateCommittedResource(
//		&vbHeapProperties,
//		D3D12_HEAP_FLAG_NONE,
//		&vbResourceDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&vertexBuffer)
//	);
//
//	// 정점 버퍼에 데이터 복사
//	UINT8* pVertexDataBegin;
//	CD3DX12_RANGE readRange(0, 0);
//	vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
//	memcpy(pVertexDataBegin, vertices, sizeof(vertices));
//	vertexBuffer->Unmap(0, nullptr);
//
//	// 텍스트를 그리기 위한 인덱스 데이터
//	WORD indices[] =
//	{
//		0, 1, 2, // 삼각형 1
//		2, 1, 3  // 삼각형 2
//	};
//
//	// 인덱스 버퍼 생성
//	D3D12_HEAP_PROPERTIES ibHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	D3D12_RESOURCE_DESC ibResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
//	ID3D12Resource* indexBuffer;
//	m_pd3dDevice->CreateCommittedResource(
//		&ibHeapProperties,
//		D3D12_HEAP_FLAG_NONE,
//		&ibResourceDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&indexBuffer)
//	);
//
//	// 인덱스 버퍼에 데이터 복사
//	UINT8* pIndexDataBegin;
//	indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin));
//	memcpy(pIndexDataBegin, indices, sizeof(indices));
//	indexBuffer->Unmap(0, nullptr);
//
//	// 셰이더 컴파일 및 로드, 파이프라인 설정 등의 작업 생략
//
//	// 그래픽 명령 리스트 작성 시작
//	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, nullptr);
//
//	// 렌더 타겟 설정 등의 작업 생략
//
//	// 렌더 타겟과 셰이더 리소스 바인딩
//	m_pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature);
//	m_pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
//	m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dDescriptorHeap);
//	m_pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// 정점 버퍼 설정
//	D3D12_VERTEX_BUFFER_VIEW vbView = {};
//	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
//	vbView.StrideInBytes = sizeof(Vertex);
//	vbView.SizeInBytes = sizeof(vertices);
//	m_pd3dCommandList->IASetVertexBuffers(0, 1, &vbView);
//
//	// 인덱스 버퍼 설정
//	D3D12_INDEX_BUFFER_VIEW ibView = {};
//	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
//	ibView.Format = DXGI_FORMAT_R16_UINT;
//	ibView.SizeInBytes = sizeof(indices);
//	m_pd3dCommandList->IASetIndexBuffer(&ibView);
//
//	// 그리기 호출
//	m_pd3dCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
//
//	// 그래픽 명령 리스트 실행 등의 작업 생략
//
//	// 텍스트 출력 함수 호출 시 화면에 텍스트가 그려짐
//
//	// ...
//
//	// 텍스트 출력 함수 호출 후 화면에 그려진 텍스트를 스왑체인에 표시하는 등의 작업 생략
//}

void CGameFramework::RenderText()
{
	// 텍스트를 그리기 위한 정점 데이터
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 texCoord;
	};

	// 텍스트의 정점 데이터
	Vertex vertices[] =
	{
		// 정점 위치         텍스처 좌표
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(100.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.0f, 100.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(100.0f, 100.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) }
	};

	// ...

	// 텍스트 출력에 사용할 문자열
	const wchar_t* text = L"VooDooDoll";

	// 문자열의 길이
	int textLength = wcslen(text);

	// 텍스처 좌표를 계산하여 정점 데이터에 적용
	for (int i = 0; i < textLength; ++i)
	{
		vertices[i].texCoord.x = static_cast<float>(i) / static_cast<float>(textLength - 1);
		vertices[i + 1].texCoord.x = static_cast<float>(i + 1) / static_cast<float>(textLength - 1);
	}

	// ...

	// 그래픽 명령 리스트 작성 시작
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, nullptr);

	// ...


	// 정점 버퍼 생성
	//D3D12_HEAP_PROPERTIES vbHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_HEAP_PROPERTIES vbHeapProperties = {};
	vbHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	vbHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	vbHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	vbHeapProperties.CreationNodeMask = 1;
	vbHeapProperties.VisibleNodeMask = 1;
	//D3D12_RESOURCE_DESC vbResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
	D3D12_RESOURCE_DESC vbResourceDesc = {};
	vbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vbResourceDesc.Alignment = 0;
	vbResourceDesc.Width = sizeof(vertices);
	vbResourceDesc.Height = 1;
	vbResourceDesc.DepthOrArraySize = 1;
	vbResourceDesc.MipLevels = 1;
	vbResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	vbResourceDesc.SampleDesc.Count = 1;
	vbResourceDesc.SampleDesc.Quality = 0;
	vbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vbResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


	ID3D12Resource* vertexBuffer;
	m_pd3dDevice->CreateCommittedResource(
		&vbHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vbResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	);

	//// 정점 버퍼에 데이터 복사
	//D3D12_RANGE readRange = {}; // 매핑 범위를 나타내는 D3D12_RANGE 구조체 초기화
	//UINT8* pVertexDataBegin;
	//vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	//memcpy(pVertexDataBegin, vertices, sizeof(vertices));
	//vertexBuffer->Unmap(0, nullptr);


	// 텍스트 출력에 사용할 정점 버퍼 설정
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.StrideInBytes = sizeof(Vertex);
	vbView.SizeInBytes = sizeof(vertices) - sizeof(Vertex) + sizeof(Vertex) * textLength;
	m_pd3dCommandList->IASetVertexBuffers(0, 1, &vbView);

	// ...

	// 그리기 호출
	m_pd3dCommandList->DrawIndexedInstanced(6, textLength - 1, 0, 0, 0);

	// ...

	// 텍스트 출력 함수 호출 시 화면에 "VooDooDoll"이 그려짐

	// ...
}