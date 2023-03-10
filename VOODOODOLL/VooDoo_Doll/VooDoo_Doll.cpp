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
char	recv_buffer[BUF_SIZE];
thread* recv_t;
DWORD Old_Direction = 0;
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

void GamePlayer_ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	DWORD dwDirection = 0;
	if (::GetKeyboardState(pKeysBuffer))
	{
		if (!gGameFramework.m_pPlayer->onAttack && !gGameFramework.m_pPlayer->onDie && !gGameFramework.m_pPlayer->onCollect) {
			if (pKeysBuffer[0x57] & 0xF0) dwDirection |= DIR_FORWARD;//w
			if (pKeysBuffer[0x53] & 0xF0) dwDirection |= DIR_BACKWARD;//s
			if (pKeysBuffer[0x41] & 0xF0) dwDirection |= DIR_LEFT;//a
			if (pKeysBuffer[0x44] & 0xF0) dwDirection |= DIR_RIGHT;//d
			if (pKeysBuffer[0x58] & 0xF0 && dwDirection) dwDirection |= DIR_RUN;//x run


			else if (pKeysBuffer[0x51] & 0xF0) dwDirection = DIR_CHANGESTATE;//q change
			else if (pKeysBuffer[0x5A] & 0xF0) dwDirection = DIR_ATTACK;//z Attack
			else if (pKeysBuffer[0x43] & 0xF0) dwDirection = DIR_COLLECT;//c collect
			else if (pKeysBuffer[0x4B] & 0xF0) dwDirection = DIR_DIE;//k die 
		}
	}

	float cxDelta = 0.0f, cyDelta = 0.0f;
	gGameFramework.m_pPlayer->cxDelta = gGameFramework.m_pPlayer->cyDelta = gGameFramework.m_pPlayer->czDelta = 0.0f;
	if (GetCapture() == gGameFramework.Get_HWND())
	{
		::SetCursor(NULL);
		POINT ptCursorPos;
		::GetCursorPos(&ptCursorPos);
		cxDelta = (float)(ptCursorPos.x - gGameFramework.Get_OldCursorPointX()) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - gGameFramework.Get_OldCursorPointY()) / 3.0f;
		::SetCursorPos(gGameFramework.Get_OldCursorPointX(), gGameFramework.Get_OldCursorPointY());
	}

	if (dwDirection) gGameFramework.m_pPlayer->Move(dwDirection, 7.0, true);


	if ((dwDirection != Old_Direction) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		CS_MOVE_PACKET p;
		p.direction = dwDirection;
		p.id = gGameFramework.m_pPlayer->c_id;
		p.size = sizeof(CS_MOVE_PACKET);
		p.type = CS_MOVE;
		p.pos = gGameFramework.m_pPlayer->GetPosition();
		if (cxDelta || cyDelta)
		{
			if (pKeysBuffer[VK_RBUTTON] & 0xF0) {
				gGameFramework.m_pPlayer->cxDelta = p.cxDelta = cyDelta;
				gGameFramework.m_pPlayer->cyDelta = p.cyDelta = 0.f;
				gGameFramework.m_pPlayer->czDelta = p.czDelta = -cxDelta;
			}
			else {
				gGameFramework.m_pPlayer->cxDelta = p.cxDelta = cyDelta;
				gGameFramework.m_pPlayer->cyDelta = p.cyDelta = cxDelta;
				gGameFramework.m_pPlayer->czDelta = p.czDelta = 0.f;
			}

		}

		int ErrorStatus = send(s_socket, (char*)&p, sizeof(CS_MOVE_PACKET), 0);
		if (ErrorStatus == SOCKET_ERROR)
			cout << "Move_Packet Error\n";
		Old_Direction = dwDirection;
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

	//clienttest
#pragma region SERVER

	WSADATA WSAData;
	int ErrorStatus = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ErrorStatus != 0)
	{
		cout << "WSAStartup 실패\n";
	}
	s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	if (s_socket == INVALID_SOCKET)
	{
		cout << "소켓 생성 실패\n";
	}


	// 서버와 연결
	SOCKADDR_IN svr_addr;
	memset(&svr_addr, 0, sizeof(svr_addr));
	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, "127.0.0.1", &svr_addr.sin_addr);
	ErrorStatus = WSAConnect(s_socket, reinterpret_cast<sockaddr*>(&svr_addr), sizeof(svr_addr), 0, 0, 0, 0);
	if (ErrorStatus == SOCKET_ERROR) err_quit("WSAConnect()");

	// 서버에게 자신의 정보를 패킷으로 전달
	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	ErrorStatus = send(s_socket, reinterpret_cast<char*>(&p), p.size, 0);
	if (ErrorStatus == SOCKET_ERROR) err_quit("send()");

	recv_t = new thread{ RecvThread };	// 서버가 보내는 패킷을 받는 스레드 생성

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
			//clienttest
			GamePlayer_ProcessInput();// 서버를 적용했을 경우 사용하는 ProcessInput 함수
			gGameFramework.FrameAdvance();
		}
	}

	//clienttest
	recv_t->join();
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

	//full screen
	//#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	if (true == gGameFramework.onFullScreen)
		gGameFramework.ChangeSwapChainState();
	else
		gGameFramework.CreateRenderTargetViews();
	//#endif
	//

	return(TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		//23.01.03
	//p를 누르면 종료
	case WM_CHAR:
		if (wParam == 'P' || wParam == 'p')
			PostQuitMessage(0);
		else if (wParam == 'N' || wParam == 'n')
		{
			if (true == gGameFramework.wakeUp)
				gGameFramework.wakeUp = false;
			else
				gGameFramework.wakeUp = true;
		}

		//else if (wParam == 'Q' || wParam == 'q')
		//{
		//	gGameFramework.changePlayerMode = true;
		//	//gGameFramework.p1Change = true;


		//	if (1 == gGameFramework.whatPlayer)
		//	{
		//		gGameFramework.whatPlayer = 2;
		//	}
		//	else if (2 == gGameFramework.whatPlayer)
		//	{
		//		gGameFramework.whatPlayer = 3;
		//		//gGameFramework.m_pPlayer->SetPosition(gGameFramework.pPlayer->GetPosition());
		//	}
		//	else if (3 == gGameFramework.whatPlayer)
		//	{
		//		gGameFramework.whatPlayer = 1;
		//		//gGameFramework.m_pPlayer->SetPosition(gGameFramework.pPlayer->GetPosition());
		//	}
		//}
		break;
		//

	case WM_KEYDOWN:
		if (wParam == 'L' || wParam == 'l')//full screen
		{
			gGameFramework.onFullScreen = true;
			gGameFramework.ChangeSwapChainState();
		}
		break;
	case WM_KEYUP:
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_SIZE:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
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

void ProcessAnimation(CPlayer* pl, SC_MOVE_PLAYER_PACKET* p)//0228
{
	if (0 == pl->m_pSkinnedAnimationController->Cur_Animation_Track || 1 == pl->m_pSkinnedAnimationController->Cur_Animation_Track ||
		3 == pl->m_pSkinnedAnimationController->Cur_Animation_Track)
		pl->m_pSkinnedAnimationController->SetTrackEnable(pl->m_pSkinnedAnimationController->Cur_Animation_Track, false);
	//else
	//{
	//	if(false == pl->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable)
	//		pl->m_pSkinnedAnimationController->SetTrackEnable(pl->m_pSkinnedAnimationController->Cur_Animation_Track, false);
	//	else if(false== pl->onDie && p->direction)
	//		pl->m_pSkinnedAnimationController->SetTrackEnable(pl->m_pSkinnedAnimationController->Cur_Animation_Track, false);
	//}

	XMFLOAT3 Cmp = Vector3::Subtract(pl->GetPosition(), p->Pos);

	if (p->direction & DIR_ATTACK) pl->onAttack = true;
	if (p->direction & DIR_RUN) pl->onRun = true; else pl->onRun = false;
	if (p->direction & DIR_DIE) pl->onDie = true;
	if (p->direction & DIR_COLLECT) pl->onCollect = true;
	if (p->direction & DIR_CHANGESTATE)
	{
		cout << p->character_num << endl;

		pl->m_pChild = pl->pAngrybotModels[p->character_num]->m_pModelRootObject;
		pl->m_pSkinnedAnimationController = pl->AnimationControllers[p->character_num];
		for (int i = 0; i < 6; i++)
		{
			pl->m_pSkinnedAnimationController->SetTrackAnimationSet(i, i);
			pl->m_pSkinnedAnimationController->SetTrackEnable(i, false);
		}
	}


	if (pl->onAttack) {
		pl->m_pSkinnedAnimationController->SetTrackEnable(2, true);
		return;
	}
	else if (pl->onRun) {
		pl->m_pSkinnedAnimationController->SetTrackEnable(3, true);
		return;
	}
	else if (pl->onDie) {
		pl->m_pSkinnedAnimationController->SetTrackEnable(4, true);
		return;
	}
	else if (pl->onCollect) {
		pl->m_pSkinnedAnimationController->SetTrackEnable(5, true);
		return;
	}


	else if (!pl->onAttack && !pl->onRun && !pl->m_pSkinnedAnimationController->m_pAnimationTracks[4].m_bEnable && !pl->onCollect) {
		if (Vector3::IsZero(Cmp)) {
			pl->m_pSkinnedAnimationController->SetTrackEnable(0, true);
			pl->m_pSkinnedAnimationController->SetTrackPosition(1, 0.0f);
		}
		else
		{
			pl->m_pSkinnedAnimationController->SetTrackEnable(1, true);
		}
	}

}

void ProcessPacket(char* ptr)//몬스터 생성
{
	switch (ptr[1]) {
	case SC_LOGIN_INFO: {
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		gGameFramework.m_pPlayer->c_id = packet->id;
		gGameFramework.m_pPlayer->SetPosition(packet->pos);
		cout << "접속 완료, id = " << gGameFramework.m_pPlayer->c_id << endl;
		break;
	}
	case SC_ADD_PLAYER: {
		SC_ADD_PLAYER_PACKET* packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		cout << "client[" << packet->id << "] Accessed\n";
		gGameFramework.CreateOtherPlayer(packet->id, packet->Pos, packet->Look, packet->Up, packet->Right);
		break;
	}
	case SC_SUMMON_MONSTER: {
		SC_SUMMON_MONSTER_PACKET* packet = reinterpret_cast<SC_SUMMON_MONSTER_PACKET*>(ptr);
		cout << packet->id << " Monster SUMMONED - " << packet->Pos.x << ", " << packet->Pos.y << ", " << packet->Pos.z << endl;
		gGameFramework.SummonMonster(packet->id, packet->monster_type, packet->Pos);
		break;
	}
	case SC_REMOVE_PLAYER: {
		SC_REMOVE_PLAYER_PACKET* packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		for (CPlayer*& player : gGameFramework.Players)
			if (player->c_id == packet->id) {
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
			ProcessAnimation(gGameFramework.m_pPlayer, packet);
			gGameFramework.m_pPlayer->SetPosition(packet->Pos);
		}
		else
			for (auto& player : gGameFramework.Players)
				if (packet->id == player->c_id) {
					player->SetLookVector(packet->Look);
					player->SetUpVector(packet->Up);
					player->SetRightVector(packet->Right);
					ProcessAnimation(player, packet);
					player->SetPosition(packet->Pos);
					break;
				}
		break;
	}
	case SC_MOVE_MONSTER: {
		SC_MOVE_MONSTER_PACKET* packet = reinterpret_cast<SC_MOVE_MONSTER_PACKET*>(ptr);
		for (auto& monster : gGameFramework.Monsters)
		{
			if (packet->id == monster->c_id) {
				if (packet->HP <= 0) {
					monster->c_id = -1;
					break;
				}
				if (monster->m_pSkinnedAnimationController->Cur_Animation_Track != packet->animation_track) {
					//cout << "packetTrack - " << packet->animation_track << ", " << "curTrack - " << monster->m_pSkinnedAnimationController->Cur_Animation_Track << endl;
					monster->m_pSkinnedAnimationController->SetTrackEnable(monster->m_pSkinnedAnimationController->Cur_Animation_Track, false);
					monster->m_pSkinnedAnimationController->SetTrackEnable(packet->animation_track, true);
				} 
				//if (Vector3::Compare(monster->GetPosition(), packet->Pos)) 
				XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(packet->Pos, gGameFramework.m_pPlayer->GetPosition(), gGameFramework.m_pPlayer->GetUpVector());
				monster->m_xmf4x4ToParent = mtxLookAt;
				cout << monster->GetLook().x << monster->GetLook().y << monster->GetLook().z << endl;
				monster->UpdateTransform(NULL);
				monster->SetPosition(packet->Pos);
				break;
			}
		}
		break;
	}
	}
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
			io_byte -= int(in_packet_size - saved_packet_size);//'-=': 'size_t'에서 'int'(으)로 변환하면서 데이터가 손실될 수 있습니다.
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
	while (1)
	{
		int Length = recv(s_socket, recv_buffer, BUF_SIZE, 0);
		if (Length == 0)
			return;
		ProcessData(recv_buffer, Length);
	}
}