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
#include "DxApp.h"

#define BOMB_PLACEMENT_LIMIT 3 // Perperson
#define BOMB_LOCATION_LIMIT_X 320

DxApp::DxApp(void)
   : m_close(false), m_mouse1(false), m_mouse2(false), m_timer(),
   m_performanceS(), m_performanceD(), m_performanceR(),
   m_counterExpectedTime(0.f), m_flip(0),
   m_simReset(0), m_simHalt(0), m_simImplode(0), m_simStart(0), m_holdTime(),
   m_first(1), m_live(0), m_explosionCount(0), m_frameCounter(0), m_localBombCounter(0)
{
	m_settings.profile = 0;
}


DxApp::~DxApp(void)
{
	if(m_pFactory)
	{
		m_pFactory->Release();
	}
	if(m_pSwapChain)
	{
		m_pSwapChain->Release();
	}

	m_dx11Res.m_pd3dDevice->Release();
	m_dx11Res.m_pImmediateContext->Release();
	m_dx11Res.m_pRasterizeNoDepth->Release();
	m_dx11Res.m_pRasterizeDepth->Release();
	m_dx11Res.m_pRenderFinalTargetView->Release();
	m_dx11Res.m_pSamplerState->Release();

	m_dx11Res.m_pVertexShader->Release();
	m_dx11Res.m_pVertexInput->Release();
	m_dx11Res.m_pPixelShader->Release();
	m_dx11Res.m_pSimShader->Release();
	m_dx11Res.m_pCSSimShader->Release();
	m_dx11Res.m_pCSSortShader[0]->Release();
	m_dx11Res.m_pResetShader->Release();
	m_dx11Res.m_pHaltShader->Release();
	if(m_settings.inFile)
		fclose(m_settings.inFile);
	if(m_settings.outFile)
		fclose(m_settings.outFile);
}

int DxApp::Init(DxAppSetupDesc *in_desc)
{
	int result=0;
	if(in_desc==0)
	{
		return -1;
	}
	m_settings.hinst = in_desc->hInst;
	m_settings.colour = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_settings.width = in_desc->windowSizeX;
	m_settings.height = in_desc->windowSizeY;
	m_settings.refreshRate = 0;
	m_settings.fullscreen = false;
	m_settings.winStyle = in_desc->windowStyle;
	m_settings.inputType = in_desc->inputType;
	m_settings.timeMode = in_desc->timeMode;
	m_settings.renderMode = in_desc->renderMode;
   m_settings.dx12 = (m_settings.renderMode > Render_DX11Compute);
   m_settings.drawMode = in_desc->draw;

	if((m_settings.inputType & 36) != 0 )
	{
		if((m_settings.inputType & 4) == 4)
		{
			if(m_Comms.CreateTCP()==0)
			{
				while((result = m_Comms.Accept())!=0)
				{
					AppErrors::Record((AppErrors::DxAppErrors)result);
				}
				m_Comms.SendInit(in_desc->timeMode, 30);
			}
		}
		else if((m_settings.inputType & 32) == 32)
		{
			if((result = m_Comms.Connect(in_desc->hostName))==0)
			{
				m_Comms.RecvInit(m_settings.timeMode);
			}
			else
				return result;
		}
	}

	if((m_settings.inputType & 16) == 16)
	{
		in_desc->windowStyle = WS_POPUP | WS_VISIBLE;
	}
	result = SetupWindow(in_desc);
	if(result!=0)
		return result;

	

	result = SetupDxResources();
	if( FAILED( result ) )
	{
		AppErrors::Record((AppErrors::DxAppErrors)result);
		AppErrors::Dump("error.x");
        return result;
	}

	result = SetupSizeDependentResources();

	if(((m_settings.inputType & 2) == 2) && (in_desc->inFileName!=0))
	{
		RecordUpdate(in_desc->inFileName);
	}
	else
	{
		if((m_settings.inputType & 2) == 2)
			return -1;
		m_settings.inFile=NULL;
	}
	if(((m_settings.inputType & 8) == 8) && in_desc->outFileName!=0)
	{
		result = _wfopen_s(&m_settings.outFile, in_desc->outFileName, L"wb");
		if(result!=0)
			m_settings.inputType-=8;
		else
		{
			int version = COMMS_CONNECT_VER;
			fwrite(&version, 4, 1, m_settings.outFile);
			fseek(m_settings.outFile, 12, SEEK_CUR);
		}
	}
	else
	{
		if((m_settings.inputType & 8) == 8)
			m_settings.inputType-=8;
		m_settings.outFile=NULL;
	}

	return result;
}

int DxApp::Run()
{
	MSG msg;
	m_timer.Reset();
	m_performanceS.Reset();
	m_performanceD.Reset();
	m_performanceR.Reset();
	TimePast pcounter = TimePast();
	wchar_t report[32];
	m_counterChunk = 0.f;
	while (!m_close)
	{
		if((PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE)) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_holdTime.Check();
			pcounter.Reset();
			Render();
			m_counterChunk+=pcounter.Peek();
			if(m_settings.profile)
			{
				swprintf(report, L" FUL, %fmsec\n", pcounter.Check()*1000.f);
				OutputDebugString(report);
			}
		}
	}
	return (int) msg.wParam;
}


int DxApp::SetupWindow(DxAppSetupDesc *in_desc)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= NULL;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= in_desc->hInst;
	wcex.hIcon			= nullptr;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= in_desc->className;
	wcex.hIconSm		= nullptr;

	if(RegisterClassEx(&wcex)==0)
		return AppErrors::failedRegister;

	int screenWidth = in_desc->windowSizeX;
	int screenHeight = in_desc->windowSizeY;

	RECT rc = { 0, 0, screenWidth, screenHeight };
	AdjustWindowRect( &rc, in_desc->windowStyle, FALSE );

	int posX = (GetSystemMetrics(SM_CXSCREEN) - (rc.right-rc.left))  / 2;
	int posY = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom-rc.top)) / 2;

	m_hWnd = CreateWindow(in_desc->className, in_desc->windowName, in_desc->windowStyle,
		posX, posY, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, in_desc->hInst, this);

	if(m_hWnd==0)
		return AppErrors::failedCreateWindow;
	
	return AppErrors::noError;
}

LRESULT CALLBACK DxApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

	bool wasHandled = false;
    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        DxApp *pDxApp = (DxApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            (LONG_PTR)pcs->lpCreateParams
            );

        result = 1;
		wasHandled = true;
    }
    else
    {
        DxApp *pDxApp = 0; pDxApp = reinterpret_cast<DxApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                )));

		if (pDxApp)
        {
			wasHandled = true;
			switch(message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pDxApp->ResizeSizeDependentResources(width, height);
				}
				result=1;
                break;
			case WM_MOUSEMOVE:
				pDxApp->m_mousePositionX = (short)LOWORD(lParam);
				pDxApp->m_mousePositionY = (short)HIWORD(lParam);
				break;
			case WM_LBUTTONDOWN:
				pDxApp->m_mouse1=true;
				break;
			case WM_LBUTTONUP:
				pDxApp->m_mouse1=false;
				break;
				case WM_RBUTTONDOWN:
				pDxApp->m_mouse2=true;
				break;
			case WM_RBUTTONUP:
				pDxApp->m_mouse2=false;
				break;
			case WM_KEYUP:
				pDxApp->OnKeyUp(static_cast<SHORT>(wParam));
				break;
			case WM_DESTROY:
				pDxApp->m_close=1;
				PostQuitMessage(0);
				break;
			default:
				return DefWindowProc(hwnd, message, wParam, lParam);
			}
		}
	}
	if(!wasHandled)
		result = DefWindowProc(hwnd, message, wParam, lParam);
	return result;
}

int DxApp::SetupDxResources()
{
   HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1) ,(void**)&m_pFactory);
   if( FAILED( hr ) )
      return hr;
   hr =  SetupDx11Resources();
	return hr;
}

int DxApp::SetupDx11Resources()
{
	HRESULT hr = S_OK;

   D3D_DRIVER_TYPE driverTypes[] =
   {
       D3D_DRIVER_TYPE_HARDWARE,
       D3D_DRIVER_TYPE_WARP,
       D3D_DRIVER_TYPE_REFERENCE,
   };
   UINT numDriverTypes = ARRAYSIZE( driverTypes );
   
   D3D_FEATURE_LEVEL featureLevels[] =
   {
       D3D_FEATURE_LEVEL_11_0 //11_1 could be added here but some windows 7 machines have video cards with support for 11_1 but without 11_1 software being installed resulting in invalid args 
   };
   UINT numFeatureLevels = ARRAYSIZE( featureLevels );
   
   DXGI_SWAP_CHAIN_DESC sd;
   ZeroMemory( &sd, sizeof( sd ) );
   sd.BufferCount = 2;
   sd.BufferDesc.Width = m_settings.width;
   sd.BufferDesc.Height = m_settings.height; 
   sd.BufferDesc.Format = m_settings.colour;
   sd.BufferDesc.RefreshRate.Numerator = 0;
   sd.BufferDesc.RefreshRate.Denominator = 1;
   sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   sd.OutputWindow = m_hWnd;
   sd.SampleDesc.Count = 1;
   sd.SampleDesc.Quality = 0;
   sd.Windowed = !(m_settings.fullscreen);

   UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
   
   for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
   {
   	IDXGISwapChain*			pSwapChain;
      m_driverType = driverTypes[driverTypeIndex];
      hr = D3D11CreateDeviceAndSwapChain( NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &pSwapChain, &m_dx11Res.m_pd3dDevice, &m_featureLevel, &m_dx11Res.m_pImmediateContext );
      if( SUCCEEDED( hr ) )
   	{
         pSwapChain->QueryInterface(&m_pSwapChain);
   		pSwapChain->Release();
         break;
   	}
   	else 
         MessageBox(m_hWnd, L"Warning, Graphics isn't running in full hardware mode", NULL, NULL);
   }
   if( FAILED( hr ) )
      return hr;
   DXGI_FORMAT activeFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

   D3D11_RASTERIZER_DESC drd = 
   {
      D3D11_FILL_SOLID,//D3D11_FILL_MODE FillMode;
      D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
      FALSE,//BOOL FrontCounterClockwise;
      0,//INT DepthBias;
      0.0,//FLOAT DepthBiasClamp;
      0.0,//FLOAT SlopeScaledDepthBias;
      TRUE,//BOOL DepthClipEnable;
      FALSE,//BOOL ScissorEnable;
      TRUE,//BOOL MultisampleEnable;
      FALSE//BOOL AntialiasedLineEnable;   
   };
#ifdef _DEBUG
   drd.CullMode=D3D11_CULL_NONE;
#endif

   m_dx11Res.m_pd3dDevice->CreateRasterizerState(&drd, &m_dx11Res.m_pRasterizeDepth);

	drd.DepthClipEnable = FALSE;

	m_dx11Res.m_pd3dDevice->CreateRasterizerState(&drd, &m_dx11Res.m_pRasterizeNoDepth);
	m_dx11Res.m_pImmediateContext->RSSetState(m_dx11Res.m_pRasterizeNoDepth);

	

	D3D11_SAMPLER_DESC SamDesc = 
   {
      D3D11_FILTER_MIN_MAG_MIP_LINEAR,// D3D11_FILTER Filter;
      D3D11_TEXTURE_ADDRESS_WRAP, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
      D3D11_TEXTURE_ADDRESS_WRAP, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
      D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
      0,//FLOAT MipLODBias;
      0,//UINT MaxAnisotropy;
      D3D11_COMPARISON_NEVER , //D3D11_COMPARISON_FUNC ComparisonFunc;
      0.0,0.0,0.0,0.0,//FLOAT BorderColor[ 4 ];
      0,//FLOAT MinLOD;
      0//FLOAT MaxLOD;   
   };

	m_dx11Res.m_pd3dDevice->CreateSamplerState( &SamDesc, &m_dx11Res.m_pSamplerState );

	// Create a render target view
   ID3D11Texture2D* pBackBuffer = NULL;
   hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
   if( FAILED( hr ) )
      return hr;

   hr = m_dx11Res.m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_dx11Res.m_pRenderFinalTargetView );
   pBackBuffer->Release();
   if( FAILED( hr ) )
      return hr;
	ID3D11DepthStencilView *dsNullview = NULL;
	m_dx11Res.m_pImmediateContext->OMSetRenderTargets( 1, &m_dx11Res.m_pRenderFinalTargetView, dsNullview );


	UINT offset = 0;
	ID3D11Buffer *vb = {NULL};

	m_dx11Res.m_pImmediateContext->IASetInputLayout(NULL);
	m_dx11Res.m_pImmediateContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );


	LPVOID data;
	SIZE_T length;
#ifdef NDEBUG
	const int maxShaderSize = 131072;
#else
	const int maxShaderSize = 131072*8;
#endif
	char* tempbuffer = new char[maxShaderSize]; //max shader size....

	FILE *FP;
	fopen_s(&FP, "VertexShader.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;


	hr = m_dx11Res.m_pd3dDevice->CreateVertexShader( data, length, NULL, &m_dx11Res.m_pVertexShader );

	D3D11_INPUT_ELEMENT_DESC layouttex[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	hr = m_dx11Res.m_pd3dDevice->CreateInputLayout( layouttex, 2, data,
	                                      length, &m_dx11Res.m_pVertexInput );
	
	fopen_s(&FP, "PixelShader.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;
	
	hr = m_dx11Res.m_pd3dDevice->CreatePixelShader( data, length, NULL, &m_dx11Res.m_pPixelShader );
	
	fopen_s(&FP, "sim.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;
	
	hr = m_dx11Res.m_pd3dDevice->CreatePixelShader( data, length, NULL, &m_dx11Res.m_pSimShader );
	
	fopen_s(&FP, "reset.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;
	
	hr = m_dx11Res.m_pd3dDevice->CreatePixelShader( data, length, NULL, &m_dx11Res.m_pResetShader );

   fopen_s(&FP, "startupreset.cso", "rb");
   length = fread(tempbuffer, 1, maxShaderSize, FP);
   fclose(FP);
   data = tempbuffer;

   hr = m_dx11Res.m_pd3dDevice->CreatePixelShader(data, length, NULL, &m_dx11Res.m_pStartupResetShader);
	
	fopen_s(&FP, "halt.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;
	
	hr = m_dx11Res.m_pd3dDevice->CreatePixelShader( data, length, NULL, &m_dx11Res.m_pHaltShader );
	
	fopen_s(&FP, "CSsim.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;
	
	hr = m_dx11Res.m_pd3dDevice->CreateComputeShader( data, length, NULL, &m_dx11Res.m_pCSSimShader );

	fopen_s(&FP, "CSsortH.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;

	hr = m_dx11Res.m_pd3dDevice->CreateComputeShader(data, length, NULL, &m_dx11Res.m_pCSSortShader[0]);

   fopen_s(&FP, "implode.cso", "rb");
   length = fread(tempbuffer, 1, maxShaderSize, FP);
   fclose(FP);
   data = tempbuffer;

   hr = m_dx11Res.m_pd3dDevice->CreatePixelShader(data, length, NULL, &m_dx11Res.m_pImplodeShader);

   fopen_s(&FP, "explode.cso", "rb");
   length = fread(tempbuffer, 1, maxShaderSize, FP);
   fclose(FP);
   data = tempbuffer;

   hr = m_dx11Res.m_pd3dDevice->CreatePixelShader(data, length, NULL, &m_dx11Res.m_pExplodeShader);

   fopen_s(&FP, "sandpit.cso", "rb");
   length = fread(tempbuffer, 1, maxShaderSize, FP);
   fclose(FP);
   data = tempbuffer;

   hr = m_dx11Res.m_pd3dDevice->CreatePixelShader(data, length, NULL, &m_dx11Res.m_pSandpit);

	/*fopen_s(&FP, "CSsortH.cso", "rb");
	length = fread(tempbuffer, 1, maxShaderSize, FP);
	fclose(FP);
	data = tempbuffer;

	hr = m_pd3dDevice->CreateComputeShader(data, length, NULL, &m_pCSSortShader[1]);*/

	delete[] tempbuffer;
	
	m_dx11Res.m_pImmediateContext->VSSetShader(m_dx11Res.m_pVertexShader, NULL, 0);
	
	m_dx11Res.m_pImmediateContext->CSSetShader(m_dx11Res.m_pCSSimShader, NULL, 0);
	
	
	m_dx11Res.m_pImmediateContext->PSSetSamplers(0, 1, &m_dx11Res.m_pSamplerState);
	
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = 16;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	
	
	m_dx11Res.m_pd3dDevice->CreateBuffer(&bd, NULL, &m_dx11Res.m_pConstantBuffer);
	m_dx11Res.m_pImmediateContext->UpdateSubresource( m_dx11Res.m_pConstantBuffer, 0, NULL, &m_settings.height, 0, 0 );
	
	bd.ByteWidth = sizeof(SimInput);
	m_dx11Res.m_pd3dDevice->CreateBuffer(&bd, NULL, &m_dx11Res.m_pSimInput);
	bd.ByteWidth = 32;
	m_dx11Res.m_pd3dDevice->CreateBuffer(&bd, NULL, &m_dx11Res.m_pSimInfoCB);
	bd.ByteWidth = 32;/// sizeof(SortDetailsCB);
	m_dx11Res.m_pd3dDevice->CreateBuffer(&bd, NULL, &m_dx11Res.m_pSortInfoCB);
   bd.ByteWidth = 128;//  sizeof(ExplosionDelayedData);/// sizeof(ExplosionDelayedData);
   m_dx11Res.m_pd3dDevice->CreateBuffer(&bd, NULL, &m_dx11Res.m_pExplosiveCB);
   

	
	D3D11_TEXTURE2D_DESC descT;
	ZeroMemory( &descT, sizeof(descT) );
	descT.Width = PARTNUM;
	descT.Height = PARTNUM; 
	descT.MipLevels = 1;
	descT.ArraySize = 1;
	descT.Format = activeFormat;
	descT.SampleDesc.Count = 1;
	descT.SampleDesc.Quality = 0;
	descT.Usage = D3D11_USAGE_DEFAULT;
	descT.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	descT.CPUAccessFlags = 0;
	descT.MiscFlags = 0;
	
	hr = m_dx11Res.m_pd3dDevice->CreateTexture2D( &descT, NULL, &m_dx11Res.m_pTextureDataBuffer[0] );
	hr = m_dx11Res.m_pd3dDevice->CreateTexture2D( &descT, NULL, &m_dx11Res.m_pTextureDataBuffer[1] );

   descT.Usage = D3D11_USAGE_STAGING;
   descT.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
   descT.BindFlags = NULL;
   hr = m_dx11Res.m_pd3dDevice->CreateTexture2D(&descT, NULL, &m_dx11Res.m_pTextureDataExport);

	m_dx11Res.m_pd3dDevice->CreateShaderResourceView(m_dx11Res.m_pTextureDataBuffer[0], NULL, &m_dx11Res.m_pTextureDataView[0]);
	m_dx11Res.m_pd3dDevice->CreateShaderResourceView(m_dx11Res.m_pTextureDataBuffer[1], NULL, &m_dx11Res.m_pTextureDataView[1]);

	m_dx11Res.m_pd3dDevice->CreateRenderTargetView(m_dx11Res.m_pTextureDataBuffer[0], NULL, &m_dx11Res.m_pDataRenderTargetView[0]);
	m_dx11Res.m_pd3dDevice->CreateRenderTargetView(m_dx11Res.m_pTextureDataBuffer[1], NULL, &m_dx11Res.m_pDataRenderTargetView[1]);

	m_dx11Res.m_pd3dDevice->CreateUnorderedAccessView(m_dx11Res.m_pTextureDataBuffer[0], NULL, &m_dx11Res.m_pTextureDataUAView[0]);
	m_dx11Res.m_pd3dDevice->CreateUnorderedAccessView(m_dx11Res.m_pTextureDataBuffer[1], NULL, &m_dx11Res.m_pTextureDataUAView[1]);
	
	m_oWidth = m_settings.width;
	m_oHeight = m_settings.height;
	m_scale = 1.f;

	return 0;
}
//
//int DxApp::SetupDx12Resources()
//{
//
//   // Load Shaders:
//   LPVOID data;
//   SIZE_T length;
//#ifdef NDEBUG
//   const int maxShaderSize = 131072;
//#else
//   const int maxShaderSize = 131072 * 8;
//#endif
//   char* tempbuffer = new char[maxShaderSize]; //max shader size....
//
//   FILE *FP;
//   fopen_s(&FP, "VertexShader.cso", "rb");
//   length = fread(tempbuffer, 1, maxShaderSize, FP);
//   fclose(FP);
//   data = tempbuffer;
//
//
//   hr = m_dx11Res.m_pd3dDevice->CreateVertexShader(data, length, NULL, &m_dx11Res.m_pVertexShader);
//
//   D3D11_INPUT_ELEMENT_DESC layouttex[] =
//   {
//      { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
//   };
//
//   ThrowIfFailed(m_dx12Res.m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
//
//   hr = m_dx11Res.m_pd3dDevice->CreateInputLayout(layouttex, 2, data,
//      length, &m_dx11Res.m_pVertexInput);
//
//   fopen_s(&FP, "PixelShader.cso", "rb");
//   length = fread(tempbuffer, 1, maxShaderSize, FP);
//   fclose(FP);
//   data = tempbuffer;
//
//   hr = m_dx11Res.m_pd3dDevice->CreatePixelShader(data, length, NULL, &m_dx11Res.m_pPixelShader);
//   return 0;
//}

void DxApp::OnKeyUp(SHORT vkey)
{
	if((m_settings.inputType & 1) == 1)
	{
		if (vkey == 'R')
		{
			SimReset();
		}
		if (vkey == 'H')
		{
			SimZeroVelocity();
		}
		if (vkey == VK_SPACE)
		{
			SimStart();			//	Reserved for start game			SimZeroVelocity(); 
		}
		if (vkey == 'Z')
		{
			SimImplode();
		}
		if (vkey == VK_ESCAPE)
		{
			PostMessage(m_hWnd, WM_DESTROY,NULL,NULL);
		}
	}
}

//int inputType; //1=local, 2=file, 4=internet, 8=record, 16=benchmark, 32 connect. Combine local file and record to add multiple local controllers.
//int timeMode; //used with local inputType, 0 for realtime, or set number of samples per second.
bool DxApp::GetInput(SimInput &input)
{
	if((m_settings.inputType & 3) == 1 && m_settings.timeMode != 0)
	{
		input.timeP = 1.f/m_settings.timeMode;
	}
	else if((m_settings.inputType & 2) == 2)
	{
		fread(&input.timeP, 4, 1, m_settings.inFile);
		if(feof(m_settings.inFile)!=0)
		{
			m_settings.inputType -= 2;
			if((m_settings.inputType & 16) == 16)
			{
				DisplayBenchmarkComplete();
				m_timer.Check();
				input.timeP=0;
			}
		}
		else
		{
			m_counterExpectedTime+=input.timeP;
		}
	}
	else
	{
		input.timeP = m_timer.Check();
	}


	if((m_settings.inputType & 2) == 2)
	{
		//readinputs
		fread(&input.numControl, 4, 1, m_settings.inFile);
		fseek(m_settings.inFile, 8, SEEK_CUR);
		fread(&input.controlInput[0], sizeof(SimControl), input.numControl, m_settings.inFile);
	}

	if((m_settings.inputType & 1) == 1 && ((m_mouse1 || m_mouse2) || (m_simReset || m_simHalt)) || m_simImplode || m_simStart)
	{
		input.controlInput[input.numControl].mousePosX = m_mousePositionX / m_scale;
		input.controlInput[input.numControl].mousePosY = m_mousePositionY / m_scale;
		input.controlInput[input.numControl].inputLow = m_mouse1<< InputMessages_pull; //old pull not used
		//input.controlInput[input.numControl].inputLow += m_mouse2<<1;
		input.controlInput[input.numControl].inputLow += m_simReset<< InputMessages_reset;
		input.controlInput[input.numControl].inputLow += m_simHalt<< InputMessages_halt;
		input.controlInput[input.numControl].inputLow += m_simStart << InputMessages_start;
		
		
		if (m_localBombCounter < BOMB_PLACEMENT_LIMIT && m_mouse1 && input.controlInput[input.numControl].mousePosX > BOMB_LOCATION_LIMIT_X && m_live==0)
		{
			m_localBombCounter++;
			input.controlInput[input.numControl].inputLow += m_mouse1 << InputMessages_explode;//m_simImplode << 5;
			input.controlInput[input.numControl].inputHigh = m_localBombCounter * 200.f;
		}

		m_simReset=0;
		m_simHalt=0;
		m_simStart = 0;
		m_simImplode = 0;
		//m_settings.
      m_mouse1 = false;

		input.numControl++;
	}

	if((m_settings.inputType & 36) != 0 )
	{
		if(m_Comms.GetInput(input)==0)
		{
			MessageBox(m_hWnd, L"Disconnected", L"Error", NULL);
			m_close=true;
			return false;
			//m_settings.inputType = !(m_settings.inputType & 36);
		}
	}

	if((m_settings.inputType & 8) == 8)
	{
		fwrite(&input, 16, input.numControl+1, m_settings.outFile);
	}
	return true;
}

void DxApp::DisplayBenchmarkComplete()
{
	float totalTime = m_performanceD.Check();
	wchar_t result[256];
	swprintf_s(result, 256, L"Recreation took %fseconds. %.2fpts!", totalTime, m_counterExpectedTime/totalTime*100.f);
	MessageBox(nullptr, result, L"Complete", NULL);
	m_settings.inputType-=16;
				
	SetWindowLong(m_hWnd, -16, m_settings.winStyle);
	RECT rc = { 0, 0, m_settings.width, m_settings.height };
	AdjustWindowRect( &rc, m_settings.winStyle, FALSE );
	int posX = (GetSystemMetrics(SM_CXSCREEN) - (rc.right-rc.left))  / 2;
	int posY = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom-rc.top)) / 2;
				
	SetWindowPos(
		m_hWnd,
		NULL,
		posX,
		posY,
		rc.right-rc.left, rc.bottom-rc.top,
		SWP_FRAMECHANGED
	);
}

void DxApp::RecordUpdate(wchar_t *filename)
{
	FILE *file;
	int	result = _wfopen_s(&file, filename, L"rb");
	if(!result)
	{
		int version;
		fread(&version, 4, 1, file);
		if(version==COMMS_CONNECT_VER)//this version is used because the sockets use the same data structures
		{
			fseek(file, 16, SEEK_SET);
			m_settings.inFile = file;
			return;
		}
		//go through list of all previous versions here if applicable

		//it wasn't a previous version, the first version didn't have a version number, assume it's that and convert
		fseek(file, 0, SEEK_END);
		int fileLength = ftell(file);
		int *fileData = new int[fileLength+4];
		fseek(file, 0, SEEK_SET);
		fread(((char*)fileData)+16, fileLength, 1, file);
		fileData[0] = COMMS_CONNECT_VER;
		fclose(file);
		int position = 5;
		for(; position<fileLength+4; position+=4)
		{
			int inputs = fileData[position];
			while(inputs!=0)
			{
				inputs--;
				position+=4;
				if(fileData[position]==1)
				{
					fileData[position-1]+=2;
					fileData[position]=0;
				}
			}
		}
		_wfopen_s(&file, filename, L"wb");
		fwrite(fileData, fileLength+4, 1, file);
		fclose(file);
		_wfopen_s(&m_settings.inFile, filename, L"rb");
	}
}