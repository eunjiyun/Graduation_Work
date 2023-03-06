//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"

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
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_CONTROL:
			ChangeSwapChainState();
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
	if (m_pLogin) m_pStage->BuildObjects(m_pd3dDevice, m_pd3dCommandList);
	if (m_pStage)
	{


	}m_pStage->BuildObjects(m_pd3dDevice, m_pd3dCommandList);


#ifdef _WITH_TERRAIN_PLAYER
	pPlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 1);
	//pPlayer->otherPlayerUpdate(m_GameTimer.GetTimeElapsed());
	pPlayer2 = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 2);
	//pPlayer2->otherPlayerUpdate(m_GameTimer.GetTimeElapsed());
	pPlayer3 = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 3);
	//pPlayer3->otherPlayerUpdate(m_GameTimer.GetTimeElapsed());
#else
	CAirplanePlayer* pPlayer = new CAirplanePlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL);
	pPlayer->SetPosition(XMFLOAT3(425.0f, 240.0f, 640.0f));
#endif


	if (pPlayer)
	{
		m_pStage->m_pPlayer = m_pPlayer = pPlayer;
		m_pCamera = m_pPlayer->GetCamera();

		for (int i = 0; i < 3; i++) {
			CTerrainPlayer* pAirplanePlayer = new CTerrainPlayer(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), 1);
			pAirplanePlayer->otherPlayerUpdate();
			Players.push_back(pAirplanePlayer);
		}

		pMonsterModel = new CLoadedModelInfo * [6];

		pMonsterModel[0] = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo19.bin", NULL, 1);//자폭하게
		pMonsterModel[1] = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo23.bin", NULL, 2);//자폭하게
		pMonsterModel[2] = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo31.bin", NULL, 3);//자폭하게
		pMonsterModel[3] = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo41.bin", NULL, 4);//자폭하게
		pMonsterModel[4] = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo52.bin", NULL, 5);//자폭하게
		pMonsterModel[5] = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo62.bin", NULL, 6);//자폭하게

		m_nHierarchicalGameObjects = 1;
		m_ppHierarchicalGameObjects = new CMonster * [m_nHierarchicalGameObjects];

		//for (int i = 0; i < 13; i++) {

		//	if (3 > i)
		//	{
		//		CLoadedModelInfo* pMonsterModel3 = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo31.bin", NULL, 3);//자폭하게
		//		m_ppHierarchicalGameObjects[0] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel3, 2, 3);//귀신
		//		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		//		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		//		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		//		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		//		m_ppHierarchicalGameObjects[0]->SetScale(1.0f, 1.0f, 1.0f);
		//		if (pMonsterModel3) delete pMonsterModel3;

		//		Monsters.push_back(m_ppHierarchicalGameObjects[0]);
		//	}
		//	else if (8 < i)
		//	{
		//		CLoadedModelInfo* pMonsterModel2 = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo23.bin", NULL, 2);
		//		m_ppHierarchicalGameObjects[1] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel2, 3, 2);//뼈다귀 다리
		//		m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		//		m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		//		m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		//		m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		//		m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		//		m_ppHierarchicalGameObjects[1]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		//		m_ppHierarchicalGameObjects[1]->SetScale(1.0f, 1.0f, 1.0f);
		//		if (pMonsterModel2) delete pMonsterModel2;

		//		Monsters.push_back(m_ppHierarchicalGameObjects[1]);
		//	}
		//	else if (9< i)
		//	{
		//		CLoadedModelInfo* pMonsterModel5 = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo52.bin", NULL, 5);
		//		m_ppHierarchicalGameObjects[2] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel5, 3, 5);//마법사
		//		m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		//		m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		//		m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		//		m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		//		m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		//		m_ppHierarchicalGameObjects[2]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		//		m_ppHierarchicalGameObjects[2]->SetScale(1.0f, 1.0f, 1.0f);
		//		if (pMonsterModel5) delete pMonsterModel5;

		//		Monsters.push_back(m_ppHierarchicalGameObjects[2]);
		//	}
		//	else if (10 < i)
		//	{
		//		CLoadedModelInfo* pMonsterModel = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo19.bin", NULL, 1);
		//		m_ppHierarchicalGameObjects[3] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel, 3, 1);//손에 칼
		//		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		//		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		//		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		//		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		//		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		//		m_ppHierarchicalGameObjects[3]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		//		m_ppHierarchicalGameObjects[3]->SetScale(1.0f, 1.0f, 1.0f);
		//		if (pMonsterModel) delete pMonsterModel;

		//		Monsters.push_back(m_ppHierarchicalGameObjects[3]);
		//	}
		//	else if (11 < i)
		//	{
		//		CLoadedModelInfo* pMonsterModel6 = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo62.bin", NULL, 6);//
		//		m_ppHierarchicalGameObjects[4] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel6, 3, 6);
		//		m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		//		m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		//		m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		//		m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		//		m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		//		m_ppHierarchicalGameObjects[4]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		//		m_ppHierarchicalGameObjects[4]->SetScale(1.0f, 1.0f, 1.0f);
		//		if (pMonsterModel6) delete pMonsterModel6;

		//		Monsters.push_back(m_ppHierarchicalGameObjects[4]);
		//	}
		//	else if (13 < i)
		//	{
		//		CLoadedModelInfo* pMonsterModel4 = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Voodoo41.bin", NULL, 4);//
		//		m_ppHierarchicalGameObjects[5] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel4, 5, 4);//손에 바늘
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackEnable(2, false);//바늘 휘두르기 딴걸로
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackEnable(3, false);
		//		m_ppHierarchicalGameObjects[5]->m_pSkinnedAnimationController->SetTrackEnable(4, false);//area attack? 화재발생
		//		m_ppHierarchicalGameObjects[5]->SetScale(1.0f, 1.0f, 1.0f);
		//		if (pMonsterModel4) delete pMonsterModel4;

		//		Monsters.push_back(m_ppHierarchicalGameObjects[5]);
		//	}
		//}


		//23.02.22
		m_ppBullets = new CGameObject * [BULLETS];

		CLoadedModelInfo* arrow = CGameObject::LoadGeometryAndAnimationFromFile(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), "Model/Warlock_weapon2.bin", NULL, 7);
		m_ppBullets[0] = new CBulletObject(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), arrow, 1);
		m_ppBullets[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppBullets[0]->SetScale(1.1f, 1.1f, 1.1f);

		//for (int i = 0; i < BULLETS; i++)
		for (int i = 0; i < 1; i++)
		{
			m_ppBullets[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
			m_ppBullets[i]->SetRotationSpeed(360.0f);
			m_ppBullets[i]->SetMovingSpeed(120.0f);
			m_ppBullets[i]->SetActive(false);
		}

		if (arrow) delete arrow;


		m_pStage->BuildDefaultLightsAndMaterials();//인형이 까맣게 출력


		m_pd3dCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
		m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

		WaitForGpuComplete();

		if (m_pStage) m_pStage->ReleaseUploadBuffers();
		if (m_pPlayer) m_pPlayer->ReleaseUploadBuffers();

		/*for (int i = 0; i < m_nHierarchicalGameObjects; i++)
			m_ppHierarchicalGameObjects[i]->ReleaseUploadBuffers();*/
	}



	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer) m_pPlayer->Release();

	for (auto& player : Players)
		delete player;
	for (auto& monster : Monsters)
		delete monster;

	/*if (m_ppHierarchicalGameObjects)
	{
		for (int i = 0; i < m_nHierarchicalGameObjects; i++) if (m_ppHierarchicalGameObjects[i]) m_ppHierarchicalGameObjects[i]->Release();
		delete[] m_ppHierarchicalGameObjects;
	}*/

	if (m_pStage) m_pStage->ReleaseObjects();
	if (m_pStage) delete m_pStage;

	if (m_pLogin) m_pLogin->ReleaseObjects();
	if (m_pLogin) delete m_pLogin;
}

void CGameFramework::CreateOtherPlayer(int p_id, XMFLOAT3 Pos, XMFLOAT3 Look, XMFLOAT3 Up, XMFLOAT3 Right)
{
	for (auto& player : Players)
		if (player->c_id < 0) {
			player->c_id = p_id;
			player->SetPosition(Pos);
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


	switch (type)
	{
	case 1:
		m_ppHierarchicalGameObjects[0] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel[0], 3, 1);//손에 칼
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_ppHierarchicalGameObjects[0]->SetScale(1.0f, 1.0f, 1.0f);
		m_ppHierarchicalGameObjects[0]->c_id = npc_id;
		m_ppHierarchicalGameObjects[0]->SetPosition(Pos);
		break;
	case 2:
		m_ppHierarchicalGameObjects[0] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel[1], 3, 2);//뼈다귀 다리
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_ppHierarchicalGameObjects[0]->SetScale(1.0f, 1.0f, 1.0f);
		m_ppHierarchicalGameObjects[0]->c_id = npc_id;
		m_ppHierarchicalGameObjects[0]->SetPosition(Pos);
		break;
	case 3:
		m_ppHierarchicalGameObjects[0] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel[2], 2, 3);//귀신
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[0]->SetScale(1.0f, 1.0f, 1.0f);
		m_ppHierarchicalGameObjects[0]->c_id = npc_id;
		m_ppHierarchicalGameObjects[0]->SetPosition(Pos);
		break;
	case 4:
		m_ppHierarchicalGameObjects[0] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel[3], 5, 4);//손에 바늘
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(2, false);//바늘 휘두르기 딴걸로
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(3, false);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(4, false);//area attack? 화재발생
		m_ppHierarchicalGameObjects[0]->SetScale(1.0f, 1.0f, 1.0f);
		m_ppHierarchicalGameObjects[0]->c_id = npc_id;
		m_ppHierarchicalGameObjects[0]->SetPosition(Pos);
		break;
	case 5:
		m_ppHierarchicalGameObjects[0] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel[4], 3, 5);//마법사
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_ppHierarchicalGameObjects[0]->SetScale(1.0f, 1.0f, 1.0f);
		m_ppHierarchicalGameObjects[0]->c_id = npc_id;
		m_ppHierarchicalGameObjects[0]->SetPosition(Pos);
		break;
	case 6:
		m_ppHierarchicalGameObjects[0] = new CMonster(m_pd3dDevice, m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), pMonsterModel[5], 3, 6);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(0, false);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		m_ppHierarchicalGameObjects[0]->m_pSkinnedAnimationController->SetTrackEnable(2, false);
		m_ppHierarchicalGameObjects[0]->SetScale(1.0f, 1.0f, 1.0f);
		m_ppHierarchicalGameObjects[0]->c_id = npc_id;
		m_ppHierarchicalGameObjects[0]->SetPosition(Pos);
		break;
	default:
		break;
	}
	Monsters.push_back(m_ppHierarchicalGameObjects[0]);


	for (auto& monster : Monsters)
	{
		if (monster->c_id < 0) {
			monster->c_id = npc_id;
			monster->npc_type = type;
			monster->SetPosition(Pos);
			//monster->SetLookVector(Look);
			//monster->SetUpVector(Up);
			//monster->SetRightVector(Right);
			//cout << player->c_id << endl;
			//break;
		}
	}
}


void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	bool bProcessedByStage = false;
	if (GetKeyboardState(pKeysBuffer) && m_pStage) bProcessedByStage = m_pStage->ProcessInput(pKeysBuffer);
	if (!bProcessedByStage)
	{
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		DWORD dwDirection = 0;


		if (pKeysBuffer[0x57] & 0xF0) dwDirection |= DIR_FORWARD;//w
		if (pKeysBuffer[0x53] & 0xF0) dwDirection |= DIR_BACKWARD;//s
		if (pKeysBuffer[0x41] & 0xF0) dwDirection |= DIR_LEFT;//a
		if (pKeysBuffer[0x44] & 0xF0) dwDirection |= DIR_RIGHT;//d

		//23.02.20
		if (pKeysBuffer[0x5A] & 0xF0) dwDirection |= DIR_ATTACK;//z Attack
		if (pKeysBuffer[0x58] & 0xF0) dwDirection |= DIR_RUN;//x run
		if (pKeysBuffer[0x4B] & 0xF0) dwDirection |= DIR_DIE;//k die
		if (pKeysBuffer[0x43] & 0xF0) dwDirection |= DIR_COLLECT;//c collect
		//

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (pKeysBuffer[VK_RBUTTON] & 0xF0)
				{
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);

					//if (true == onRotate)
					//{
						//m_ppBullets[0]->Rotate(cyDelta , 0.0f, -cxDelta );
						//onRotate = false;
					//}
				}
				else
				{
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);

					//if(true)
					//m_ppBullets[0]->Rotate(cyDelta*3, cxDelta*3, 0.0f);
				}
			}
			if (dwDirection)
			{
				m_pPlayer->Move(dwDirection, 7.0f, true);

				m_pPlayer->playerAttack(whatPlayer, m_pLockedObject, &m_ppBullets);
				m_pLockedObject = NULL;

				m_pPlayer->playerRun();
				m_pPlayer->playerDie();
				m_pPlayer->playerCollect();
			}
		}
	}

	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
	//m_pPlayer->otherPlayerUpdate(m_GameTimer.GetTimeElapsed());

	for (auto& player : Players) {
		if (player->c_id > -1)
		{
			//player->Update(m_GameTimer.GetTimeElapsed());
			player->otherPlayerUpdate();
		}

	}
	/*for (auto& monster : Monsters) {
		if (monster->c_id > -1)
		{
			monster->otherPlayerUpdate();
		}

	}*/
}

void CGameFramework::AnimateObjects(float fTimeElapsed)
{
	if (m_pStage) m_pStage->AnimateObjects(fTimeElapsed);

	m_pPlayer->Animate(fTimeElapsed);

	for (auto& player : Players)
		if (player->c_id > -1)
			player->Animate(fTimeElapsed);
	for (auto& monster : Monsters)
		if (monster->c_id > -1)
			monster->Animate(fTimeElapsed);
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
	m_GameTimer.Tick(30.0f);

	//ProcessInput();
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();


	m_pPlayer->Update(fTimeElapsed);

	m_pStage->CheckObjectByObjectCollisions(fTimeElapsed);

	m_pPlayer->Deceleration(fTimeElapsed);

	//if(2==whatPlayer&&true== m_pPlayer->onAttack)
	m_ppBullets[0]->Animate(fTimeElapsed);//총알 업뎃

	AnimateObjects(fTimeElapsed);

	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

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
		m_pStage->wakeUp = false;
	else
		m_pStage->wakeUp = true;

	if (m_pStage) m_pStage->Render(m_pd3dCommandList, m_pCamera);

	/*for (int i = 0; i < m_nHierarchicalGameObjects; i++)
	{
		if (m_ppHierarchicalGameObjects[i])
		{
			m_ppHierarchicalGameObjects[i]->Animate(fTimeElapsed);
			if (!m_ppHierarchicalGameObjects[i]->m_pSkinnedAnimationController) m_ppHierarchicalGameObjects[i]->UpdateTransform(NULL);
			m_ppHierarchicalGameObjects[i]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		}
	}*/

	m_pStage->whatPlayer = whatPlayer;

	changePlayerForm(&m_pStage->m_pPlayer, &m_pPlayer, pPlayer, pPlayer2);//플레이어 모드 변경

	for (const auto& player : Players) {
		if (player->c_id > -1) {

			//changePlayerForm(m_pStage->m_pPlayer, player, pPlayer, pPlayer2);
			player->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		}
	}
	for (const auto& monster : Monsters) {
		if (monster->c_id > -1) {

			//changePlayerForm(m_pStage->m_pPlayer, player, pPlayer, pPlayer2);
			monster->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
		}
	}

#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif

	//m_pPlayer->SetPosition(XMFLOAT3(300, 0, 0));
	if (m_pPlayer)
	{
		m_pPlayer->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);

		if (2 == whatPlayer)//궁수 모드일 때 총알 렌더
		{
			//for (int i = 0; i < BULLETS; ++i)
			for (int i = 0; i < 1; ++i)
			{
				if (m_ppBullets[i]->m_bActive)
				{
					m_ppBullets[i]->Render(m_pd3dCommandList, m_pStage->GetGraphicsRootSignature(), NULL, m_pCamera);
				}
			}
		}
	}

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

void CGameFramework::changePlayerForm(CPlayer** sceneOldPlayer, CPlayer** oldPlayer, CTerrainPlayer* newPlayer, CTerrainPlayer* newPlayer2)//받을 플레이어, 할당할 플레이어
{
	if (1 == whatPlayer)
	{
		m_pStage->m_pPlayer = m_pPlayer = pPlayer;

		if (true == changePlayerMode)
		{
			m_pPlayer->SetPosition(pPlayer3->GetPosition());

			m_pPlayer->SetLookVector(pPlayer3->GetLookVector());
			m_pPlayer->SetUpVector(pPlayer3->GetUpVector());
			m_pPlayer->SetRightVector(pPlayer3->GetRightVector());

			changePlayerMode = false;
		}
	}
	else if (2 == whatPlayer)
	{
		m_pStage->m_pPlayer = m_pPlayer = pPlayer2;

		if (true == changePlayerMode)
		{
			m_pPlayer->SetPosition(pPlayer->GetPosition());

			m_pPlayer->SetLookVector(pPlayer->GetLookVector());
			m_pPlayer->SetUpVector(pPlayer->GetUpVector());
			m_pPlayer->SetRightVector(pPlayer->GetRightVector());

			changePlayerMode = false;
		}
	}
	else if (3 == whatPlayer)
	{
		m_pStage->m_pPlayer = m_pPlayer = pPlayer3;

		if (true == changePlayerMode)
		{
			m_pPlayer->SetPosition(pPlayer2->GetPosition());

			m_pPlayer->SetLookVector(pPlayer2->GetLookVector());
			m_pPlayer->SetUpVector(pPlayer2->GetUpVector());
			m_pPlayer->SetRightVector(pPlayer2->GetRightVector());

			changePlayerMode = false;
		}
	}

	m_pCamera = m_pPlayer->GetCamera();
}

