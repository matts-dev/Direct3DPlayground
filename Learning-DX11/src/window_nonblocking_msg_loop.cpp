
//required by windows desktop applications; it is a pre-compiled header(PCH) to speed up compilation time (the user can add headers to this file).
#include"stdafx.h"

//contains all the win32 structures and functions we will need for creating and managing a window. 
//stdafx.h actually includes this file but leaving here because it is important to know about. (and intellisense needs it)
#include<windows.h>

#include<iostream>

//reminder: anonymous namespaces effectively make everything within them static to the current .cpp (eg we can have 100s of TrueWinMain functions in different files).
//my goal is to have each cpp file be its own demo, and you simply uncomment out the main method to test the demo.
//the "TrueWinMain" method within the namespace need not be commented out when another .cpp is created.
//only the small 3-line main method at the bottom needs to be commented out. 
namespace
{
	HWND ghMainWnd = 0;				//global handle to the main window of this application; applications can have multiple windows

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
		wc.style = CS_HREDRAW | CS_VREDRAW;		//these class styles (CS) specify window should be re-painted when size changes (horrizontal or vertical).
		wc.lpfnWndProc = &WndProc_MessageHandler;	//assign a function pointer to our callback
		wc.cbClsExtra = 0;							//extra memory slot; we're not going to use
		wc.cbWndExtra = 0;							//extra memory slow; we're not going to use
		wc.hInstance = hInstance;					//gives the class a pointer to the main application object
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);	//creates a default icon
		wc.hCursor = LoadCursor(0, IDC_ARROW);		//creates the default cursor
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
			CW_USEDEFAULT, CW_USEDEFAULT,	//the width/height of window in pixels
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
			}
		}
		return msg.wParam;	//windows expects us to return th wParam of the WM_QUIT message
	}

}


//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd)
//{
//	return TrueWinMain(hInstance, hPrevInstance, pCmdLine, nShowCmd);
//}