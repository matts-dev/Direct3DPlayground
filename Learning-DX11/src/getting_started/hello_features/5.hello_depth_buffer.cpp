// see https://www.braynzarsoft.net/viewtutorial/q16390-8-world-view-and-local-spaces-static-camera
// see https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules for ocnstant buffer packing

//window includes
#include"stdafx.h"	//windows PCH

//#directx includes
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")
#include<d3dcommon.h>	//blob shader type
#include<d3dcompiler.h>
#include<d3d11.h>

#include<windows.h>

//c++ includes
#include<iostream>
#include<cstdio>
#include<cstdint>
#include<ctime>
#include <chrono>


namespace	//anonymous namespace makes everything within it essentially static.
{

	HRESULT hr_result; //global hr so macro doesn't require scoping; which may have overhead.
#define hr(func)\
	hr_result = func;\
	if(FAILED(hr_result))\
	{\
		std::cout << "D3D error" << __FUNCTION__ << " " << __LINE__ << #func << std::endl;\
		__debugbreak();\
	}


	ID3DBlob* pErrorBlob = nullptr;	//a shared error blob
#define hr_error(func)\
	if(pErrorBlob) pErrorBlob->Release();\
	hr_result = func;\
	if(pErrorBlob)\
	{\
		/*Print any warnings even if we didn't fail the hresult*/\
		const char* errorMessage = static_cast<const char*>(pErrorBlob->GetBufferPointer());\
		std::cerr << errorMessage << std::endl;\
		pErrorBlob->Release();\
	}\
	/*don't break until after we've printed message*/\
	if(FAILED(hr_result))\
	{\
		std::cout << "D3D error" << __FUNCTION__ << " " << __LINE__ << #func << std::endl;\
		__debugbreak();\
	}\



	//window instance's callback for events
	LRESULT CALLBACK WndProc_MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
			case WM_KEYDOWN:
				if (wParam == VK_ESCAPE) { DestroyWindow(hWnd); }
				return 0;
			case WM_LBUTTONDOWN:
				MessageBox(hWnd, L"Left Mouse Button DOwn", L"Popup-Title", MB_OK);
				return 0;
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
		}

		//if not handled, let the default window proc handle this message.
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	int WINAPI TrueWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd)
	{
		////////////////////////////////////////////////////////
		// WindowsOS window creation
		////////////////////////////////////////////////////////
		const wchar_t* const WindowClassName = L"MyWindowClass";
		WNDCLASS windowClass = {};
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;		//these class styles (CS) specify window should be re-painted when size changes (horizontal or vertical). Some tutorials recommend only using CS_OWNDC
		windowClass.lpfnWndProc = &WndProc_MessageHandler;			//assign a function pointer to our callback
		windowClass.cbClsExtra = 0;									//extra memory slot; we're not going to use
		windowClass.cbWndExtra = 0;									//extra memory slow; we're not going to use
		windowClass.hInstance = hInstance;							//gives the class a pointer to the main application object
		windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);			//creates a default icon
		windowClass.hCursor = LoadCursor(0, IDC_ARROW);				//creates the default cursor
		windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)); //makes the background white
		windowClass.lpszMenuName = 0;								//we don't have a menu pop up
		windowClass.lpszClassName = WindowClassName;					//a string identifier to find this wc struct at creation

		if (!RegisterClass(&windowClass))
		{
			MessageBox(0, L"Failed to register window class", 0, 0);
			return 0;
		}

		int windowHeight = 500;
		int windowWidth = 500;
		HWND hMainWindow = CreateWindow(
			WindowClassName,					//name used to find registered window class
			L"DirectX Window",					//window name
			WS_OVERLAPPEDWINDOW, 				//a combination of bitflags to give basic window behavior (minimize, maximize, sysmenu, etc.)
			CW_USEDEFAULT, CW_USEDEFAULT,		//the x,y position of top-left corner relative to the screen; positive y runs downward
			windowWidth, windowHeight,			//the width/height of window in pixels
			0, 0,								//parent and menu windows; should be null for our purposes
			hInstance,							//handle to the application instance
			0									//extra info available to the WM_Create event message
		);

		if (!hMainWindow)
		{
			MessageBox(0, L"Failed to create window", 0, 0);
			return 0;
		}

		ShowWindow(hMainWindow, nShowCmd);
		UpdateWindow(hMainWindow);

		////////////////////////////////////////////////////////
		//create a console for visual logging
		////////////////////////////////////////////////////////
		AllocConsole();
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
		freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
		std::cout << " redirected stdout test" << std::endl;
		std::cerr << " redirected stderr test" << std::endl;

		/////////////////////////////////////////////////////////////////////////////////////
		// Direct3D 11
		/////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////
		// d3d11: Create the device, device context, and swap chain
		////////////////////////////////////////////////////////
		RECT windowRect;
		GetClientRect(hMainWindow, &windowRect);
		uint32_t clientWidth = static_cast<uint32_t>(windowRect.right - windowRect.left);
		uint32_t clientHeight = static_cast<uint32_t>(windowRect.bottom - windowRect.top);

		DXGI_MODE_DESC backbufferDesc = {}; //value init struct with zeros;
		backbufferDesc.Width = clientWidth;
		backbufferDesc.Height = clientHeight;
		backbufferDesc.RefreshRate.Numerator = 60;
		backbufferDesc.RefreshRate.Denominator = 1;
		backbufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		backbufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		backbufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc = backbufferDesc;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.OutputWindow = hMainWindow;
		swapChainDesc.Windowed = true;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; //must be 2 if using new DXGI_SWAP_EFFECT_FLIP_DISCARD	//new, but has multisampling restrictions
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;								//new
		//swapChainDesc.BufferCount = 1; //use 1 with DXGI_SWAP_EFFECT_DISCARD	//legacy
		//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;					//legacy

		IDXGISwapChain* pSwapChain = nullptr;
		ID3D11Device* pDevice = nullptr;
		ID3D11DeviceContext* pDeviceContext = nullptr;
		hr(D3D11CreateDeviceAndSwapChain(
			nullptr,					//IDXGIAdapter* pAdapter,
			D3D_DRIVER_TYPE_HARDWARE,	//D3D_DRIVER_TYPE DriverType,
			nullptr,					//HMODULE Software,
			D3D11_CREATE_DEVICE_DEBUG,	//UINT Flags, may want to or in "| D3D11_CREATE_DEVICE_DEBUGGABLE"; documenation implies it allows shader debugging
			nullptr,					//CONST D3D_FEATURE_LEVEL* pFeatureLevels,
			0u,							//UINT FeatureLevels,
			D3D11_SDK_VERSION,			//UINT SDKVersion,
			&swapChainDesc,				//DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
			&pSwapChain,				//IDXGISwapChain** ppSwapChain,
			&pDevice,					//ID3D11Device** ppDevice,
			nullptr,					//D3D_FEATURE_LEVEL* pFeatureLevel,
			&pDeviceContext				//ID3D11DeviceContext** ppImmediateContext);
		));

		////////////////////////////////////////////////////////
		// d3d11: Depth and Stencil buffer
		////////////////////////////////////////////////////////
		D3D11_TEXTURE2D_DESC depth_stencil_texture_desc;
		depth_stencil_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depth_stencil_texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
		depth_stencil_texture_desc.Width = clientWidth;
		depth_stencil_texture_desc.Height = clientHeight;
		depth_stencil_texture_desc.ArraySize = 1u;				//number of textures in this array
		depth_stencil_texture_desc.MipLevels = 1u;				//single mip
		depth_stencil_texture_desc.SampleDesc.Count = 1;		//anti aliasing
		depth_stencil_texture_desc.SampleDesc.Quality = 0;
		depth_stencil_texture_desc.CPUAccessFlags = 0u;
		depth_stencil_texture_desc.MiscFlags = 0u;

		ID3D11Texture2D* pDepthStencilBufferTexture = nullptr;
		hr(pDevice->CreateTexture2D(
			&depth_stencil_texture_desc,	//const D3D11_TEXTURE2D_DESC *pDesc,
			nullptr,						//const D3D11_SUBRESOURCE_DATA *pInitialData,
			&pDepthStencilBufferTexture		//ID3D11Texture2D **ppTexture2D
		));
		
		ID3D11DepthStencilView* pDepthStencilView = nullptr;
		hr(pDevice->CreateDepthStencilView(
			pDepthStencilBufferTexture,		//ID3D11Resource *pResource,
			nullptr,						//const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
			&pDepthStencilView				//ID3D11DepthStencilView **ppDepthStencilView
		));

		////////////////////////////////////////////////////////
		// d3d11: Create the render target
		////////////////////////////////////////////////////////
		ID3D11Texture2D* pBackBufferTexture = nullptr;
		hr(pSwapChain->GetBuffer(
			0,													//UINT Buffer,
			__uuidof(ID3D11Texture2D),							//REFIID riid,
			reinterpret_cast<void**>(&pBackBufferTexture)		//void **ppSurface
		));

		ID3D11RenderTargetView* pRenderTargetView = nullptr;
		hr(pDevice->CreateRenderTargetView(
			pBackBufferTexture,		// ID3D11Resource *pResource,
			nullptr,				// const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
			&pRenderTargetView		// ID3D11RenderTargetView **ppRTView)
		));

		if (pBackBufferTexture) { pBackBufferTexture->Release(); } //no longer needed

//DEPTH/STENCIL
//#NOTICE depth and stencil are set here!
		pDeviceContext->OMSetRenderTargets(
			1,						//UINT NumViews,
			&pRenderTargetView,		//ID3D11RenderTargetView *const *ppRenderTargetViews,
			pDepthStencilView		//ID3D11DepthStencilView *pDepthStencilView
		);

		////////////////////////////////////////////////////////
		// Create shaders
		////////////////////////////////////////////////////////
		const char* const vertShaderHLSL = R"(
			
			struct vertexOutput 
			{
				float4 position : SV_POSITION;		//sv = system value, SV_POSITION is a built in semantic
			};

			cbuffer ConstantBuffer_PerObject
			{
				float3 offset;	
				float time;		
				float3 color;	
			};

			vertexOutput main_vertex(
					float3 position : MY_POSITION		//notice this is a float3, whereas the output is a float4
				)
			{
				vertexOutput vs_out;
				vs_out.position = float4(position, 1) + float4(offset, 0); 

				return vs_out;
			}
		)";

		const char* const pixelShaderHSLS = R"(
			
			cbuffer ConstantBuffer_PerObject
			{
				float3 offset;	
				float time;			
				float3 color;
			};

			float4 main_pixel(
					float4 position:SV_POSITION	//we can just specify the semantics, we don't need to redefine the output struct
				) 
				: SV_TARGET	// SV_TARGET is a semantic; return value semantics follow for the function signature.
			{
				float baseIntensity = 0.05f;

				float4 outColor = float4(color, 1.0f);

				return outColor;
			}
		)";

		////////////////////////////////////////////////////////
		//	compile and set vertex shader
		////////////////////////////////////////////////////////
		ID3DBlob* pVertShader_ByteCodeBlob = nullptr;
		hr_error(D3DCompile(
			vertShaderHLSL,				//LPCVOID pSrcData,
			strlen(vertShaderHLSL),		//SIZE_T SrcDataSize,
			nullptr,					//LPCSTR pSourceName,
			nullptr,					//CONST D3D_SHADER_MACRO* pDefines,
			nullptr,					//ID3DInclude* pInclude,
			"main_vertex",				//LPCSTR pEntrypoint,
			"vs_5_0",					//LPCSTR pTarget,
			D3DCOMPILE_DEBUG,			//UINT Flags1,		//pass 0 if you don't want debugging information
			0u,							//UINT Flags2,
			&pVertShader_ByteCodeBlob,	//ID3DBlob** ppCode,
			&pErrorBlob					//ID3DBlob** ppErrorMsgs
		));

		ID3D11VertexShader* pVertShader = nullptr;
		hr(pDevice->CreateVertexShader(
			pVertShader_ByteCodeBlob->GetBufferPointer(),	//const void *pShaderBytecode,
			pVertShader_ByteCodeBlob->GetBufferSize(),		//SIZE_T BytecodeLength,
			nullptr,		//ID3D11ClassLinkage *pClassLinkage,
			&pVertShader	//ID3D11VertexShader **ppVertexShader) = 0;
		));
		pDeviceContext->VSSetShader(
			pVertShader,	//ID3D11VertexShader *pVertexShader,
			nullptr,		//ID3D11ClassInstance *const *ppClassInstances,
			0u				//UINT NumClassInstances
		);

		////////////////////////////////////////////////////////
		// compile and set pixel shader
		////////////////////////////////////////////////////////
		ID3DBlob* pPixelShader_ByteCodeBlob = nullptr;
		hr_error(D3DCompile(
			pixelShaderHSLS,			//LPCVOID pSrcData,
			strlen(pixelShaderHSLS),	//SIZE_T SrcDataSize,
			nullptr,					//LPCSTR pSourceName,
			nullptr,					//CONST D3D_SHADER_MACRO* pDefines,
			nullptr,					//ID3DInclude* pInclude,
			"main_pixel",				//LPCSTR pEntrypoint,
			"ps_5_0",					//LPCSTR pTarget,
			D3DCOMPILE_DEBUG,			//UINT Flags1,		//pass 0 if you don't want debugging information
			0u,							//UINT Flags2,
			&pPixelShader_ByteCodeBlob,	//ID3DBlob** ppCode,
			&pErrorBlob					//ID3DBlob** ppErrorMsgs
		));

		ID3D11PixelShader* pPixelShader;
		hr(pDevice->CreatePixelShader(
			pPixelShader_ByteCodeBlob->GetBufferPointer(),	//const void *pShaderBytecode,
			pPixelShader_ByteCodeBlob->GetBufferSize(),		//SIZE_T BytecodeLength,
			nullptr,		//ID3D11ClassLinkage *pClassLinkage,
			&pPixelShader	//ID3D11PixelShader **ppPixelShader) = 0;
		));

		pDeviceContext->PSSetShader(
			pPixelShader,	//ID3D11PixelShader *pPixelShader,
			nullptr,		//ID3D11ClassInstance *const *ppClassInstances,
			0u				//UINT NumClassInstances
		);

		////////////////////////////////////////////////////////
		// Creating a vertex buffer
		////////////////////////////////////////////////////////
		struct MyVertex
		{
			float x, y, z;
		};

		MyVertex vertices[] =
		{
			//x		y		z				
			{0.f,	0.25f,	 0.f,},
			{0.25f,	-0.25f,	 0.f,},
			{-0.25f,-0.25f,	 0.f,}
		};

		D3D11_BUFFER_DESC vertexBuffer_desc = {}; //zero initialize the entire structure via c++value init
		vertexBuffer_desc.ByteWidth = sizeof(vertices);		//since this is not a pointer type (it's MyVertex[3]), we can take the true size
		vertexBuffer_desc.Usage = D3D11_USAGE_DEFAULT;				//specifies how buffer is read/written to
		vertexBuffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;		//specifies how buffer is bound to pipeline
		vertexBuffer_desc.MiscFlags = 0u;							//extra information, 
		vertexBuffer_desc.CPUAccessFlags = 0u;						//zero specifies CPU doesn't need to access this buffer

		D3D11_SUBRESOURCE_DATA vertexDataCPU = {}; //zero out via value init
		vertexDataCPU.pSysMem = vertices;

		ID3D11Buffer* pVertexBuffer = nullptr;
		hr(pDevice->CreateBuffer(
			&vertexBuffer_desc,	//const D3D11_BUFFER_DESC *pDesc,
			&vertexDataCPU,		//const D3D11_SUBRESOURCE_DATA *pInitialData,
			&pVertexBuffer		//ID3D11Buffer **ppBuffer
		));

		//set vertex buffer
		UINT vertexOffset = 0u;
		UINT vertexStride = sizeof(MyVertex);// 0u; #check
		pDeviceContext->IASetVertexBuffers(
			0u,				//UINT StartSlot,		which input (eg vertex buffer) slot of 16 total slots (or 32 with some feature levels) to bind to
			1u,				//UINT NumBuffers,		you can bind multiple input slots at once, but we're only binding 1
			&pVertexBuffer,	//ID3D11Buffer *const *ppVertexBuffers,		//pointer to pointer because this may or may not be an array of vertex buffers
			&vertexStride,	//const UINT *pStrides,						//size of elements to be used from buffer;	pointer because this needs to accept an array if multiple bound
			&vertexOffset	//const UINT *pOffsets						//offset between first element and starting pointer want; pointer because this needs to accept an array if multiple bound
		);


		////////////////////////////////////////////////////////
		// Configuring the vertex shader input
		////////////////////////////////////////////////////////
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{
				"MY_POSITION",					//LPCSTR SemanticName;
				0,								//UINT SemanticIndex;
				DXGI_FORMAT_R32G32B32_FLOAT,	//DXGI_FORMAT Format;
				0,								// UINT InputSlot;
				0,								// UINT AlignedByteOffset;
				D3D11_INPUT_PER_VERTEX_DATA,	//D3D11_INPUT_CLASSIFICATION InputSlotClass;
				0								//UINT InstanceDataStepRate;
			}
		};
		ID3D11InputLayout* pInputLayout = nullptr;
		hr(pDevice->CreateInputLayout(
			inputElementDesc,	//const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs,
			1,					//UINT NumElements,
			pVertShader_ByteCodeBlob->GetBufferPointer(),	//const void *pShaderBytecodeWithInputSignature,
			pVertShader_ByteCodeBlob->GetBufferSize(),		//SIZE_T BytecodeLength,
			&pInputLayout									//ID3D11InputLayout **ppInputLayout
		));

		pDeviceContext->IASetInputLayout(pInputLayout);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		////////////////////////////////////////////////////////
		// setting viewport
		////////////////////////////////////////////////////////
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(clientWidth);
		viewport.Height = static_cast<float>(clientHeight);
//DEPTH/STENCIL
//#NOTICE we define our depth in the viewport too^^
		viewport.MaxDepth = 1.0;
		viewport.MinDepth = 0.0;
		pDeviceContext->RSSetViewports(1, &viewport);


		////////////////////////////////////////////////////////
		// Constant buffers
		////////////////////////////////////////////////////////

		auto startTimePoint = std::chrono::high_resolution_clock::now();

		struct MyVec3
		{
			float x, y, z; //12 bytes (3x4bytes)
		};

		struct ObjectConstantBuffer
		{
			float offset[3];	//+12bytes
			float time;			//+4bytes
			MyVec3 color;			//+12bytes	
			float padding;		//+4bytes
		};
		ObjectConstantBuffer cb_PerObject = {};
		cb_PerObject.time = 1.0f;

		ID3D11Buffer* pConstantBuffer;

		D3D11_BUFFER_DESC cbPerObjectDesc = {};
		cbPerObjectDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbPerObjectDesc.ByteWidth = sizeof(ObjectConstantBuffer);
		cbPerObjectDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //<-this gives the CPU permission to write to this buffer
		cbPerObjectDesc.Usage = D3D11_USAGE_DYNAMIC; //<-this signals to d3d that we're going to be updating this buffer frequently
		cbPerObjectDesc.MiscFlags = 0u;

		D3D11_SUBRESOURCE_DATA cbtime_subdata = {};
		cbtime_subdata.pSysMem = &cb_PerObject;

		hr(pDevice->CreateBuffer(
			&cbPerObjectDesc,					//const D3D11_BUFFER_DESC *pDesc,
			&cbtime_subdata,					//const D3D11_SUBRESOURCE_DATA *pInitialData,
			&pConstantBuffer					//ID3D11Buffer **ppBuffer) = 0;
		));
		pDeviceContext->PSSetConstantBuffers(/*StartSlot*/ 0, /*NumBuffers*/ 1, &pConstantBuffer);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Message Queue and Game Loop
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		MSG msg = {}; //zero init message field
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, hMainWindow, /*filterMin*/0, /*filterMax*/0, /*removeMsg*/PM_REMOVE))
			{
				//there is a new message, handle it.
				TranslateMessage(&msg);
				DispatchMessage(&msg);		//ultimately ends in sending message to the WinProc function we created
			}
			else
			{
				//no message, do gameloop stuff
				float clearColor[] = { 0,0,0,1 };
				pDeviceContext->ClearRenderTargetView(pRenderTargetView, clearColor);

//DEPTH/STENCIL
//#NOTICE you have to clear the depth and stencil separately from the color target.
				pDeviceContext->ClearDepthStencilView(
					pDepthStencilView,							//ID3D11DepthStencilView *pDepthStencilView,
					D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,	//UINT ClearFlags,
					1.0f,										//FLOAT Depth,
					0u											//UINT8 Stencil
				);


//DEPTH/STENCIL
//#NOTICE be sure to apply the depth/stencil view to the pipeline render target!
				//pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, /*depth_stencil*/ nullptr); //uncomment not bind depth buffer, depth will be ignored
				pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);			//uncomment to view depth buffer in action!

				////////////////////////////////////////////////////////
				// Update Constant Buffers
				////////////////////////////////////////////////////////

				auto nowTimePoint = std::chrono::high_resolution_clock::now();
				float runtimeSecs = std::chrono::duration<float>(nowTimePoint - startTimePoint).count();
				cb_PerObject.time = runtimeSecs;

				pDeviceContext->PSSetConstantBuffers(0u/*StartSlot*/, 1 /*numBuffers*/, &pConstantBuffer);
				pDeviceContext->VSSetConstantBuffers(0u/*StartSlot*/, 1 /*numBuffers*/, &pConstantBuffer);

				D3D11_MAPPED_SUBRESOURCE mappedPixelCB = {};

				//closest depth
				cb_PerObject.offset[0] = 0.25f;
				cb_PerObject.offset[1] = 0.25f;
				cb_PerObject.offset[2] = 0.1f; //<------------------------------depth
				cb_PerObject.color.x = 1.0;
				cb_PerObject.color.y = 0.0f;
				cb_PerObject.color.z = 0.0f;
				pDeviceContext->Map(pConstantBuffer, 0u/*Subresource*/, D3D11_MAP_WRITE_DISCARD, 0u, &mappedPixelCB);
				std::memcpy(mappedPixelCB.pData, &cb_PerObject, sizeof(ObjectConstantBuffer)); //copy to pData from our struct
				pDeviceContext->Unmap(pConstantBuffer, 0u/*Subresource*/);
				pDeviceContext->Draw(3, 0);

				//furthest depth
				cb_PerObject.offset[0] = -0.25f;
				cb_PerObject.offset[1] = -0.25f;
				cb_PerObject.offset[2] = 0.9f; //<------------------------------depth
				cb_PerObject.color.x = 0.0;
				cb_PerObject.color.y = 0.0f;
				cb_PerObject.color.z = 1.0f;
				pDeviceContext->Map(pConstantBuffer, 0u/*Subresource*/, D3D11_MAP_WRITE_DISCARD, 0u, &mappedPixelCB);
				std::memcpy(mappedPixelCB.pData, &cb_PerObject, sizeof(ObjectConstantBuffer)); //copy to pData from our struct
				pDeviceContext->Unmap(pConstantBuffer, 0u/*Subresource*/);
				pDeviceContext->Draw(3, 0);

				//middle depth
				cb_PerObject.offset[0] = 0.f;
				cb_PerObject.offset[1] = 0.0f;
				cb_PerObject.offset[2] = 0.5f; //<------------------------------depth
				cb_PerObject.color.x = 0.0;
				cb_PerObject.color.y = 1.0f;
				cb_PerObject.color.z = 0.0f;
				pDeviceContext->Map(pConstantBuffer, 0u/*Subresource*/, D3D11_MAP_WRITE_DISCARD, 0u, &mappedPixelCB);
				std::memcpy(mappedPixelCB.pData, &cb_PerObject, sizeof(ObjectConstantBuffer)); //copy to pData from our struct
				pDeviceContext->Unmap(pConstantBuffer, 0u/*Subresource*/);
				pDeviceContext->Draw(3, 0);


				pSwapChain->Present(0, 0);
			}
		}


		////////////////////////////////////////////////////////
		// Release resources
		////////////////////////////////////////////////////////
		//release console for logging.
		FreeConsole();
		if (pSwapChain) pSwapChain->Release();
		if (pDevice) pDevice->Release();
		if (pDeviceContext) pDeviceContext->Release();
		if (pRenderTargetView) pRenderTargetView->Release();
		if (pVertShader_ByteCodeBlob) pVertShader_ByteCodeBlob->Release();
		if (pVertShader) pVertShader->Release();
		if (pPixelShader_ByteCodeBlob) pPixelShader_ByteCodeBlob->Release();
		if (pPixelShader) pPixelShader->Release();
		if (pVertexBuffer) pVertexBuffer->Release();
		if (pInputLayout) pInputLayout->Release();
		if (pConstantBuffer) pConstantBuffer->Release();
		if (pDepthStencilBufferTexture) pDepthStencilBufferTexture->Release();
		if (pDepthStencilView) pDepthStencilView->Release();

		return 0;
	}

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd)
{
	return TrueWinMain(hInstance, hPrevInstance, pCmdLine, nShowCmd);
}