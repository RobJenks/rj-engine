#include "Utility.h"
#include "TestApplication.h"
#include "RJMain.h"

#include "RJ.h"


// Determines whether the debug log application will be invoked upon application start
#define RJ_SHOW_DEBUG_LOG

// Conditionally-included headers
#if defined(_DEBUG) && defined(RJ_SHOW_DEBUG_LOG)
#	include <windows.h>
#endif

// Forward declarations
HANDLE CreateDebugLogWindow(void);
void TerminateDebugLogWindow(HANDLE process);

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
	// Open the debug log application if we are in debug mode, and if we have it enabled
#	if defined(_DEBUG) && defined(RJ_SHOW_DEBUG_LOG)
		HANDLE debug_log_handle = CreateDebugLogWindow();
		OutputDebugString(concat("Attempting to create debug log window; process handle: ")(debug_log_handle)("\n").str().c_str());
#	endif

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

	// Terminate the debug log process as well, if it was active
#	if defined(_DEBUG) && defined(RJ_SHOW_DEBUG_LOG)
		OutputDebugString(concat("Attempting to terminate debug log window with window handle ")(debug_log_handle)("\n").str().c_str());
		TerminateDebugLogWindow(debug_log_handle);
#	endif

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


#if defined(_DEBUG) && defined(RJ_SHOW_DEBUG_LOG)

	HANDLE CreateDebugLogWindow(void)
	{
		//std::string process_string = concat("\"\"")(base_path)("\\RJ-Log\\bin\\Release\\RJ-Log.exe\" \"")(base_path)("\\RJ\"\"").str();
		STARTUPINFO debug_log_startup_info;
		PROCESS_INFORMATION debug_log_process_info;

		ZeroMemory(&debug_log_process_info, sizeof(debug_log_process_info));
		ZeroMemory(&debug_log_startup_info, sizeof(debug_log_startup_info));
		debug_log_startup_info.cb = sizeof(debug_log_startup_info);

		std::string base_path = "C:\\Users\\robje\\Documents\\Visual Studio 2017\\Projects\\RJ";
		std::string process_name = concat(base_path)("\\RJ-Log\\bin\\Release\\RJ-Log.exe").str();
		std::string process_args = concat
			("\"")(process_name)("\" ")												// args[0] == process name
			("-logDirectory \"")(base_path)("\\RJ\" ")								// args[1..2] == location of log directory
			("-parentWindowClass \"")(RJMain::APPLICATION_WINDOW_CLASSNAME)("\" ")	// args[3..4] == parent window classname
			("-parentWindowName \"")(RJMain::APPLICATION_WINDOW_WINDOWNAME)("\" ")	// args[5..6] == parent window name
			.str();

		if (CreateProcess(process_name.c_str(), (LPSTR)process_args.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &debug_log_startup_info, &debug_log_process_info) == TRUE)
		{
			return debug_log_process_info.hProcess;
		}

		return (HANDLE)0;
	}
	
	void TerminateDebugLogWindow(HANDLE process)
	{
		if (process == (HANDLE)0) return;

		TerminateProcess(process, 0U);
	}


#endif
