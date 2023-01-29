
#include "stdafx.h"
#include "VOODOO DOLL.h"
#include "GameFramework.h"
#include "SERVER.h"

#define MAX_LOADSTRING 100

#pragma region S_variables
// 서버 통신에 사용되는 변수들의 집합
constexpr short SERVER_PORT = 3500;
SOCKET s_socket;
char	recv_buffer[BUF_SIZE];
thread* recv_t;
OVER_EXP _over;
mutex m;
#pragma endregion


HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];

CGameFramework					gGameFramework;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

void RecvThread();

// 23.01.27
void GamePlayer_ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	DWORD dwDirection = 0;

	if (::GetKeyboardState(pKeysBuffer))
	{
		if (pKeysBuffer[0x57] & 0xF0) dwDirection |= DIR_FORWARD;//w
		if (pKeysBuffer[0x53] & 0xF0) dwDirection |= DIR_BACKWARD;//s
		if (pKeysBuffer[0x41] & 0xF0) dwDirection |= DIR_LEFT;//a
		if (pKeysBuffer[0x44] & 0xF0) dwDirection |= DIR_RIGHT;//d
		if (pKeysBuffer[0x58] & 0xF0) dwDirection |= DIR_UP;//x
		if (pKeysBuffer[0x43] & 0xF0) dwDirection |= DIR_DOWN;//c
	}

	float cxDelta = 0.0f, cyDelta = 0.0f;
	if (GetCapture() == gGameFramework.Get_HWNG())
	{
		::SetCursor(NULL);
		POINT ptCursorPos;
		::GetCursorPos(&ptCursorPos);
		cxDelta = (float)(ptCursorPos.x - gGameFramework.Get_OldCursorPointX()) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - gGameFramework.Get_OldCursorPointY()) / 3.0f;
		::SetCursorPos(gGameFramework.Get_OldCursorPointX(), gGameFramework.Get_OldCursorPointY());
	}

	if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		CS_MOVE_PACKET p;
		p.id = gGameFramework.m_pPlayer->c_id;
		p.size = sizeof(CS_MOVE_PACKET);
		p.type = CS_MOVE;
		if (cxDelta || cyDelta)
		{
			if (pKeysBuffer[VK_RBUTTON] & 0xF0) {
				p.cxDelta = cyDelta;
				p.cyDelta = 0.f;
				p.czDelta = -cxDelta;
				//gGameFramework.m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
			}
			else {
				p.cxDelta = cyDelta;
				p.cyDelta = cxDelta;
				p.czDelta = 0.f;
				//gGameFramework.m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);				
			}
		}

		if (dwDirection) {
			p.direction = dwDirection;
			//gGameFramework.m_pPlayer->Move(dwDirection, 150.0f * gGameFramework.m_GameTimer.GetTimeElapsed(), true); // Player Velocity 
		}
		p.Pos = gGameFramework.m_pPlayer->GetPosition();
		p.Look = gGameFramework.m_pPlayer->GetLook();
		p.Up = gGameFramework.m_pPlayer->GetUp();
		p.Right = gGameFramework.m_pPlayer->GetRight();
		int ErrorStatus = send(s_socket, (char*)&p, sizeof(CS_MOVE_PACKET), 0);
		if (ErrorStatus == SOCKET_ERROR)
			cout << "Move_Packet Error\n";
	}

	gGameFramework.m_pPlayer->Update(gGameFramework.m_GameTimer.GetTimeElapsed());
	for (auto& player : gGameFramework.Players) {
		if (player->c_id > -1) {
			player->Update(gGameFramework.m_GameTimer.GetTimeElapsed());
		}
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_VOODOODOLL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VOODOODOLL));

//#pragma region SERVER
//
//	WSADATA WSAData;
//	int ErrorStatus = WSAStartup(MAKEWORD(2, 2), &WSAData);
//	if (ErrorStatus != 0)
//	{
//		cout << "WSAStartup 실패\n";
//	}
//	s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
//	if (s_socket == INVALID_SOCKET)
//	{
//		cout << "소켓 생성 실패\n";
//	}
//
//
//	// 서버와 연결
//	SOCKADDR_IN svr_addr;
//	memset(&svr_addr, 0, sizeof(svr_addr));
//	svr_addr.sin_family = AF_INET;
//	svr_addr.sin_port = htons(SERVER_PORT);
//	inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr);
//	ErrorStatus = WSAConnect(s_socket, reinterpret_cast<sockaddr*>(&svr_addr), sizeof(svr_addr), 0, 0, 0, 0);
//	if (ErrorStatus == SOCKET_ERROR) err_quit("WSAConnect()");
//
//	// 서버에게 자신의 정보를 패킷으로 전달
//	CS_LOGIN_PACKET p;
//	p.size = sizeof(CS_LOGIN_PACKET);
//	p.type = CS_LOGIN;
//	ErrorStatus = send(s_socket, reinterpret_cast<char*>(&p), p.size, 0);
//	if (ErrorStatus == SOCKET_ERROR) err_quit("send()");
//
//	recv_t = new thread{ RecvThread };	// 서버가 보내는 패킷을 받는 스레드 생성
//#pragma endregion

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
			gGameFramework.FrameAdvance();
		}
	}

	//recv_t->join();
	gGameFramework.OnDestroy();

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
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_LABPROJECT0891);
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

	switch (message)
	{
		//23.01.28
	case WM_CHAR:
		if (wParam == 'P' || wParam == 'p')
			PostQuitMessage(0);
		else if (wParam == 'N' || wParam == 'n')
		{
			if (true == gGameFramework.wakeUp)
				gGameFramework.wakeUp = false;
			else
				gGameFramework.wakeUp = true;

			/*gGameFramework.pfClearColor[0] = 0.0525f;
			gGameFramework.pfClearColor[1] =  0.0525f;
			gGameFramework.pfClearColor[2] = 0.0525f;
			gGameFramework.pfClearColor[3] = 1.0f ;*/
		}
		break;
		//
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
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

void ProcessPacket(char* ptr)
{
	m.lock();
	switch (ptr[1]) {
	case SC_LOGIN_INFO: {
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		gGameFramework.m_pPlayer->c_id = packet->id;
		gGameFramework.m_pPlayer->SetPosition(XMFLOAT3(packet->x, packet->y, packet->z));
		cout << "접속 완료, id = " << gGameFramework.m_pPlayer->c_id << endl;
		break;
	}
	case SC_ADD_PLAYER: {
		SC_ADD_PLAYER_PACKET* packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		//23.01.23
		int id = packet->id;
		cout << "client[" << packet->id << "] Accessed\n";
		gGameFramework.CreateOtherPlayer(id, packet->Pos, packet->Look, packet->Up, packet->Right);
		break;
	}
	case SC_REMOVE_PLAYER: {
		SC_REMOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		int id = packet->id;
		for (CPlayer*& player : gGameFramework.Players)
			if (player->c_id == id) {
				player->c_id = -1;
				cout << "client[" << packet->id << "] Disconnected\n";
			}
		break;
	}
	case SC_MOVE_PLAYER: {
		SC_MOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);

		if (packet->id == gGameFramework.m_pPlayer->c_id) {
			gGameFramework.m_pPlayer->SetLookVector(packet->Look);
			gGameFramework.m_pPlayer->SetUpVector(packet->Up);
			gGameFramework.m_pPlayer->SetRightVector(packet->Right);
			gGameFramework.m_pPlayer->Move(packet->direction, 150.0f * gGameFramework.m_GameTimer.GetTimeElapsed(), true);
		}
		else
			for (auto& player : gGameFramework.Players)
				if (packet->id == player->c_id) {
					player->SetLookVector(packet->Look);
					player->SetUpVector(packet->Up);
					player->SetRightVector(packet->Right);
					player->Move(packet->direction, 150.0f * gGameFramework.m_GameTimer.GetTimeElapsed(), true);
					break;
				}
		break;
	}
	}
	m.unlock();


}

void ProcessData(char* packet, int io_byte)
{
	char* ptr = packet;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (io_byte != 0) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			//ptr += in_packet_size - saved_packet_size;
			ptr += in_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}



void RecvThread()
{
	while (true) {
		int Length = recv(s_socket, recv_buffer, BUF_SIZE, 0);
		if (Length == 0)
			break;
		ProcessData(recv_buffer, Length);
	}
}