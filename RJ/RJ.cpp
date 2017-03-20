#include "Utility.h"
#include "TestApplication.h"

#include "RJ.h"



//
// WndProc
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
		
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{

	// Initialise the application, and enter the main message loop if we are successful
	Result result = Game::Application.Initialise(hinstance, WndProc);

	if (result == ErrorCodes::NoError)
	{
		// DEBUG: Run tests if required
#		if (defined(_DEBUG) && defined(RJ_RUN_TESTS))
			TestApplication TestApp;
			TestApp.RunAllTests(Game::Application);
#		else

			// Enter the main game loop
			EnterMsgLoop(&RJMain::Display);

#		endif
	}
	else
	{
		#ifdef _DEBUG
			OutputDebugString(concat("\n\n***** Initialisation failed with error code: ")(result)(" *****\n\n").str().c_str());
		#endif
	}

	// Terminate the application when execution completes, or if initialisation failed
	Game::Application.TerminateApplication();

	// TODO: Return the eventual ErrorCode here, either 0 for no error or the specific error that caused a failure (e.g. return value from Initialise())
}

int EnterMsgLoop( bool (RJMain::*ptr_display)(void) )
{
	MSG msg;
	::ZeroMemory(&msg, sizeof(MSG));

	// Initialise the frame timers before entering the message loop
	Game::Application.InitialiseInternalClock();

	// Run the message loop until a WM_QUIT message is received
	while(msg.message != WM_QUIT)
	{
		if(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
        {	
			(Game::Application.*ptr_display)();
        }
    }
    return (int)msg.wParam;
}


