// Copyright (c) 2014 All Right Reserved, http://8bitbear.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// <author>Stephen Wheeler</author>
// <email>bear@8bitbear.com</email>
// <date>2014-01-15</date>
#pragma once
#include "Comms.h"
#include "Common.h"
#include "errorrecord.h"
#include <stdio.h>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "timecounter.h"


using namespace Microsoft::WRL;
using namespace DirectX;

#define PARTNUM 2048

enum RenderMode
{
   Render_DX11Pixel,
   Render_DX11Compute,
   Render_DX12Pixel
};

struct DxAppSetupDesc
{
	HINSTANCE      hInst;
	wchar_t        *windowName;
	wchar_t        *className;
	DWORD          windowStyle;
	int            windowSizeX, windowSizeY;
	int            cmdShow;
	RenderMode     renderMode;
	int            inputType;
	int            timeMode;
	wchar_t        *inFileName, *outFileName;
	wchar_t        *hostName;
};


struct Settings
{
	int width;
	int height;
	
	DXGI_FORMAT colour;
	int refreshRate;

	HINSTANCE hinst;
	bool fullscreen;
	DWORD winStyle;

	RenderMode renderMode;
   bool dx12;

	//gamesettings
	//Todo: Make an enum for inputType bitfield
	int inputType; //1=local, 2=file, 4=internet, 8=record, 16=benchmark, 32 connect. Combine local file and record to add multiple local controllers.
	int timeMode; //used with local inputType, 0 for realtime, or set number of samples per second.

	FILE* inFile;
	FILE* outFile;

	int profile;
};

class DxApp
{
private:
	HWND m_hWnd;
	Settings m_settings;
	Comms m_Comms;

	// Common
	D3D_DRIVER_TYPE         m_driverType;
	D3D_FEATURE_LEVEL       m_featureLevel;
	IDXGIFactory1*          m_pFactory;
	IDXGISwapChain*         m_pSwapChain;
	bool                    m_close;
	int                     m_flip;
	
	// DX11
   struct PartiSimDX11Res
   {
      ID3D11Device*              m_pd3dDevice               = nullptr;
      ID3D11DeviceContext*       m_pImmediateContext        = nullptr;
      ID3D11RasterizerState*     m_pRasterizeNoDepth;
      ID3D11RasterizerState*     m_pRasterizeDepth;
      ID3D11RenderTargetView*    m_pRenderFinalTargetView;
      ID3D11SamplerState*        m_pSamplerState;
      D3D11_VIEWPORT             m_simViewPort;
      D3D11_VIEWPORT             m_finalViewPort;

      // Resources
      ID3D11VertexShader*        m_pVertexShader;
      ID3D11InputLayout*         m_pVertexInput;
      ID3D11PixelShader*         m_pPixelShader;
      ID3D11PixelShader*         m_pSimShader;
      ID3D11ComputeShader*       m_pCSSimShader;
      ID3D11ComputeShader*       m_pCSSortShader[2];
      ID3D11PixelShader*         m_pResetShader;
      ID3D11PixelShader*         m_pHaltShader;
      ID3D11PixelShader*         m_pImplodeShader;
      ID3D11PixelShader*         m_pExplodeShader;
      ID3D11Buffer*              m_pConstantBuffer;
      ID3D11Buffer*              m_pSimInfoCB;
      ID3D11Buffer*              m_pSortInfoCB;
      ID3D11Buffer*              m_pSimInput;

      // Resource Views
      ID3D11Buffer               *m_pTextureBuffer;
      //ID3D11Buffer             *m_pTextureExport; // can be used for debugging
      ID3D11UnorderedAccessView  *m_pTextureBufferUAView;
      ID3D11ShaderResourceView   *m_pTextureView            = nullptr;
      ID3D11Texture2D            *m_pTextureDataBuffer[2];
      //ID3D11Texture2D          *m_pTextureDataExport; // can be used for debugging
      ID3D11ShaderResourceView   *m_pTextureDataView[2];
      ID3D11RenderTargetView     *m_pDataRenderTargetView[2];
      ID3D11UnorderedAccessView  *m_pTextureDataUAView[2];
   
      ID3D11Buffer               *m_pTextureRefBuffer;
      ID3D11ShaderResourceView   *m_pTextureRefView         = nullptr;
   } m_dx11Res;

   //// DX12
   //struct PartiSimDX12Res
   //{
   //   ComPtr<ID3D12Device>                m_d3dDevice;
	  // ComPtr<ID3D12CommandAllocator>      m_commandAllocators[2];
   //   ComPtr<ID3D12Resource>              m_pRenderFinalTargetView[2];
   //   ComPtr<ID3D12CommandQueue>          m_commandQueue;
   //   ComPtr<ID3D12RootSignature>         m_rootSignature;
   //   ComPtr<ID3D12DescriptorHeap>        m_rtvHeap;
   //   ComPtr<ID3D12PipelineState>         m_pipelineState;
   //   ComPtr<ID3D12GraphicsCommandList>   m_commandList;
   //   D3D12_VIEWPORT                      m_simViewPort;
   //   D3D12_VIEWPORT                      m_finalViewPort;
   //   
   //} m_dx12Res;

	

	// App related
   bool                 m_mouse1;
   bool                 m_mouse2;
   int                  m_mousePositionX;
   int                  m_mousePositionY;
   TimePast             m_timer;
   TimePast             m_performanceS;
   TimePast             m_performanceD;
   TimePast             m_performanceR;
   float                m_counterExpectedTime;
   float                m_counterChunk;

   int                  m_oWidth;
   int                  m_oHeight;
   float                m_scale;
   int                  m_noRedraw;
   TimePast             m_holdTime;
   int                  m_simReset, m_simHalt;
   int                  m_first;
public:
	DxApp(void);
	~DxApp(void);

	int Init(DxAppSetupDesc *in_desc);
	

	int Run();
private:
	int SetupWindow(DxAppSetupDesc *in_desc);

   int SetupDxResources();
	int SetupDx11Resources();
   //int SetupDx12Resources();
	int SetupSizeDependentResources();
	void ResizeSizeDependentResources(int width, int height);

   int SetupSizeDependentResources11();
	void ResizeSizeDependentResources11(int width, int height);
   int SetupSizeDependentResources12();
	void ResizeSizeDependentResources12(int width, int height);
	void Render();

	static LRESULT CALLBACK DxApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void OnKeyUp(SHORT vkey);

	bool GetInput(SimInput &input);

	void SimReset();
	void SimZeroVelocity();

	void DisplayBenchmarkComplete();

	void RecordUpdate(wchar_t *filename);
};

struct SimDetailsCB
{
	float Box[4];
	float tscale;
	int   dimensions[2];
};

struct SortDetailsCB
{
	int offset;
	int limit;
};