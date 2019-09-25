//see http://www.directxtutorial.com/Lesson.aspx?lessonid=11-4-5
//see http://www.neatware.com/lbstudio/web/hlsl.html
//see https://www.gamedev.net/forums/topic/695654-cant-compile-basic-shader/  for error checking
//see https://stackoverflow.com/questions/15543571/allocconsole-not-displaying-cout for opening console
//see https://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing for examples


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

		pDeviceContext->OMSetRenderTargets(
			1,						//UINT NumViews,
			&pRenderTargetView,		//ID3D11RenderTargetView *const *ppRenderTargetViews,
			nullptr					//ID3D11DepthStencilView *pDepthStencilView
		);

		////////////////////////////////////////////////////////
		// Create shaders
		////////////////////////////////////////////////////////
		const char* const vertShaderHLSL = R"(
			
			struct vertexOutput 
			{
				float4 position : SV_POSITION;		//sv = system value, SV_POSITION is a built in semantic
				float4 color : MY_COLOR;			//MY_COLOR is a custom semantic name
			};

			vertexOutput main_vertex(
					float3 position : MY_POSITION,		//notice this is a float3, whereas the output is a float4
					float3 color : MY_COLOR			//making parameters float3s is just a convenience for us later when defining vertices
				)
			{
				vertexOutput vs_out;
				vs_out.position = float4(position, 1);
				vs_out.color = float4(color, 1);
				return vs_out;
			}
		)";

		const char* const pixelShaderHSLS = R"(
			
			float4 main_pixel(
					float4 position:SV_POSITION,	//we can just specify the semantics, we don't need to redefine the output struct
					float4 color:MY_COLOR			//
				) 
				: SV_TARGET	// SV_TARGET is a semantic; return value semantics follow for the function signature.
			{
				return color;
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
			float r, g, b;
		};

		//   0
		//  2 1
		// 5 4 3
		MyVertex vertices[] =
		{
			//x		y		z				r		g		b
			{0.f,	0.5f,	 0.f,			1.f,	875.f,	0.f},	//0
			{0.25f,	0.0f,	 0.f,			1.f,	875.f,	0.f},	//1
			{-0.25f,0.0f,	 0.f,			1.f,	875.f,	0.f},	//2

			{0.5f,	-0.5f,	 0.f,			1.f,	875.f,	0.f},	//3
			{0.0f,	-0.5f,	 0.f,			1.f,	875.f,	0.f},	//4
			{-0.5f,	-0.5f,	 0.f,			1.f,	875.f,	0.f}	//5
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
		// Create index buffer
		////////////////////////////////////////////////////////
		uint32_t triangleIndices[] = {
			0, 1, 2,
			1, 3, 4,
			2, 4, 5,
		};
		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(triangleIndices);	//this will be sizeof(uint32_t) * 9; since this is an actual array and not pointer to an array
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0u;
		indexBufferDesc.MiscFlags = 0u;

		D3D11_SUBRESOURCE_DATA indexData = {};
		indexData.pSysMem = triangleIndices;

		//create a buffer like we did for the index buffer!
		ID3D11Buffer* pIndexBuffer = nullptr;
		hr(pDevice->CreateBuffer(
			&indexBufferDesc,	//const D3D11_BUFFER_DESC *pDesc,
			&indexData,			//const D3D11_SUBRESOURCE_DATA *pInitialData,
			&pIndexBuffer		//ID3D11Buffer **ppBuffer
		));

		pDeviceContext->IASetIndexBuffer(
			pIndexBuffer,			//ID3D11Buffer *pIndexBuffer,
			DXGI_FORMAT_R32_UINT,	//DXGI_FORMAT Format,
			0u						//UINT Offset
		);

		////////////////////////////////////////////////////////
		// Configuring the vertex shader input
		////////////////////////////////////////////////////////
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			//LPCSTR SemanticName;
			//				UINT SemanticIndex;
			//					DXGI_FORMAT Format;
			//												UINT InputSlot;
			//			   									   UINT AlignedByteOffset;
			//																D3D11_INPUT_CLASSIFICATION InputSlotClass;
			//																							UINT InstanceDataStepRate;
			{"MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,			D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"MY_COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		ID3D11InputLayout* pInputLayout = nullptr;
		hr(pDevice->CreateInputLayout(
			inputElementDesc,	//const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs,
			2,					//UINT NumElements,
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
		pDeviceContext->RSSetViewports(1, &viewport);

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
				pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, /*depth_stencil*/ nullptr);

				//In real applications we will be changing the objects currently bound to pipeline; let's rebind everything we need here to show what rendering a specified vertex buffer would look like
				//we did this while creating, check out the first time we called these functions for the arguments
				//pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &vertexStride, &vertexOffset);
				//pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0u);
				//pDeviceContext->IASetInputLayout(pInputLayout);
				//pDeviceContext->IASetPrimitiveTopology(/*D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/ D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
				//pDeviceContext->VSSetShader(pVertShader, nullptr, 0);
				//pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
				//pDeviceContext->RSSetViewports(1, &viewport);

				pDeviceContext->DrawIndexed(9, 0, 0);

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
		if (pIndexBuffer) pIndexBuffer->Release();
		if (pInputLayout) pInputLayout->Release();

		return 0;
	}

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd)
{
	return TrueWinMain(hInstance, hPrevInstance, pCmdLine, nShowCmd);
}