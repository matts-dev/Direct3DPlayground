//see http://www.directxtutorial.com/Lesson.aspx?lessonid=11-4-5
//see http://www.neatware.com/lbstudio/web/hlsl.html
//see https://www.gamedev.net/forums/topic/695654-cant-compile-basic-shader/  for error checking
//see https://stackoverflow.com/questions/15543571/allocconsole-not-displaying-cout for opening console
//see https://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing for examples


//required by windows desktop applications; it is a pre-compiled header(PCH) to speed up compilation time (the user can add headers to this file).
#include<stdafx.h>

#include <d3dcommon.h> //blob type for shaders
#include <d3dcompiler.h>

//add libraries for linking to directx
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3dcompiler.lib")

#include<d3d11.h>	//base d3d 11 features
//#include <d3dx11.h> //this has been deprecated and removed from directx; many tutorials still reference it

//#include <dxgitype.h>
//#include <dxgi.h>


//contains all the win32 structures and functions we will need for creating and managing a window. 
//stdafx.h actually includes this file but leaving here because it is important to know about. (and intellisense needs it)
#include<windows.h>

#include<iostream>		 //printing
#include<cstdio>		 //console redirection
#include <stdint.h>


//reminder: anonymous namespaces effectively make everything within them static to the current .cpp (eg we can have 100s of TrueWinMain functions in different files).
//my goal is to have each cpp file be its own demo, and you simply uncomment out the main method to test the demo.
//the "TrueWinMain" method within the namespace need not be commented out when another .cpp is created.
//only the small 3-line main method at the bottom needs to be commented out. 
namespace
{
	HWND ghMainWnd = 0;				//global handle to the main window of this application; applications can have multiple windows
	int gWidth = 500;
	int gHeight = 500;

	/** WINDOWS: A window instance's callback to handle events; called internally after message loop calls DispatchMessage*/
	LRESULT CALLBACK WndProc_MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
			case WM_KEYDOWN:
				if (wParam == VK_ESCAPE) { DestroyWindow(hWnd); }
				return 0;
			case WM_LBUTTONDOWN:
				MessageBox(hWnd, L"Left Mouse Button Down", L"Popup-Title", MB_OK);
				return 0;
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}


	/** Windows version of main(); this is "True" main because the 3-line main at bottom of file diorectly calls this
	 * see comment at namespace
	 */
	int WINAPI TrueWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd)
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//INIT WINDOW 
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		const wchar_t* WindowClassName = L"MyWidowClass";

		WNDCLASS wc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	//these class styles (CS) specify window should be re-painted when size changes (horizontal or vertical). Some tutorials recommend only using CS_OWNDC
		wc.lpfnWndProc = &WndProc_MessageHandler;		//assign a function pointer to our callback
		wc.cbClsExtra = 0;								//extra memory slot; we're not going to use
		wc.cbWndExtra = 0;								//extra memory slow; we're not going to use
		wc.hInstance = hInstance;						//gives the class a pointer to the main application object
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);		//creates a default icon
		wc.hCursor = LoadCursor(0, IDC_ARROW);			//creates the default cursor
		wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));	//makes the background white
		wc.lpszMenuName = 0;							//we don't have a menu pop up
		wc.lpszClassName = WindowClassName;				//a string identifier to find this wc struct at creation

		if (!RegisterClass(&wc))
		{
			MessageBox(0, L"Failed to register window class", 0, 0);		//MessageBox is a small window-popup displaying a message; it is unrelated to event messages.
			return 0;
		}

		//Note: this may fail if you have not yet written a WinProc function; be sure to call default window proc (DefWindowProc) to handle leftover messages; wm_create needs to be handled for window creation.
		ghMainWnd = CreateWindow(
			WindowClassName,				//name used to find registered window class
			L"DirectX Window",				//window name
			WS_OVERLAPPEDWINDOW,			//a combination of bitflags to give basic window behavior (minimize, maximize, sysmenu, etc.)
			CW_USEDEFAULT, CW_USEDEFAULT,	//the x,y position of top-left corner relative to the screen; positive y runs downward
			gWidth, gHeight,				//the width/height of window in pixels
			0, 0,							//parent and menu windows; should be null for our purposes
			hInstance,						//handle to the application instance
			0								//extra info available to the WM_Create event message
		);

		if (!ghMainWnd)
		{
			MessageBox(0, L"Failed to create window", 0, 0); //display error popup window
			return 0;
		}

		ShowWindow(ghMainWnd, nShowCmd);		//display window on the screen.
		UpdateWindow(ghMainWnd);				//manually refresh the window since we just finished initializing it.

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// BEGIN DIRECTX
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////
		// Open Console for viewing logging
		////////////////////////////////////////////////////////
		AllocConsole();
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
		freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
		std::cout << " redirected stdout test" << std::endl;
		std::cerr << " redirected stderr test" << std::endl;

		RECT windowRect;
		GetClientRect(ghMainWnd, &windowRect);
		uint32_t clientWidth = static_cast<uint32_t>(windowRect.right - windowRect.left);
		uint32_t clientHeight = static_cast<uint32_t>(windowRect.bottom - windowRect.top);

		DXGI_MODE_DESC backBufferDesc = {}; //this value initialization will zero out memory.
		backBufferDesc.Width = clientWidth;
		backBufferDesc.Height = clientHeight;
		backBufferDesc.RefreshRate.Numerator = 60;
		backBufferDesc.RefreshRate.Denominator = 1;
		backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		backBufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;		//this is 0 anyways... so probably unnecessary
		backBufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc = backBufferDesc;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = ghMainWnd;
		swapChainDesc.Windowed = true;
		///* DXGI_SWAP_EFFECT_DISCARD can be used with multisampling*/
		//swapChainDesc.BufferCount = 1; //use 1 with DXGI_SWAP_EFFECT_DISCARD
		//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //legacy

		/* DXGI_SWAP_EFFECT_FLIP_DISCARD cannot be used with multisampling*/
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //new but requires tweaks

		IDXGISwapChain* pSwapChain;
		ID3D11Device* pDevice;
		ID3D11DeviceContext* pDeviceContext;
		HRESULT createSwapChainResult =
			D3D11CreateDeviceAndSwapChain(
				nullptr,					//IDXGIAdapter* pAdapter,
				D3D_DRIVER_TYPE_HARDWARE,	//D3D_DRIVER_TYPE DriverType,
				nullptr,					//HMODULE Software,
				D3D11_CREATE_DEVICE_DEBUG,	//UINT Flags,			PASS 0u for non-debug output
				nullptr,					//CONST D3D_FEATURE_LEVEL* pFeatureLevels,
				0u,							//UINT FeatureLevels,
				D3D11_SDK_VERSION,			//UINT SDKVersion,
				&swapChainDesc,				//DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
				&pSwapChain,				//IDXGISwapChain** ppSwapChain,
				&pDevice,					//ID3D11Device** ppDevice,
				nullptr,					//D3D_FEATURE_LEVEL* pFeatureLevel,
				&pDeviceContext				//ID3D11DeviceContext** ppImmediateContext
			);
		if (FAILED(createSwapChainResult))
		{
			std::cerr << "failed to create swap chain" << std::endl;
		}

		////////////////////////////////////////////////////////
		// Render target view
		////////////////////////////////////////////////////////
		ID3D11Texture2D* pBackBuffer;
		HRESULT hrGetBB = pSwapChain->GetBuffer(
			0u,										//UINT Buffer,
			__uuidof(ID3D11Texture2D),				//REFIID riid,
			reinterpret_cast<void**>(&pBackBuffer)	//void **ppSurface
		);
		if (FAILED(hrGetBB))
		{
			std::cerr << "failed to get the backbuffer" << std::endl;
		}

		ID3D11RenderTargetView* pRenderTargetView;
		HRESULT hrMakeRTView = pDevice->CreateRenderTargetView(
			pBackBuffer,		//ID3D11Resource *pResource,
			nullptr,			//const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
			&pRenderTargetView	//ID3D11RenderTargetView **ppRTView) = 0;
		);
		pBackBuffer->Release();
		if (FAILED(hrMakeRTView))
		{
			std::cerr << "failed to create the render target view" << std::endl;
		}
		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

		/////////////////////////////////////////////////////////////////////////////////////
		// Create Shaders 
		/////////////////////////////////////////////////////////////////////////////////////

		const char* const vertShaderSrc = R"(
			//struct name can be whatever you want, many name it based on data source and datadestination (eg AppToVertData for application to vertex data)
			struct vertexOutput 
			{
				float4 pos : SV_POSITION;
				float4 color : COLOR;
			};

			// SV_POSITION and COLOR are output semantics.

			//this is the entry point of the shader, it can be named what ever you like -- you just need to specify it at compilation
			vertexOutput main_vertex(
				float3 position: POSITION,
				float3 color : COLOR)
			{
				vertexOutput vsout;
				vsout.pos = float4(position, 1.0);
				vsout.color = float4(color, 1.0f);	

				return vsout;
			}
		)";

		const char* const pixelShaderSrc = R"(
			// you don't necessarily have to define a return struct, instead you can add the output semantic directly to the 
			// return of the shader entry_point. In this case the output semantic follows the functName(param, param).
			// eg returnType functionName(Param a, Param b) : outputSemantic
			float4 main_pixel(
					float4 position : SV_Position,
					float4 color : COLOR
				) 
				: SV_TARGET
			{
				return color;
			}
		)";


		ID3DBlob* errorMsgs;

		////////////////////////////////////////////////////////
		// compile vertex shader
		////////////////////////////////////////////////////////
		ID3DBlob* pVertShader_blob;
		ID3D11VertexShader* pVertShader;
		HRESULT vertCompileResult =
			D3DCompile(
				vertShaderSrc,	//LPCVOID pSrcData,
				strlen(vertShaderSrc),//SIZE_T SrcDataSize,
				NULL,			//LPCSTR pSourceName,
				NULL,			//CONST D3D_SHADER_MACRO* pDefines,
				NULL,			//ID3DInclude* pInclude,
				"main_vertex",	//LPCSTR pEntrypoint,
				"vs_5_0",		//LPCSTR pTarget,
				D3DCOMPILE_DEBUG,//UINT Flags1,			//pass null if you do not want debug info
				NULL,			//UINT Flags2,
				&pVertShader_blob,	//ID3DBlob** ppCode,
				&errorMsgs		//ID3DBlob** ppErrorMsgs
			);

		if (FAILED(vertCompileResult)) //don't check S_OK directly, use the FAILED() macro!
		{
			std::cerr << "failed to compile vert shader." << std::endl;
			if (errorMsgs)
			{
				const char* errorMessage = static_cast<const char*>(errorMsgs->GetBufferPointer());
				std::cerr << errorMessage << std::endl;
			}
		}
		//errorMsgs may contain warnings, we need to release it even if we didn't fail to compile shader
		if (errorMsgs) { errorMsgs->Release(); }

		HRESULT createVS = pDevice->CreateVertexShader(
			pVertShader_blob->GetBufferPointer(),	//const void *pShaderBytecode,
			pVertShader_blob->GetBufferSize(),		//SIZE_T BytecodeLength,
			nullptr,			//ID3D11ClassLinkage *pClassLinkage,
			&pVertShader		//ID3D11VertexShader **ppVertexShader
		);
		if (FAILED(createVS))
		{
			std::cerr << "failed to create the vertex shader from blob bytecode" << std::endl;
		}

		////////////////////////////////////////////////////////
		// compile pixel shader
		////////////////////////////////////////////////////////
		ID3DBlob* pPixelShader_blob;
		ID3D11PixelShader* pPixelShader;
		HRESULT pixelShaderCompileResult = D3DCompile(
			/* pSrcData */ pixelShaderSrc,
			/* dataSize*/ strlen(pixelShaderSrc),
			/* pSrcName*/ NULL,
			/* pDefines*/ NULL,
			/* pInclude*/ NULL,
			/* pEntryPoint*/ "main_pixel",
			/* pTarget */ "ps_5_0",
			/* flags1*/ /*NULL*/ D3DCOMPILE_DEBUG,
			/* flags2*/ NULL,
			/* ppCode*/ &pPixelShader_blob,
			/* ppErrorMsgs*/ &errorMsgs
		);
		if (FAILED(pixelShaderCompileResult))
		{
			std::cerr << "failed to compile the pixel shader." << std::endl;
			if (errorMsgs)
			{
				const char* errorMessage = static_cast<const char*>(errorMsgs->GetBufferPointer());
				std::cerr << errorMessage << std::endl;
			}
		}
		if (errorMsgs)(errorMsgs->Release());
		HRESULT createPS = pDevice->CreatePixelShader(
			pPixelShader_blob->GetBufferPointer(),	//const void *pShaderBytecode,
			pPixelShader_blob->GetBufferSize(),		//SIZE_T BytecodeLength,
			nullptr,			//ID3D11ClassLinkage *pClassLinkage,
			&pPixelShader		//ID3D11VertexShader **ppVertexShader
		);
		if (FAILED(createPS))
		{
			std::cerr << "failed to create the pixel shader from blob bytecode" << std::endl;
		}

		pDeviceContext->VSSetShader(pVertShader, nullptr, 0);
		pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);

		struct MyVertex
		{
			float x, y, z;
			float r, g, b;
		};
		MyVertex vertices[] =
		{
			{0.5f, 0.5f, 0.f,		0.f, 1.f, 0.f},
			{0.f, -0.5f, 0.f,		1.f, 0.f, 0.f},
			{-0.5f, 0.5f, 0.f,		0.f, 0.f, 1.f}
		};

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = sizeof(vertices);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexData = {};
		vertexData.pSysMem = vertices;

		ID3D11Buffer* pVertexBuffer;
		HRESULT hrMkVbuffer = pDevice->CreateBuffer(
			&vertexBufferDesc,	//const D3D10_BUFFER_DESC *pDesc,
			&vertexData,		//const D3D10_SUBRESOURCE_DATA *pInitialData,
			&pVertexBuffer		//ID3D10Buffer **ppBuffer
		);
		if (FAILED(hrMkVbuffer))
		{
			std::cerr << "failed to create the device's vertex buffer" << std::endl;
		}

		UINT offset = 0;
		UINT stride = sizeof(MyVertex);
		pDeviceContext->IASetVertexBuffers(
			0u,				//UINT StartSlot,
			1u,				//UINT NumBuffers,
			&pVertexBuffer, //ID3D11Buffer *const *ppVertexBuffers,
			&stride,		//const UINT *pStrides,
			&offset			//const UINT *pOffsets) = 0;
		);

		D3D11_INPUT_ELEMENT_DESC inputLayout[] =
		{
			//LPCSTR SemanticName;
			//			UINT SemanticIndex;
			//				DXGI_FORMAT Format;
			//											UINT InputSlot;
			//												UINT AlignedByteOffset;
			//																D3D11_INPUT_CLASSIFICATION InputSlotClass;
			//																							UINT InstanceDataStepRate;
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,	0,				 D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",	 0,	DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		ID3D11InputLayout* pVertLayout;
		HRESULT hrMkInputLayout = pDevice->CreateInputLayout(
			inputLayout,							//const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs,
			2,										//UINT NumElements,	//--this is like number of vertex attributes in opengl --//
			pVertShader_blob->GetBufferPointer(),			//const void *pShaderBytecodeWithInputSignature,
			pVertShader_blob->GetBufferSize(),			//SIZE_T BytecodeLength,
			&pVertLayout							//ID3D11InputLayout **ppInputLayout
		);
		if (FAILED(hrMkInputLayout))
		{
			std::cerr << "failed to create input layout" << std::endl;
		}

		pDeviceContext->IASetInputLayout(pVertLayout);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP); //good for debugging to see where your vertices are

		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(clientWidth);
		viewport.Height = static_cast<float>(clientHeight);
		pDeviceContext->RSSetViewports(1, &viewport);


		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// BEGIN MESSAGE LOOP (ie the game loop)
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// -------------------- BEGIN MESSAGE LOOP -----------------------------
		MSG msg = { 0 };		//create a blank message struct use as an out variable; initialize all its fields to zero.
		while (msg.message != WM_QUIT)
		{
			//handle all messages first before game loop
			if (PeekMessage(&msg, ghMainWnd, 0, 0, PM_REMOVE))	//PM_REMOVE removes the message from queue
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				//no message, do gameloop stuff here
				float clearColor[] = { 0, 0, 0, 1 };
				pDeviceContext->ClearRenderTargetView(pRenderTargetView, clearColor);

				pDeviceContext->VSSetShader(pVertShader, nullptr, 0);
				pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
				pDeviceContext->IASetInputLayout(pVertLayout);
				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				pDeviceContext->IASetVertexBuffers(0u,1u,&pVertexBuffer,&stride,&offset); //need to define local structs externally
				pDeviceContext->RSSetViewports(1, &viewport);

				pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

				pDeviceContext->Draw(3, 0);

				pSwapChain->Present(0, 0);
			}
		}

		////////////////////////////////////////////////////////
		// free direct x resources
		////////////////////////////////////////////////////////

		//release console
		FreeConsole();
		pVertShader_blob->Release();
		pVertShader->Release();

		pPixelShader_blob->Release();
		pPixelShader->Release();

		pVertexBuffer->Release();
		pVertLayout->Release();
		pRenderTargetView->Release();

		pSwapChain->Release();
		pDeviceContext->Release();
		pDevice->Release();

		return msg.wParam;	//windows expects us to return th wParam of the WM_QUIT message
	}
}


//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd)
//{
//	return TrueWinMain(hInstance, hPrevInstance, pCmdLine, nShowCmd);
//}

