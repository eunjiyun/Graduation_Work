#include "stdafx.h"
#include "Login.h"

CLogin::CLogin()
{
}

CLogin::~CLogin()
{
}

bool CLogin::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CLogin::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

void CLogin::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CLogin::ReleaseObjects()
{
}

void CLogin::BuildLightsAndMaterials()
{
}

ID3D12RootSignature* CLogin::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	return nullptr;
}

void CLogin::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CLogin::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CLogin::ReleaseShaderVariables()
{
}

bool CLogin::ProcessInput(UCHAR* pKeysBuffer)
{
	return false;
}

void CLogin::AnimateObjects(float fTimeElapsed)
{
}

void CLogin::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
}

void CLogin::ReleaseUploadBuffers()
{
}
