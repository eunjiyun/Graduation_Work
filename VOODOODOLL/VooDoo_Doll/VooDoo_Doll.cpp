// VooDoo_Doll.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "VooDoo_Doll.h"
#include "GameFramework.h"
#include "SERVER.h"
#include <chrono>

using namespace chrono;

#define MAX_LOADSTRING 100

#pragma region S_variables
// 서버 통신에 사용되는 변수들의 집합
constexpr short SERVER_PORT = 3500;
SOCKET s_socket;
OVER_EXP recv_over;
short _prev_remain = 0;
auto elapsedTime = high_resolution_clock::now();
#pragma endregion

HINSTANCE                  ghAppInstance;
TCHAR                     szTitle[MAX_LOADSTRING];
TCHAR                     szWindowClass[MAX_LOADSTRING];

CGameFramework               gGameFramework;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

void do_recv();

void ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	DWORD dwDirection = 0;
	if (::GetKeyboardState(pKeysBuffer))
	{
		if (!gGameFramework.m_pPlayer->onAct && 1 == gGameFramework.gameButton) {
			if (pKeysBuffer[0x57] & 0xF0) dwDirection |= DIR_FORWARD;//w
			if (pKeysBuffer[0x53] & 0xF0) dwDirection |= DIR_BACKWARD;//s
			if (pKeysBuffer[0x41] & 0xF0) dwDirection |= DIR_LEFT;//a
			if (pKeysBuffer[0x44] & 0xF0) dwDirection |= DIR_RIGHT;//d
			if (pKeysBuffer[0x10] & 0xF0 && dwDirection) dwDirection |= DIR_RUN;// Shift run
			if (pKeysBuffer[0x20] & 0xF0) dwDirection |= DIR_JUMP; //space jump
		}
	}
	float cxDelta = 0.0f, cyDelta = 0.0f;

	gGameFramework.m_pPlayer->cxDelta = gGameFramework.m_pPlayer->cyDelta = gGameFramework.m_pPlayer->czDelta = 0.0f;
	//if (1 == gGameFramework.gameButton && ::GetForegroundWindow() == gGameFramework.Get_HWND())
	//{
	//    POINT ptCursorPos;
	//    ::GetCursorPos(&ptCursorPos);

	//    RECT rcClient;
	//    ::GetClientRect(gGameFramework.Get_HWND(), &rcClient);

	//    //if (::PtInRect(&rcClient, ptCursorPos))
	//    //{
	//        if (ptCursorPos.x - gGameFramework.Get_OldCursorPointX() > 0)
	//            cxDelta = min((float)(ptCursorPos.x - gGameFramework.Get_OldCursorPointX()) / 3.0f, 10.f);
	//        else
	//            cxDelta = max((float)(ptCursorPos.x - gGameFramework.Get_OldCursorPointX()) / 3.0f, -10.f);

	//        if (ptCursorPos.y - gGameFramework.Get_OldCursorPointY() > 0)
	//            cyDelta = min((float)(ptCursorPos.y - gGameFramework.Get_OldCursorPointY()) / 3.0f, 10.f);
	//        else
	//            cyDelta = max((float)(ptCursorPos.y - gGameFramework.Get_OldCursorPointY()) / 3.0f, -10.f);

	//        ::SetCursorPos(gGameFramework.Get_OldCursorPointX(), gGameFramework.Get_OldCursorPointY());
	//    //}
	//}

	if (GetCapture() == gGameFramework.Get_HWND())
	{
		if (1 == gGameFramework.gameButton)
		{
			::SetCursor(NULL);
			POINT ptCursorPos;
			::GetCursorPos(&ptCursorPos);
			if (ptCursorPos.x - gGameFramework.Get_OldCursorPointX() > 0)
				cxDelta = min((float)(ptCursorPos.x - gGameFramework.Get_OldCursorPointX()) / 3.0f, 10.f);
			else
				cxDelta = max((float)(ptCursorPos.x - gGameFramework.Get_OldCursorPointX()) / 3.0f, -10.f);

			if (ptCursorPos.y - gGameFramework.Get_OldCursorPointY() > 0)
				cyDelta = min((float)(ptCursorPos.y - gGameFramework.Get_OldCursorPointY()) / 3.0f, 10.f);
			else
				cyDelta = max((float)(ptCursorPos.y - gGameFramework.Get_OldCursorPointY()) / 3.0f, -10.f);
			::SetCursorPos(gGameFramework.Get_OldCursorPointX(), gGameFramework.Get_OldCursorPointY());
		}
	}

	if (dwDirection) gGameFramework.m_pPlayer->Move(dwDirection, 700, true);

	if (cxDelta != 0.0f || cyDelta != 0.0f)
	{
		CS_ROTATE_PACKET p;
		p.size = sizeof(CS_ROTATE_PACKET);
		p.type = CS_ROTATE;

		gGameFramework.m_pPlayer->cxDelta = p.cxDelta = cyDelta;
		gGameFramework.m_pPlayer->cyDelta = p.cyDelta = cxDelta;
		gGameFramework.m_pPlayer->czDelta = p.czDelta = 0.f;
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(&p) };
		int ErrorStatus = WSASend(s_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, &send_callback);
		if (ErrorStatus == SOCKET_ERROR) err_quit("send()");
	}

	if (duration_cast<milliseconds>(high_resolution_clock::now() - elapsedTime).count() > 100) {
		CS_HEARTBEAT_PACKET p;
		p.direction = dwDirection;
		p.size = sizeof(CS_HEARTBEAT_PACKET);
		p.type = CS_HEARTBEAT;
		p.pos = gGameFramework.m_pPlayer->GetPosition();
		p.vel = gGameFramework.m_pPlayer->GetVelocity();

		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(&p) };
		int ErrorStatus = WSASend(s_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, &send_callback);
		if (ErrorStatus == SOCKET_ERROR) err_quit("send()");
		elapsedTime = high_resolution_clock::now();
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	wcout.imbue(locale("korean"));

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_VOODOODOLL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VOODOODOLL));


#pragma region SERVER ACCESS

	WSADATA WSAData;
	int ErrorStatus = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ErrorStatus != 0)
	{
		cout << "WSAStartup 실패\n";
	}
	s_socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (s_socket == INVALID_SOCKET)
	{
		cout << "소켓 생성 실패\n";
	}

	string IP;
	cout << "서버 IP 입력 : ";
	cin >> IP;
	// 서버와 연결
	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(SERVER_PORT);

	inet_pton(AF_INET, IP.c_str(), &svr_addr.sin_addr);
	ErrorStatus = WSAConnect(s_socket, reinterpret_cast<sockaddr*>(&svr_addr), sizeof(svr_addr), 0, 0, 0, 0);
	if (ErrorStatus == SOCKET_ERROR) err_quit("WSAConnect()");


	do_recv();
#pragma endregion 


	while (1)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			SleepEx(0, true);
			gGameFramework.FrameAdvance();
			if (gGameFramework.m_pPlayer->alive)
				ProcessInput();
		}
	}

	gGameFramework.OnDestroy();

	closesocket(s_socket);
	WSACleanup();

	return((int)msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VOODOODOLL));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_LABPROJECT0797ANIMATION);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghAppInstance = hInstance;

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);
	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hMainWnd) return(FALSE);

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::ShowWindow(hMainWnd, nCmdShow);
	::UpdateWindow(hMainWnd);



	return(TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	// hWnd는 게임 창의 윈도우 핸들입니다.
	RECT rcWindow;
	GetWindowRect(gGameFramework.Get_HWND(), &rcWindow);

	// rcWindow 변수에는 윈도우 창의 위치와 크기가 저장됩니다.
	int windowX = rcWindow.left;
	int windowY = rcWindow.top;

	switch (message)
	{
	case WM_CHAR:
		if (!gGameFramework.lobby[0] && !gGameFramework.idSet && 10 > gGameFramework.userId.size())
		{
			// 사용할 수 있는 키 입력 범위를 지정합니다.
			if (('A' <= wParam && wParam <= 'Z') || ('a' <= wParam && wParam <= 'z') ||
				('0' <= wParam && wParam <= '9') || wParam == '_' || wParam == '*')
			{
				gGameFramework.userId.push_back(wParam);
			}
		}
		else if (!gGameFramework.lobby[0] && gGameFramework.idSet && 10 > gGameFramework.userPw.size())
		{
			// 사용할 수 있는 키 입력 범위를 지정합니다.
			if (('A' <= wParam && wParam <= 'Z') || ('a' <= wParam && wParam <= 'z') ||
				('0' <= wParam && wParam <= '9') || wParam == '_' || wParam == '*')
			{
				gGameFramework.userPw.push_back(wParam);
			}
		}
		break;
	case WM_RBUTTONDOWN:
		if (gGameFramework.m_pPlayer->alive && gGameFramework.m_pPlayer->onAct == false && gGameFramework.m_pPlayer->onFloor == true)
		{
			gGameFramework.m_pPlayer->SetVelocity(XMFLOAT3(0, 0, 0));
			CS_ATTACK_PACKET p;
			p.size = sizeof(CS_ATTACK_PACKET);
			p.type = CS_ATTACK;
			OVER_EXP* attack_data = new OVER_EXP{ reinterpret_cast<char*>(&p) };
			int ErrorStatus = WSASend(s_socket, &attack_data->_wsabuf, 1, 0, 0, &attack_data->_over, &send_callback);
			if (ErrorStatus == SOCKET_ERROR) err_display("WSASend()");
			gGameFramework.m_pPlayer->onAct = true;
		}
		break;
	case WM_KEYDOWN:
		if (VK_CONTROL == wParam)//full screen
		{
			if (gGameFramework.onFullScreen)
				gGameFramework.onFullScreen = false;
			else
				gGameFramework.onFullScreen = true;

			gGameFramework.ChangeSwapChainState();
		}
		else if (!gGameFramework.lobby[0] && VK_BACK == wParam)
			gGameFramework.delUser = true;



		if (gGameFramework.m_pPlayer->alive && gGameFramework.m_pPlayer->onAct == false && gGameFramework.m_pPlayer->onFloor == true)
		{
			if (wParam == 'C' || wParam == 'c')
			{
				CS_INTERACTION_PACKET p;
				p.size = sizeof(CS_INTERACTION_PACKET);
				p.type = CS_INTERACTION;
				OVER_EXP* collect_data = new OVER_EXP{ reinterpret_cast<char*>(&p) };
				int ErrorStatus = WSASend(s_socket, &collect_data->_wsabuf, 1, 0, 0, &collect_data->_over, &send_callback);
				if (ErrorStatus == SOCKET_ERROR) err_display("WSASend()");
			}
			else if (wParam == 'Q' || wParam == 'q')
			{
				CS_CHANGEWEAPON_PACKET p;
				p.size = sizeof(CS_CHANGEWEAPON_PACKET);
				p.type = CS_CHANGEWEAPON;
				OVER_EXP* weapon_data = new OVER_EXP{ reinterpret_cast<char*>(&p) };
				int ErrorStatus = WSASend(s_socket, &weapon_data->_wsabuf, 1, 0, 0, &weapon_data->_over, &send_callback);
				if (ErrorStatus == SOCKET_ERROR) err_display("WSASend()");


                gGameFramework.m_pStage->m_pShadowMapToViewport->init = false;
            }
            else if ('b' == wParam|| 'B' == wParam)
            {
                if (gGameFramework.m_pStage->pComputeShader->blur)
                    gGameFramework.m_pStage->pComputeShader->blur = false;
                else
                    gGameFramework.m_pStage->pComputeShader->blur = true;
            }
        }
        break;
    case WM_KEYUP:
        if (VK_BACK == wParam)
        {
            gGameFramework.delUser = false;
            break;
        }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_SIZE:
        if (gGameFramework.lobby[0])
        {
            if (false == gGameFramework.onFullScreen)
            {
                if (31 <= gGameFramework.m_ptOldCursorPos.x - windowX && 146 >= gGameFramework.m_ptOldCursorPos.x - windowX
                    && 103 <= gGameFramework.m_ptOldCursorPos.y - windowY && 162 >= gGameFramework.m_ptOldCursorPos.y - windowY)//match
                {
                    if (!gGameFramework.lobby[1])
                    {
                        gGameFramework.lobby[1] = true;


						gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = gGameFramework.m_pStage->m_ppShaders[0]->gameMat[5];
						gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
						gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[1]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;


                        // 서버에게 자신의 정보를 패킷으로 전달
                        CS_LOGIN_PACKET p;
                        p.size = sizeof(CS_LOGIN_PACKET);
                        p.type = CS_LOGIN;
                        OVER_EXP* start_data = new OVER_EXP{ reinterpret_cast<char*>(&p) };
                        int ErrorStatus = WSASend(s_socket, &start_data->_wsabuf, 1, 0, 0, &start_data->_over, &send_callback);
                        if (ErrorStatus == SOCKET_ERROR) err_display("WSASend()");
                    }
                    else
                    {
                        //end search
                    }
                }
                else if (30 <= gGameFramework.m_ptOldCursorPos.x - windowX && 145 >= gGameFramework.m_ptOldCursorPos.x - windowX
                    && 166 <= gGameFramework.m_ptOldCursorPos.y - windowY && 225 >= gGameFramework.m_ptOldCursorPos.y - windowY)//quit
                {
                    gGameFramework.gameButton = 2;
                    gGameFramework.m_pStage->exitGame = true;
                }
            }
            else//full
            {
                if (22 <= gGameFramework.m_ptOldCursorPos.x - windowX && 139 >= gGameFramework.m_ptOldCursorPos.x - windowX
                    && 72 <= gGameFramework.m_ptOldCursorPos.y - windowY && 133 >= gGameFramework.m_ptOldCursorPos.y - windowY)//match
                {
                    if (!gGameFramework.lobby[1])
                    {
                        gGameFramework.lobby[1] = true;


						gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = gGameFramework.m_pStage->m_ppShaders[0]->gameMat[5];
						gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
						gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[1]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;


                        // 서버에게 자신의 정보를 패킷으로 전달
                        CS_LOGIN_PACKET p;
                        p.size = sizeof(CS_LOGIN_PACKET);
                        p.type = CS_LOGIN;
                        OVER_EXP* start_data = new OVER_EXP{ reinterpret_cast<char*>(&p) };
                        int ErrorStatus = WSASend(s_socket, &start_data->_wsabuf, 1, 0, 0, &start_data->_over, &send_callback);
                        if (ErrorStatus == SOCKET_ERROR) err_display("WSASend()");
                    }
                    else
                    {
                        //end search
                    }
                }
                else if (22 <= gGameFramework.m_ptOldCursorPos.x - windowX && 139 >= gGameFramework.m_ptOldCursorPos.x - windowX
                    && 136 <= gGameFramework.m_ptOldCursorPos.y - windowY && 194 >= gGameFramework.m_ptOldCursorPos.y - windowY)//quit
                {
                    gGameFramework.gameButton = 2;
                    gGameFramework.m_pStage->exitGame = true;
                    gGameFramework.ChangeSwapChainState();
                }
            }
        }
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
        gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);

		if (0 == gGameFramework.signIn)
		{
			//if (!gGameFramework.loginSign[0])
			//{
			CS_SIGN_PACKET p;
			p.size = sizeof(CS_SIGN_PACKET);
			p.type = CS_SIGNUP;

			wchar_t* wcharArray = new wchar_t[IDPW_SIZE] {L""};
			copy(gGameFramework.userId.begin(), gGameFramework.userId.end(), wcharArray);
			wcscpy_s(p.id, sizeof(p.id) / sizeof(p.id[0]), wcharArray);

			memset(wcharArray, 0, IDPW_SIZE * sizeof(wchar_t));
			copy(gGameFramework.userPw.begin(), gGameFramework.userPw.end(), wcharArray);
			wcscpy_s(p.password, sizeof(p.password) / sizeof(p.password[0]), wcharArray);

			wcout << p.id << ", " << p.password << endl;


			cout << "SIGNUP_PACKET SENT\n";
			OVER_EXP* signup_data = new OVER_EXP{ reinterpret_cast<char*>(&p) };
			int ErrorStatus = WSASend(s_socket, &signup_data->_wsabuf, 1, 0, 0, &signup_data->_over, &send_callback);
			if (ErrorStatus == SOCKET_ERROR) err_display("WSASend()");
			gGameFramework.loginSign[0] = true;
			delete[] wcharArray;
			gGameFramework.signIn = -1;
			//}
		}
		else if (1 == gGameFramework.signIn)
		{
			//if (!gGameFramework.loginSign[1])
		   // {
			CS_SIGN_PACKET p;
			p.size = sizeof(CS_SIGN_PACKET);
			p.type = CS_SIGNIN;

			wchar_t* wcharArray = new wchar_t[IDPW_SIZE] {L""};
			copy(gGameFramework.userId.begin(), gGameFramework.userId.end(), wcharArray);
			wcscpy_s(p.id, sizeof(p.id) / sizeof(p.id[0]), wcharArray);

			memset(wcharArray, 0, IDPW_SIZE * sizeof(wchar_t));
			copy(gGameFramework.userPw.begin(), gGameFramework.userPw.end(), wcharArray);
			wcscpy_s(p.password, sizeof(p.password) / sizeof(p.password[0]), wcharArray);

			wcout << p.id << ", " << p.password << endl;


			cout << "SIGNIN_PACKET SENT\n";
			OVER_EXP* signin_data = new OVER_EXP{ reinterpret_cast<char*>(&p) };
			int ErrorStatus = WSASend(s_socket, &signin_data->_wsabuf, 1, 0, 0, &signin_data->_over, &send_callback);

			if (ErrorStatus == SOCKET_ERROR)
			{
				err_display("WSASend()");
			}
			delete[] wcharArray;
			gGameFramework.signIn = -1;
			//}
		}
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return(::DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		gGameFramework.m_pStage->exitGame = true;
		if (gGameFramework.onFullScreen)
			gGameFramework.ChangeSwapChainState();

		::PostQuitMessage(0);
		break;
	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return((INT_PTR)TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);
		}
		break;
	}
	return((INT_PTR)FALSE);
}

//void ProcessAnimation(CPlayer* pl, SC_UPDATE_PLAYER_PACKET* p)
//{
//    pl->m_pSkinnedAnimationController->SetTrackEnable(pl->m_pSkinnedAnimationController->Cur_Animation_Track, false);
//    if (pl->onFloor == false) {
//        pl->m_pSkinnedAnimationController->SetTrackEnable(5, true);
//        return;
//    }
//    else pl->m_pSkinnedAnimationController->SetTrackPosition(5, 1.0f);
//
//    if (Vector3::Length(pl->m_xmf3Velocity) > 0.f) {
//        pl->m_pSkinnedAnimationController->SetTrackEnable(1, true);
//    }
//    else
//    {
//        pl->m_pSkinnedAnimationController->SetTrackEnable(0, true);
//        pl->m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
//    }
//}

void ProcessPacket(char* ptr)//몬스터 생성
{
	switch (ptr[1]) {
	case SC_GAME_START: {
		gGameFramework.lobby[2] = true;
		gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;
		gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[1]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;
		SC_GAME_START_PACKET* packet = reinterpret_cast<SC_GAME_START_PACKET*>(ptr);
		gGameFramework.m_pPlayer->c_id = packet->id;
		gGameFramework.m_pPlayer->SetPosition(packet->pos);
		gGameFramework.m_pPlayer->recv_time = high_resolution_clock::now();
		gGameFramework.m_pPlayer->alive = true;
		cout << "접속 완료, id = " << gGameFramework.m_pPlayer->c_id << endl;
		break;
	}
	case SC_ADD_PLAYER: {
		SC_ADD_PLAYER_PACKET* packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		cout << "client[" << packet->id << "] Accessed\n";
		gGameFramework.CreateOtherPlayer(packet->id, packet->Pos);
		break;
	}
	case SC_SIGNIN: {
		SC_SIGN_PACKET* packet = reinterpret_cast<SC_SIGN_PACKET*>(ptr);
		if (packet->success == true)
		{
			gGameFramework.loginSign[1] = true;
			gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0] = gGameFramework.m_pStage->m_ppShaders[0]->popUpMat[0];
			wcout << "SIGNIN SUCCEED\n";
		}
		else {
			gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0] = gGameFramework.m_pStage->m_ppShaders[0]->popUpMat[1];
			wcout << "SIGNIN FAILED\n";

			while (!gGameFramework.userId.empty())
				gGameFramework.userId.pop_back();

			while (!gGameFramework.userPw.empty())
				gGameFramework.userPw.pop_back();

			gGameFramework.idSet = false;
		}

		gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
		break;
	}
	case SC_SIGNUP: {
		SC_SIGN_PACKET* packet = reinterpret_cast<SC_SIGN_PACKET*>(ptr);
		if (packet->success == true) {
			gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0] = gGameFramework.m_pStage->m_ppShaders[0]->popUpMat[2];
			wcout << "SIGNUP SUCCEED\n";
		}
		else {
			wcout << "SIGNUP FAILED\n";
			gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0] = gGameFramework.m_pStage->m_ppShaders[0]->popUpMat[3];
		}

		while (!gGameFramework.userId.empty())
			gGameFramework.userId.pop_back();

		while (!gGameFramework.userPw.empty())
			gGameFramework.userPw.pop_back();

		gGameFramework.idSet = false;
		gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[10]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
		break;
	}

	case SC_REMOVE_PLAYER: {
		SC_REMOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		auto iter = find_if(gGameFramework.Players.begin(), gGameFramework.Players.end(), [packet](CPlayer* pl) {return packet->id == pl->c_id; });
		(*iter)->c_id = -1;
		cout << "client[" << packet->id << "] Disconnected\n";
		break;
	}
	case SC_UPDATE_PLAYER: {
		SC_UPDATE_PLAYER_PACKET* packet = reinterpret_cast<SC_UPDATE_PLAYER_PACKET*>(ptr);
		auto iter = find_if(gGameFramework.Players.begin(), gGameFramework.Players.end(), [packet](CPlayer* pl) {return packet->id == pl->c_id; });
		if (iter == gGameFramework.Players.end()) break;

		(*iter)->packet_HP = packet->HP;

		if (packet->HP <= 0) {
			(*iter)->onAct = true;
			(*iter)->alive = false;
			(*iter)->cxDelta = (*iter)->cyDelta = (*iter)->czDelta = 0;
			(*iter)->m_pSkinnedAnimationController->SetTrackEnable((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track, false);
			(*iter)->m_pSkinnedAnimationController->SetTrackEnable(4, true);
			return;
		}
		if ((*iter)->onAct == false) {
			//auto cur_time = high_resolution_clock::now();
			//float time = duration_cast<milliseconds>(cur_time - (*iter)->recv_time).count() / 1000.f;
			//float fLength = sqrtf(packet->vel.x * packet->vel.x + packet->vel.z * packet->vel.z);
			//if (fLength > 100)
			//{
			//    packet->vel.x *= (100 / fLength);
			//    packet->vel.z *= (100 / fLength);
			//}

			//fLength = sqrtf(packet->vel.y * packet->vel.y);
			//if (fLength > 300) packet->vel.y *= (300 / fLength);

			//(*iter)->Deceleration(time);
			//XMFLOAT3 targetPos = Vector3::Add((*iter)->GetPosition(), Vector3::ScalarProduct(packet->vel, time, false));
			//(*iter)->SetVelocity(packet->vel);  
			//(*iter)->SetPosition(targetPos);
			//(*iter)->recv_time = cur_time;

			XMFLOAT3 deltaPos = Vector3::Subtract(packet->Pos, (*iter)->GetPosition());
			XMFLOAT3 targetPos = Vector3::Add((*iter)->GetPosition(), Vector3::ScalarProduct(deltaPos, 0.1f, false));
			if ((*iter) != gGameFramework.m_pPlayer) {
				(*iter)->SetVelocity(packet->vel);
			}
			(*iter)->SetPosition(targetPos);
		}

		if (0 == packet->id % 3)
		{
			if (0 != packet->HP && gGameFramework.beforeHp[0] != packet->HP)
			{
				gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[5]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
				gGameFramework.beforeHp[0] = packet->HP;
				gGameFramework.plTime[0] = 0.f;
			}
		}
		else if (1 == packet->id % 3)
		{
			if (0 != packet->HP && gGameFramework.beforeHp[1] != packet->HP)
			{
				gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[6]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[1] = true;
				gGameFramework.beforeHp[1] = packet->HP;
				gGameFramework.plTime[1] = 0.f;
			}
		}
		else if (2 == packet->id % 3)
		{
			if (0 != packet->HP && gGameFramework.beforeHp[2] != packet->HP)
			{
				gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[7]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[2] = true;
				gGameFramework.beforeHp[2] = packet->HP;
				gGameFramework.plTime[2] = 0.f;
			}
		}

		break;
	}
	case SC_ROTATE_PLAYER: {
		SC_ROTATE_PLAYER_PACKET* packet = reinterpret_cast<SC_ROTATE_PLAYER_PACKET*>(ptr);
		auto iter = find_if(gGameFramework.Players.begin(), gGameFramework.Players.end(), [packet](CPlayer* pl) {return packet->id == pl->c_id; });
		if (iter == gGameFramework.Players.end() || (*iter) == gGameFramework.m_pPlayer) break;
		(*iter)->SetLookVector(packet->Look);
		(*iter)->SetRightVector(packet->Right);
		(*iter)->SetUpVector(Vector3::CrossProduct((*iter)->GetLookVector(), (*iter)->GetRightVector(), true));
		break;
	}
	case SC_ATTACK: {
		SC_ATTACK_PACKET* packet = reinterpret_cast<SC_ATTACK_PACKET*>(ptr);
		auto iter = find_if(gGameFramework.Players.begin(), gGameFramework.Players.end(), [packet](CPlayer* pl) {return packet->id == pl->c_id; });
		if (iter == gGameFramework.Players.end()) break;
		(*iter)->onAct = true;
		(*iter)->SetVelocity(XMFLOAT3(0, 0, 0));
		(*iter)->m_pSkinnedAnimationController->SetTrackEnable((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track, false);
		(*iter)->m_pSkinnedAnimationController->SetTrackEnable(2, true);

		if (0 == (*iter)->c_id && 1 == gGameFramework.curAtt[0])
			gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[0]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
		else if (1 == (*iter)->c_id && 1 == gGameFramework.curAtt[1])
			gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[8]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[1] = true;
		else if (2 == (*iter)->c_id && 1 == gGameFramework.curAtt[2])
			gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[9]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[2] = true;

		break;
	}
	case SC_CHANGEWEAPON: {
		SC_CHANGEWEAPON_PACKET* packet = reinterpret_cast<SC_CHANGEWEAPON_PACKET*>(ptr);
		auto iter = find_if(gGameFramework.Players.begin(), gGameFramework.Players.end(), [packet](CPlayer* pl) {return packet->id == pl->c_id; });
		int cur_track = (*iter)->m_pSkinnedAnimationController->Cur_Animation_Track;
		(*iter)->m_pChild = (*iter)->pAngrybotModels[packet->cur_weaponType]->m_pModelRootObject;
		(*iter)->m_pSkinnedAnimationController = (*iter)->AnimationControllers[packet->cur_weaponType];

		//if (packet->id == gGameFramework.m_pPlayer->c_id)
		//    if (packet->cur_weaponType == 1) {
		//        gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[11]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
		//    }
		//    else {
		//        gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[11]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = false;
		//    }


		for (int i{}; i < 3; ++i)
		{
			if (i == (*iter)->c_id)
			{
				switch (packet->cur_weaponType)
				{
				case 0:
					gGameFramework.curAtt[i] = 0;
					break;
				case 1:
					gGameFramework.curAtt[i] = 1;
					break;
				case 2:
					gGameFramework.curAtt[i] = 2;
					break;
				}
			}
		}

		for (int i = 0; i < 6; i++)
		{
			(*iter)->m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
			(*iter)->m_pSkinnedAnimationController->SetTrackEnable(i, false);
		}
		(*iter)->m_pSkinnedAnimationController->SetTrackEnable(cur_track, true);
		break;
	}
	case SC_SUMMON_MONSTER: {
		SC_SUMMON_MONSTER_PACKET* packet = reinterpret_cast<SC_SUMMON_MONSTER_PACKET*>(ptr);

		gGameFramework.SummonMonster(packet->id, packet->monster_type, packet->Pos);//debugMode


		break;
	}
	case SC_MOVE_MONSTER: {
		SC_MOVE_MONSTER_PACKET* packet = reinterpret_cast<SC_MOVE_MONSTER_PACKET*>(ptr);
		auto iter = find_if(gGameFramework.m_pStage->Monsters.begin(), gGameFramework.m_pStage->Monsters.end(), [packet](CMonster* Mon) {return packet->id == Mon->c_id; });
		if (iter == gGameFramework.m_pStage->Monsters.end()) return;
		if (packet->is_alive == false) {
			short type = (*iter)->npc_type;
			gGameFramework.pMonsterModel[type].push((*iter)->_Model);   // 받아온 모델타입 다시 큐로 반환

			if ((*iter)->npc_type == 2)
			{
				gGameFramework.MagiciansHat.push((*iter)->Hat_Model);
			}
			(*iter)->Sound.Stop();
			(*iter)->Sound.Terminate();
			gGameFramework.m_pStage->Monsters.erase(iter);
			break;
		}
		if ((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track != packet->animation_track) {
			if ((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track != 3 && packet->animation_track != 0) {
				(*iter)->Sound.Stop();
				(*iter)->Sound.LoadWave(gGameFramework.monster[packet->animation_track - 1], 1);
				(*iter)->Sound.Play();
			}

			if (packet->animation_track > (*iter)->m_pSkinnedAnimationController->Cur_Animation_Track) {
				auto target = find_if(gGameFramework.Players.begin(), gGameFramework.Players.end(), [packet](CPlayer* Pl) {return packet->target_id == Pl->c_id; });
				if (target != gGameFramework.Players.end()) {
					XMFLOAT3 pos = (*iter)->GetPosition();
					XMFLOAT4X4 mtkLookAt = Matrix4x4::LookAtLH((*iter)->GetPosition(), (*target)->GetPosition(), XMFLOAT3(0, 1, 0));
					mtkLookAt._11 = -mtkLookAt._11;
					mtkLookAt._21 = -mtkLookAt._21;
					mtkLookAt._31 = -mtkLookAt._31;
					(*iter)->m_xmf4x4ToParent = mtkLookAt;
					(*iter)->SetPosition(pos);
				}
			}

			(*iter)->m_pSkinnedAnimationController->SetTrackPosition((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track, 0.0f);
			(*iter)->m_pSkinnedAnimationController->SetTrackEnable((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track, false);
			(*iter)->m_pSkinnedAnimationController->SetTrackEnable(packet->animation_track, true);
		}

		if ((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track == 1) {
			XMFLOAT3 FROM = (*iter)->GetPosition();
			XMFLOAT3 TO = packet->Pos;
			XMFLOAT4X4 mtkLookAt = Matrix4x4::LookAtLH(FROM, TO, XMFLOAT3(0, 1, 0));
			mtkLookAt._11 = -mtkLookAt._11;
			mtkLookAt._21 = -mtkLookAt._21;
			mtkLookAt._31 = -mtkLookAt._31;
			(*iter)->m_xmf4x4ToParent = mtkLookAt;

			XMFLOAT3 deltaPos = Vector3::Subtract(packet->Pos, FROM);
			XMFLOAT3 targetPos = Vector3::Add(FROM, Vector3::ScalarProduct(deltaPos, 0.1f, false));
			(*iter)->m_xmOOBB.Center = targetPos;
			(*iter)->m_xmf3Velocity = Vector3::ScalarProduct(Vector3::Normalize(deltaPos), (*iter)->speed, false);
			(*iter)->SetPosition(targetPos);
		}
		else if ((*iter)->npc_type == 2 && (*iter)->m_pSkinnedAnimationController->Cur_Animation_Track == 2) {
			auto target = find_if(gGameFramework.Players.begin(), gGameFramework.Players.end(), [packet](CPlayer* Pl) {return packet->target_id == Pl->c_id; });
			if (target != gGameFramework.Players.end()) {
				XMFLOAT3 FROM = (*iter)->GetPosition();
				XMFLOAT3 TO = (*target)->GetPosition();
				TO.y = FROM.y;
				XMFLOAT4X4 mtkLookAt = Matrix4x4::LookAtLH(FROM, TO, XMFLOAT3(0, 1, 0));
				mtkLookAt._11 = -mtkLookAt._11;
				mtkLookAt._21 = -mtkLookAt._21;
				mtkLookAt._31 = -mtkLookAt._31;
				(*iter)->m_xmf4x4ToParent = mtkLookAt;
				(*iter)->SetPosition(FROM);
			}
		}
		(*iter)->m_ppHat->SetPosition(packet->BulletPos);
		break;
	}
	case SC_OPEN_DOOR: {
		SC_OPEN_DOOR_PACKET* packet = reinterpret_cast<SC_OPEN_DOOR_PACKET*>(ptr);
		int cur_stage = packet->door_num;
		gGameFramework.openDoor[cur_stage] = true;
		gGameFramework.m_pStage->m_ppShaders[0]->door[cur_stage]->obBox.Center.y -= 10000.f;
		break;
	}
	case SC_GAME_CLEAR: {
		gGameFramework.m_pPlayer->onAct = true;
		gGameFramework.m_pPlayer->alive = false;
		gGameFramework.m_pPlayer->cxDelta = gGameFramework.m_pPlayer->cyDelta = gGameFramework.m_pPlayer->czDelta = 0.0f;
		//gGameFramework.m_pPlayer->cxDelta = gGameFramework.m_pPlayer->cyDelta = gGameFramework.m_pPlayer->czDelta = 0;
		//gGameFramework.m_pCamera->SetPosition(XMFLOAT3(800, -150, 700));
		//gGameFramework.m_pCamera->SetLookAt(XMFLOAT3(800, -150, 800));
		//gGameFramework.m_pCamera->m_lock = true;

		gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0] = gGameFramework.m_pStage->m_ppShaders[0]->gameMat[1];
		gGameFramework.m_pStage->pMultiSpriteObjectShader->obj[4]->m_ppMaterials[0]->m_ppTextures[0]->m_bActive[0] = true;
		gGameFramework.lobby[2] = false;
		gGameFramework.gameEnd = true;

		//gGameFramework.monsterSound.Stop();//몬스터
		gGameFramework.sound[0].Stop();//인게임
		gGameFramework.sound[3].Play();//윈
		break;
	}
	case SC_INTERACTION: {
		SC_INTERACTION_PACKET* packet = reinterpret_cast<SC_INTERACTION_PACKET*>(ptr);
		gGameFramework.m_pStage->pPuzzles[packet->stage_id][packet->obj_id]->m_bGetItem = true;
		gGameFramework.m_pStage->DeleteObject.push_back(gGameFramework.m_pStage->pPuzzles[packet->stage_id][packet->obj_id]->m_iObjID);
		break;
	}
	case SC_MONSTER_DAMAGED: {
		SC_MONSTER_DAMAGED_PACKET* packet = reinterpret_cast<SC_MONSTER_DAMAGED_PACKET*>(ptr);

		auto iter = find_if(gGameFramework.m_pStage->Monsters.begin(), gGameFramework.m_pStage->Monsters.end(), [packet](CMonster* Mon) {return packet->monster_id == Mon->c_id; });
		if (iter == gGameFramework.m_pStage->Monsters.end()) return;

		if (packet->player_id == gGameFramework.m_pPlayer->c_id) {
			gGameFramework.damagedMon = packet->monster_id;
			gGameFramework.m_pPlayer->gun_hit = 1;
			gGameFramework.hitSound.Stop();
			gGameFramework.hitSound.LoadWave(gGameFramework.hit_marker, 1);
			gGameFramework.hitSound.Play();
			(*iter)->damaged = true;
			(*iter)->damaged_timer = 0.f;
		}
		(*iter)->HP = packet->remain_HP;
		if ((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track < 2 && packet->remain_HP > 0) {
			(*iter)->m_pSkinnedAnimationController->SetTrackPosition((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track, 0.0f);
			(*iter)->m_pSkinnedAnimationController->SetTrackEnable((*iter)->m_pSkinnedAnimationController->Cur_Animation_Track, false);
			(*iter)->m_pSkinnedAnimationController->SetTrackEnable(3, true);
		}
		break;
	}
	}
}


void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
	delete ex_over;
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	char* ptr = recv_over._send_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (num_bytes != 0) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (num_bytes + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);

			ptr += in_packet_size - saved_packet_size;
			//ptr += in_packet_size;
			num_bytes -= int(in_packet_size - saved_packet_size);
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, num_bytes);
			saved_packet_size += num_bytes;
			num_bytes = 0;
		}
	}
	do_recv();
}



void do_recv()
{
	DWORD r_flag = 0;
	memset(&recv_over._over, 0, sizeof(recv_over._over));
	recv_over._wsabuf.len = BUF_SIZE;
	recv_over._wsabuf.buf = recv_over._send_buf;
	int ret = WSARecv(s_socket, &recv_over._wsabuf, 1, NULL, &r_flag, &recv_over._over, recv_callback);
	if (ret != 0 && WSAGetLastError() != ERROR_IO_PENDING) err_display("WSARecv()");
}