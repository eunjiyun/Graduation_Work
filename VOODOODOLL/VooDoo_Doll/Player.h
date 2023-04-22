#pragma once

#define BULLETS					50


#include "Object.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CCamera* m_pCamera = NULL;

public:
	CGameObject* m_ppBullet;
	int c_id = -1;
	short cur_weapon = 0;
	bool alive = true;
	short HP = 0;
	float cxDelta, cyDelta, czDelta = 0.0f;
	CLoadedModelInfo* pAngrybotModels[3];
	CAnimationController* AnimationControllers[3];
	HWND						m_hWnd;
public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetPosition2D(const float& _x, const float& _z) { Move(XMFLOAT3(_x - m_xmf3Position.x, 0, _z - m_xmf3Position.z), false); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera* GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);
	
	virtual void playerAttack(int, CGameObject*, CGameObject***) {}
	virtual void playerRun() {}
	virtual void playerDie() {}
	virtual void playerCollect(){}
	

	virtual void Update(float fTimeElapsed);
	virtual void otherPlayerUpdate(float fTimeElapsed) {};

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* m_pd3dGraphicsRootSignature, ID3D12PipelineState* m_pd3dPipelineState,bool shadow, CCamera* pCamera = NULL);


	//230205
	void		OnUpdateTransform();
	void		UpdateBoundingBox();
	void		boundingAnimate(float fElapsedTime);

	void SetLookVector(const XMFLOAT3& xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetUpVector(const XMFLOAT3& xmf3Up) { m_xmf3Up = xmf3Up; }
	void SetRightVector(const XMFLOAT3& xmf3Right) { m_xmf3Right = xmf3Right; }
	void Deceleration(float fTimeElapsed);

};



class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int choosePl, HWND						m_hWnd);
	virtual ~CTerrainPlayer();

public:
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void Move(ULONG nDirection, float fDistance, bool bVelocity = false);

	virtual void Update(float fTimeElapsed);
	virtual void otherPlayerUpdate(float fTimeElapsed);

	virtual void playerAttack(int, CGameObject*, CGameObject***);
	virtual void playerRun();
	virtual void playerDie();
	virtual void playerCollect();
};

class SoundPlayer {
public:
	SoundPlayer() : m_pDSound(NULL), m_pPrimaryBuffer(NULL), m_pSecondaryBuffer(NULL) {}

	~SoundPlayer() {
		Cleanup();
	}

	bool Initialize(HWND hWnd) {
		HRESULT result;

		// DirectSound 객체를 생성합니다.
		result = DirectSoundCreate8(NULL, &m_pDSound, NULL);
		if (FAILED(result)) {
			return false;
		}


		result = m_pDSound->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
		if (FAILED(result)) {
			return false;
		}

		// 프라이머리 버퍼를 생성합니다.
		DSBUFFERDESC primaryDesc;
		ZeroMemory(&primaryDesc, sizeof(primaryDesc));
		primaryDesc.dwSize = sizeof(primaryDesc);
		primaryDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

		result = m_pDSound->CreateSoundBuffer(&primaryDesc, &m_pPrimaryBuffer, NULL);
		if (FAILED(result)) {
			return false;
		}

		// WAV 파일을 로드합니다.
		result = LoadWaveFile("Sound/opening.wav");
		if (FAILED(result)) {
			return false;
		}

		return true;
	}

	void Cleanup() {
		if (m_pSecondaryBuffer) {
			m_pSecondaryBuffer->Stop();
			m_pSecondaryBuffer->Release();
			m_pSecondaryBuffer = NULL;
		}

		if (m_pPrimaryBuffer) {
			m_pPrimaryBuffer->Release();
			m_pPrimaryBuffer = NULL;
		}

		if (m_pDSound) {
			m_pDSound->Release();
			m_pDSound = NULL;
		}
	}

	bool LoadWaveFile(const char* fileName) {
		HRESULT result;

		// WAV 파일을 읽기 위해 MMIO API를 사용합니다.
		HMMIO hFile = mmioOpenA((LPSTR)fileName, NULL, MMIO_ALLOCBUF | MMIO_READ);
		if (!hFile) {
			return false;
		}

		MMCKINFO ckRIFF, ckChild;
		memset(&ckRIFF, 0, sizeof(MMCKINFO));
		memset(&ckChild, 0, sizeof(MMCKINFO));

		// RIFF chunk를 찾습니다.
		ckRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		result = mmioDescend(hFile, &ckRIFF, NULL, MMIO_FINDRIFF);
		if (FAILED(result)) {
			mmioClose(hFile, 0);
			return false;
		}

		// fmt chunk를 찾습니다.
		ckChild.fccType = mmioFOURCC('f', 'm', 't', ' ');
		result = mmioDescend(hFile, &ckChild, &ckRIFF, MMIO_FINDCHUNK);
		if (FAILED(result)) {
			mmioClose(hFile, 0);
			return false;
		}

		// fmt chunk를 읽어
		WAVEFORMATEX waveFormat;
		result = mmioRead(hFile, (HPSTR)&waveFormat, sizeof(waveFormat));
		if (FAILED(result)) {
			mmioClose(hFile, 0);
			return false;
		}

		// 데이터 청크를 찾습니다.
		ckChild.fccType = mmioFOURCC('d', 'a', 't', 'a');
		result = mmioDescend(hFile, &ckChild, &ckRIFF, MMIO_FINDCHUNK);
		if (FAILED(result)) {
			mmioClose(hFile, 0);
			return false;
		}

		// WAV 파일 데이터를 메모리로 로드합니다.
		BYTE* waveData = new BYTE[ckChild.cksize];
		result = mmioRead(hFile, (HPSTR)waveData, ckChild.cksize);
		if (FAILED(result)) {
			delete[] waveData;
			mmioClose(hFile, 0);
			return false;
		}

		// WAV 파일을 재생하기 위해 DirectSound 버퍼를 생성합니다.
		DSBUFFERDESC secondaryDesc;
		ZeroMemory(&secondaryDesc, sizeof(secondaryDesc));
		secondaryDesc.dwSize = sizeof(secondaryDesc);
		secondaryDesc.dwFlags = DSBCAPS_CTRLVOLUME;
		secondaryDesc.dwBufferBytes = ckChild.cksize;
		secondaryDesc.lpwfxFormat = &waveFormat;

		//// DirectSound 객체 생성
		//if (FAILED(DirectSoundCreate8(NULL, &m_pDSound, NULL))) {
		//	// DirectSound 초기화 실패 처리
		//}

		result = m_pDSound->CreateSoundBuffer(&secondaryDesc, &m_pSecondaryBuffer, NULL);
		if (FAILED(result)) {
			delete[] waveData;
			mmioClose(hFile, 0);
			return false;
		}

		// DirectSound 버퍼의 메모리를 잠급니다.
		BYTE* bufferPtr;
		DWORD bufferSize;
		result = m_pSecondaryBuffer->Lock(0, ckChild.cksize, (LPVOID*)&bufferPtr, &bufferSize, NULL, NULL, 0);
		if (FAILED(result)) {
			delete[] waveData;
			mmioClose(hFile, 0);
			return false;
		}

		// WAV 파일 데이터를 DirectSound 버퍼로 복사합니다.
		memcpy(bufferPtr, waveData, ckChild.cksize);

		// DirectSound 버퍼의 메모리를 해제합니다.
		result = m_pSecondaryBuffer->Unlock(bufferPtr, bufferSize, NULL, 0);
		if (FAILED(result)) {
			delete[] waveData;
			mmioClose(hFile, 0);
			return false;
		}

		// WAV 파일 데이터 메모리를 해제합니다.
		delete[] waveData;
		mmioClose(hFile, 0);

		return true;
	}

	void Play() {
		m_pSecondaryBuffer->SetCurrentPosition(0);
		m_pSecondaryBuffer->Play(0, 0, 0);
	}

	private:
		IDirectSound8* m_pDSound;
		IDirectSoundBuffer* m_pPrimaryBuffer;
		IDirectSoundBuffer* m_pSecondaryBuffer;
};

