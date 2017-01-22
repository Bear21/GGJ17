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



void DxApp::Render()
{
	/*array<int, 3>& gidd = *m_pData;
	array<graphics::float_4, 2>& locs = *m_pDataPartLoc;
	array<unsigned int, 2>& gids = *m_pCountGrid;*/
	//graphics::texture<graphics::float_2, 3>& gidd = *m_pDataGrid;
	
	
	float resw=m_oWidth, resh=m_oHeight;
	float tscale = m_scale;
	int iresw=m_settings.width;
	//float timeP = m_timer.Check();
#ifdef DEBUG_OUTPUT_PROFILE
	wchar_t report[64];
#endif
	SimInput input;
	input.numControl = 0;
    m_frameCounter++;

	if(!GetInput(input))
		return;
	float timeP = input.timeP;
	//simulate
	//m_pTextureBufferUAView

	ID3D11DepthStencilView *dsNullview = NULL;
	ID3D11ShaderResourceView *srvNull = NULL;
	ID3D11RenderTargetView *rtvNull = NULL;
	ID3D11UnorderedAccessView *uavNull = NULL;
	ID3D11ShaderResourceView *srvSim[] = {m_dx11Res.m_pTextureDataView[m_flip], NULL, NULL};
	m_dx11Res.m_pImmediateContext->RSSetViewports( 1, &m_dx11Res.m_simViewPort );
	m_dx11Res.m_pImmediateContext->UpdateSubresource( m_dx11Res.m_pSimInput, 0, NULL, &input, 0, 0 );


	if (m_first) // Initalisation
	{
		m_first = 0;
		m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &m_dx11Res.m_pDataRenderTargetView[!m_flip], dsNullview);
		m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvSim);
		m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pStartupResetShader, NULL, 0);
		m_dx11Res.m_pImmediateContext->Draw(4, 0);
		m_flip = !m_flip;
		srvSim[0] = m_dx11Res.m_pTextureDataView[m_flip];
		m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
		m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &rtvNull, dsNullview);
	}

   if (m_live) 
   { // Sandpit
      m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &m_dx11Res.m_pDataRenderTargetView[!m_flip], dsNullview);
      m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvSim);
      m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pSandpit, NULL, 0);
      m_dx11Res.m_pImmediateContext->Draw(4, 0);
      m_flip = !m_flip;
      srvSim[0] = m_dx11Res.m_pTextureDataView[m_flip];
      m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
      m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &rtvNull, dsNullview);
   }

	for(int i=0; i<input.numControl; i++) // modifiers
	{
		if((input.controlInput[i].inputLow & 1<<3) == 1<<3) // Reset
		{
			m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &m_dx11Res.m_pDataRenderTargetView[!m_flip], dsNullview);
			m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvSim);
			m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pResetShader, NULL, 0);
			m_dx11Res.m_pImmediateContext->Draw(4, 0);
			m_flip = !m_flip;
			srvSim[0] = m_dx11Res.m_pTextureDataView[m_flip];
			m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
			m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &rtvNull, dsNullview);
			
			break;//no point in running any more
		}
		if((input.controlInput[i].inputLow & 1<<4) == 1<<4) // Halt (sets velocity to 0)
		{
			m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &m_dx11Res.m_pDataRenderTargetView[!m_flip], dsNullview);
			m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvSim);
			m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pHaltShader, NULL, 0);
			m_dx11Res.m_pImmediateContext->Draw(4, 0);
			m_flip = !m_flip;
			srvSim[0] = m_dx11Res.m_pTextureDataView[m_flip];
			m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
			m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &rtvNull, dsNullview);
		}
      if ((input.controlInput[i].inputLow & 1 << 5) == 1 << 5) // Implode - Beginning of the explosion.
      {
         ExplosionDelayedData foo = ExplosionDelayedData(input.controlInput[i].mousePosX, input.controlInput[i].mousePosY, 0);
         
         m_dx11Res.m_pImmediateContext->UpdateSubresource(m_dx11Res.m_pExplosiveCB, 0, NULL, &foo, 0, 0);


         m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &m_dx11Res.m_pDataRenderTargetView[!m_flip], dsNullview);
         m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvSim);
         m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pExplodeShader, NULL, 0);
         m_dx11Res.m_pImmediateContext->Draw(4, 0);
         m_flip = !m_flip;
         srvSim[0] = m_dx11Res.m_pTextureDataView[m_flip];
         m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
         m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &rtvNull, dsNullview);
         //m_explosionDataList.push_front(ExplosionDelayedData(input.controlInput[i].mousePosX, input.controlInput[i].mousePosY, 0/* input.timeP * (m_frameCounter + 2850)*/));
         m_explosionDataQueue.push(ExplosionDelayedData(input.controlInput[i].mousePosX, input.controlInput[i].mousePosY, 0));/* input.timeP * (m_frameCounter + 2850)*/

      }
	}

	while ((m_explosionDataQueue.size() > 0) && m_explosionDataQueue.front().timePDeadline < (input.timeP * m_frameCounter))
	{

		m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &m_dx11Res.m_pDataRenderTargetView[!m_flip], dsNullview);
		m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvSim);
		m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pExplodeShader, NULL, 0);
		m_dx11Res.m_pImmediateContext->Draw(4, 0);
		m_flip = !m_flip;
		srvSim[0] = m_dx11Res.m_pTextureDataView[m_flip];
		m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
		m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &rtvNull, dsNullview);
    
		m_explosionDataQueue.pop();
	}
	
	if(m_settings.renderMode==0)
	{
		m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvSim);
		m_dx11Res.m_pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_dx11Res.m_pDataRenderTargetView[!m_flip], dsNullview, 1, 1, &m_dx11Res.m_pTextureBufferUAView, NULL);
		m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pSimShader, NULL, 0);
		m_dx11Res.m_pImmediateContext->Draw(4, 0);

		m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
		m_dx11Res.m_pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &rtvNull, dsNullview, 1, 1, &uavNull, NULL);
	}
	else
	{		
		m_dx11Res.m_pImmediateContext->CSSetShaderResources(0, 1, srvSim);
		ID3D11UnorderedAccessView *UAVList[] = {m_dx11Res.m_pTextureDataUAView[!m_flip], m_dx11Res.m_pTextureBufferUAView};
		m_dx11Res.m_pImmediateContext->CSSetUnorderedAccessViews(0, 2, UAVList, NULL);
		m_dx11Res.m_pImmediateContext->Dispatch(PARTNUM/16, PARTNUM/16, 1);

		UAVList[0] = NULL;
		UAVList[1] = NULL;
		m_dx11Res.m_pImmediateContext->CSSetUnorderedAccessViews(0, 2, UAVList, NULL);
		m_dx11Res.m_pImmediateContext->CSSetShaderResources(0, 1, &srvNull);
	}

	
	m_dx11Res.m_pImmediateContext->OMSetRenderTargets(1, &m_dx11Res.m_pRenderFinalTargetView, dsNullview);
	ID3D11ShaderResourceView *resources[] = {m_dx11Res.m_pTextureView, NULL, NULL};
	m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 3, resources);
	m_dx11Res.m_pImmediateContext->PSSetShader(m_dx11Res.m_pPixelShader, NULL, 0);
	m_dx11Res.m_pImmediateContext->RSSetViewports( 1, &m_dx11Res.m_finalViewPort );
	m_dx11Res.m_pImmediateContext->Draw(4, 0);

	m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, &srvNull);
	

	unsigned int initTemp[4] = {0,0,0,0};
	m_dx11Res.m_pImmediateContext->ClearUnorderedAccessViewUint(m_dx11Res.m_pTextureBufferUAView, initTemp);
	m_flip = !m_flip;

	if(m_settings.timeMode!=0 && (m_settings.inputType & 36)!=0)
	{
		float gap;
		//temp code for stable network play
		while((gap = m_holdTime.Peek())<1.f/m_settings.timeMode)
		{
			if((1.f/m_settings.timeMode-gap)>0.002f)
				Sleep(1);
		}
	}
	m_pSwapChain->Present( 0, 0 );
	if(m_settings.profile)
	{
#ifdef DEBUG_OUTPUT_PROFILE
		swprintf(report, L" PRE, %fmsec", m_performanceS.Check()*1000.f);
		OutputDebugString(report);
#endif
	}
	//m_pImmediateContext->PSSetShaderResources(0, 1, srvNull);
}

int DxApp::SetupSizeDependentResources()
{
   return m_settings.dx12 ? SetupSizeDependentResources12() : SetupSizeDependentResources11();
}

int DxApp::SetupSizeDependentResources11()
{
	m_dx11Res.m_simViewPort.Width = (FLOAT)PARTNUM;
	m_dx11Res.m_simViewPort.Height = (FLOAT)PARTNUM; 
	m_dx11Res.m_simViewPort.MinDepth = 0.0f;
   m_dx11Res.m_simViewPort.MaxDepth = 1.0f;
   m_dx11Res.m_simViewPort.TopLeftX = 0;
   m_dx11Res.m_simViewPort.TopLeftY = 0;

	m_dx11Res.m_finalViewPort.Width = (FLOAT)m_settings.width;
	m_dx11Res.m_finalViewPort.Height = (FLOAT)m_settings.height; 

   m_dx11Res.m_finalViewPort.MinDepth = 0.0f;
   m_dx11Res.m_finalViewPort.MaxDepth = 1.0f;
   m_dx11Res.m_finalViewPort.TopLeftX = 0;
   m_dx11Res.m_finalViewPort.TopLeftY = 0;

	m_dx11Res.m_pImmediateContext->RSSetViewports( 1, &m_dx11Res.m_simViewPort );


	D3D11_BUFFER_DESC descT;
	ZeroMemory( &descT, sizeof(descT) );
	descT.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	descT.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	descT.ByteWidth = m_settings.width*m_settings.height*4;
	descT.Usage = D3D11_USAGE_DEFAULT;
	descT.StructureByteStride = 0;
	m_dx11Res.m_pd3dDevice->CreateBuffer(&descT, NULL, &m_dx11Res.m_pTextureBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.FirstElement = 0;
	UAVDesc.Buffer.NumElements = m_settings.width*m_settings.height;
	UAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
	m_dx11Res.m_pd3dDevice->CreateUnorderedAccessView(m_dx11Res.m_pTextureBuffer, &UAVDesc, &m_dx11Res.m_pTextureBufferUAView);

	D3D11_SHADER_RESOURCE_VIEW_DESC descSR;
	ZeroMemory( &descSR, sizeof(descSR) );
	descSR.Format = DXGI_FORMAT_R32_TYPELESS;
	descSR.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	descSR.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
	descSR.BufferEx.NumElements = m_settings.width*m_settings.height;
	m_dx11Res.m_pd3dDevice->CreateShaderResourceView(m_dx11Res.m_pTextureBuffer, &descSR, &m_dx11Res.m_pTextureView);

	unsigned int initTemp[4] = {0,0,0,0};
	m_dx11Res.m_pImmediateContext->ClearUnorderedAccessViewUint(m_dx11Res.m_pTextureBufferUAView, initTemp);

	SimDetailsCB CB = {0, 0, (float)m_oWidth, (float)m_oHeight, m_scale, m_settings.width, m_settings.height};
	m_dx11Res.m_pImmediateContext->UpdateSubresource( m_dx11Res.m_pSimInfoCB, 0, NULL, &CB, 0, 0 );

	ID3D11Buffer *CBList[] = {m_dx11Res.m_pConstantBuffer, m_dx11Res.m_pSimInfoCB, m_dx11Res.m_pSimInput, m_dx11Res.m_pExplosiveCB };
	m_dx11Res.m_pImmediateContext->PSSetConstantBuffers(0, 4, CBList);
	m_dx11Res.m_pImmediateContext->CSSetConstantBuffers(0, 4, CBList);

	return 0;
}

int DxApp::SetupSizeDependentResources12()
{
   return S_FALSE;
}

void DxApp::ResizeSizeDependentResources(int width, int height)
{
   m_settings.dx12 ? ResizeSizeDependentResources12(width, height) : ResizeSizeDependentResources11(width, height);
}

void DxApp::ResizeSizeDependentResources11(int width, int height)
{
	if(width!=0)
	{
		if((m_settings.inputType&16)!=16)
		{
			m_settings.width = width;
			m_settings.height = height;

			m_scale = (float)m_settings.width/(float)m_oWidth;
			if((float)m_settings.height/(float)m_oHeight< m_scale)
			{
				m_scale = (float)m_settings.height/(float)m_oHeight;
			}
			if(m_dx11Res.m_pImmediateContext)
			{
				ID3D11ShaderResourceView *srvNull[] = {NULL, NULL, NULL};
				m_dx11Res.m_pImmediateContext->PSSetShaderResources(0, 1, srvNull);

			
				m_dx11Res.m_finalViewPort.Width = (FLOAT)m_settings.width;
				m_dx11Res.m_finalViewPort.Height = (FLOAT)m_settings.height; 
				m_dx11Res.m_finalViewPort.MinDepth = 0.0f;
				m_dx11Res.m_finalViewPort.MaxDepth = 1.0f;
				m_dx11Res.m_finalViewPort.TopLeftX = 0;
				m_dx11Res.m_finalViewPort.TopLeftY = 0;

				m_dx11Res.m_pImmediateContext->RSSetViewports( 0, NULL );

				// Create a render target view
				ID3D11DepthStencilView *dsNullview = NULL;
				ID3D11RenderTargetView *rtNullview = NULL;
				m_dx11Res.m_pImmediateContext->OMSetRenderTargets( 1, &rtNullview, dsNullview );
				m_dx11Res.m_pRenderFinalTargetView->Release();

				m_dx11Res.m_pTextureView->Release();
				m_dx11Res.m_pTextureBufferUAView->Release();
				m_dx11Res.m_pTextureBuffer->Release();

				m_pSwapChain->ResizeBuffers(2, m_settings.width, m_settings.height, DXGI_FORMAT_UNKNOWN, 0);

				DXGI_SWAP_CHAIN_DESC swdesc;
				m_pSwapChain->GetDesc(&swdesc);
				if(swdesc.BufferDesc.Width!=m_settings.width)
					OutputDebugString(L"welp");

				ID3D11Texture2D* pBackBuffer = NULL;
				m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );

				m_dx11Res.m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_dx11Res.m_pRenderFinalTargetView );
				pBackBuffer->Release();

				m_dx11Res.m_pImmediateContext->OMSetRenderTargets( 1, &m_dx11Res.m_pRenderFinalTargetView, dsNullview );

				m_dx11Res.m_pImmediateContext->RSSetState(m_dx11Res.m_pRasterizeNoDepth);

				m_dx11Res.m_pImmediateContext->VSSetShader(m_dx11Res.m_pVertexShader, NULL, 0);

				D3D11_BUFFER_DESC descT;
				ZeroMemory( &descT, sizeof(descT) );
				descT.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
				descT.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
				descT.ByteWidth = m_settings.width*m_settings.height*4;
				descT.Usage = D3D11_USAGE_DEFAULT;
				descT.StructureByteStride = 0;
				m_dx11Res.m_pd3dDevice->CreateBuffer(&descT, NULL, &m_dx11Res.m_pTextureBuffer);

				D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
				UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				UAVDesc.Buffer.FirstElement = 0;
				UAVDesc.Buffer.NumElements = m_settings.width*m_settings.height;
				UAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
				m_dx11Res.m_pd3dDevice->CreateUnorderedAccessView(m_dx11Res.m_pTextureBuffer, &UAVDesc, &m_dx11Res.m_pTextureBufferUAView);

				D3D11_SHADER_RESOURCE_VIEW_DESC descSR;
				ZeroMemory( &descSR, sizeof(descSR) );
				descSR.Format = DXGI_FORMAT_R32_TYPELESS;
				descSR.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
				descSR.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
				descSR.BufferEx.NumElements = m_settings.width*m_settings.height;
				m_dx11Res.m_pd3dDevice->CreateShaderResourceView(m_dx11Res.m_pTextureBuffer, &descSR, &m_dx11Res.m_pTextureView);

				unsigned int initTemp[4] = {0,0,0,0};
				m_dx11Res.m_pImmediateContext->ClearUnorderedAccessViewUint(m_dx11Res.m_pTextureBufferUAView, initTemp);

				SimDetailsCB CB = {0, 0, (float)m_oWidth, (float)m_oHeight, m_scale, m_settings.width, m_settings.height};
				m_dx11Res.m_pImmediateContext->UpdateSubresource( m_dx11Res.m_pSimInfoCB, 0, NULL, &CB, 0, 0 );

				ID3D11Buffer *CBList[] = { m_dx11Res.m_pConstantBuffer, m_dx11Res.m_pSimInfoCB, m_dx11Res.m_pSimInput, m_dx11Res.m_pExplosiveCB };
				m_dx11Res.m_pImmediateContext->PSSetConstantBuffers(0, 4, CBList);
				m_dx11Res.m_pImmediateContext->CSSetConstantBuffers(0, 4, CBList);
			}
		}
		else
		{
		
		}
	}

}

void DxApp::ResizeSizeDependentResources12(int width, int height)
{
}

void DxApp::SimReset()
{
	m_simReset=1;
}
void DxApp::SimZeroVelocity()
{
	m_simHalt=1;
}

void DxApp::SimImplode()
{
   m_simImplode = 1;
}