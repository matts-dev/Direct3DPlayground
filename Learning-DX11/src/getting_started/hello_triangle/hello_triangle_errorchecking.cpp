//see http://www.directxtutorial.com/Lesson.aspx?lessonid=11-4-5
//see http://www.neatware.com/lbstudio/web/hlsl.html
//see https://www.gamedev.net/forums/topic/695654-cant-compile-basic-shader/  for error checking
//see https://stackoverflow.com/questions/15543571/allocconsole-not-displaying-cout for opening console
//see https://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing for examples


//window includes
#include"stdafx.h"	//windows PCH
#include<windows.h>

//#directx includes
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include<d3dcommon.h>	//blob shader type
#include<d3dcompiler.h>
#include<d3d11.h>

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
		const wchar_t* const WindowClassName = L"MyWindowClass";
		WNDCLASS wc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;		//these class styles (CS) specify window should be re-painted when size changes (horizontal or vertical). Some tutorials recommend only using CS_OWNDC
		wc.lpfnWndProc = &WndProc_MessageHandler;			//assign a function pointer to our callback
		wc.cbClsExtra = 0;									//extra memory slot; we're not going to use
		wc.cbWndExtra = 0;									//extra memory slow; we're not going to use
		wc.hInstance = hInstance;							//gives the class a pointer to the main application object
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);			//creates a default icon
		wc.hCursor = LoadCursor(0, IDC_ARROW);				//creates the default cursor
		wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)); //makes the background white
		wc.lpszMenuName = 0;								//we don't have a menu pop up
		wc.lpszClassName = WindowClassName;					//a string identifier to find this wc struct at creation

		if (!RegisterClass(&wc))
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

		//test macro
		ID3D11Device* pDevice = nullptr;
		hr(pDevice->CreateRenderTargetView(0,0,0));
		
		return 0;
	}

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd)
{
	return TrueWinMain(hInstance, hPrevInstance, pCmdLine, nShowCmd);
}