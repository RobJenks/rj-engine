#pragma once

#ifndef __GameInputSystemH__
#define __GameInputSystemH__

#include "DX11_Core.h"
#include <dinput.h>
#include "GameInput.h"
#include "ErrorCodes.h"

namespace GameInputSystem {

	Result Initialise(HINSTANCE hInstance, HWND hwnd, LPDIRECTINPUT8 m_di, GameInputDevice *m_keyboard, GameInputDevice *m_mouse)
	{
		// We need a handle to the DX application
		if (!hwnd) return ErrorCodes::InvalidHwnd;

		// Attempt to create a new DI device
		if( FAILED( DirectInput8Create( hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_di, NULL ) ) )
			return ErrorCodes::CannotCreateDirectInputDevice;

		// Now create a device for each of the primary input devices: keyboard and mouse
		if ( !m_keyboard->Initialise(m_di, hwnd, DIT_KEYBOARD) ) {
			return ErrorCodes::CannotCreateGameInputDevice;
		}
		if ( !m_mouse->Initialise(m_di, hwnd, DIT_MOUSE) ) {
			return ErrorCodes::CannotCreateGameInputDevice;
		}

		// Return success
		return ErrorCodes::NoError;
	}

	void Release(LPDIRECTINPUT8 *m_di, GameInputDevice *m_keyboard, GameInputDevice *m_mouse) 
	{
		m_keyboard->Release();
		m_mouse->Release();
	}
}



#endif