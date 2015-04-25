#pragma once

#ifndef __RJH__
#define __RJH__

#include "RJMain.h"

// Function Prototypes
int WINAPI			WinMain(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd);
LRESULT CALLBACK	WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int					EnterMsgLoop( bool (RJMain::*ptr_display)(void) );




#endif