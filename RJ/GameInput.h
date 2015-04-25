#pragma once

#ifndef __GameInputH__
#define __GameInputH__

#include "DX11_Core.h"
#include <dinput.h>
#include "Utility.h"

// Device types
enum DIRECTINPUTTYPE
{
  DIT_KEYBOARD,
  DIT_MOUSE,
  DIT_FORCE_DWORD = 0x7fffffff
};

enum MouseInputControlMode
{
	MC_MOUSE_FLIGHT,
	MC_COCKPIT_CONTROL
};


class GameInputDevice
{
public:
	struct InputKey 
	{
		DWORD keycode; char keychar; 
		InputKey(DWORD _keycode, char _keychar) { keycode = _keycode; keychar = _keychar; }
		static const InputKey None;
	};

    GameInputDevice();
    ~GameInputDevice() { Release(); }

    BOOL Initialise( LPDIRECTINPUT8 pDI, HWND hWnd, DIRECTINPUTTYPE type );
    void Release();
    void Read();
	CMPINLINE BOOL GetKey(DWORD key) { return m_pressedKeys[key]; }
    void LockKey( DWORD key );
	void ConsumeKey(DWORD key);
	void ConsumeAllKeys(void);

    long GetX() { return m_x; }
    long GetY() { return m_y; }
	INTVECTOR2 GetCursor() { return m_cursor; }
	long GetXDelta()    { return m_mouseState.lX; }
    long GetYDelta()    { return m_mouseState.lY; }
    long GetZDelta()    { return m_mouseState.lZ; }
    BOOL* GetKeys() { return m_pressedKeys; }
	
	BOOL* GetButtons() { return m_pressedButtons; }
	bool LMB() { return (m_pressedButtons[0] == TRUE); }
	bool RMB() { return (m_pressedButtons[1] == TRUE); }

	BOOL* GetButtonsHeld() { return m_isdown; }
	bool LMBIsHeld() { return (m_isdown[0] == TRUE); }
	bool RMBIsHeld() { return (m_isdown[1] == TRUE); }

	BOOL* GetButtonsFirstDown() { return m_firstdown; }
	bool LMBFirstDown() { return (m_firstdown[0] == TRUE); }
	bool RMBFirstDown() { return (m_firstdown[1] == TRUE); }

	BOOL* GetButtonsFirstUp() { return m_firstup; }
	bool LMBFirstUp() { return (m_firstup[0] == TRUE); }
	bool RMBFirstUp() { return (m_firstup[1] == TRUE); }

	INTVECTOR2 *GetMouseStartPosition() { return m_startpos; }
	INTVECTOR2 GetLMBStartPosition() { return m_startpos[0]; }
	INTVECTOR2 GetRMBStartPosition() { return m_startpos[1]; }

	InputKey ReadTextInputKey(void);

private:

    LPDIRECTINPUTDEVICE8  m_pDevice;
    HWND                  m_hWnd;
    DIRECTINPUTTYPE       m_type;
    char                  m_keyboardState[256];
    BOOL                  m_pressedKeys[256];
    DIMOUSESTATE          m_mouseState;
    BOOL                  m_keyLock[256];
	BOOL                  m_pressedButtons[4];		// Mouse buttons that are currently pressed
    long                  m_x, m_y;					// Cursor position	
	INTVECTOR2			  m_cursor;					// Cursor position
	INTVECTOR2			  m_screencursor;			// Cursor position in global screen coordinates
	BOOL				  m_isdown[4];				// Identifies whether each mouse button is being held down
	BOOL				  m_firstdown[4];			// Identifies whether this is the first press of each mouse button
	BOOL				  m_firstup[4];				// Identifies whether this is the first release of each mouse button
	INTVECTOR2			  m_startpos[4];			// Stores the starting position of each mouse down event

public:
	struct ALPHANUM_KEY_DATA
	{
		int index;
		char character;
		int DIKEY;				// The key that is being held, assuming no modifiers

		ALPHANUM_KEY_DATA() { index = -1; character = 0; DIKEY = -1; }
		ALPHANUM_KEY_DATA(int _index, char _character, int _DIKEY) { index = _index; character = _character; DIKEY = _DIKEY; }
	};

	static const int ALPHANUMERIC_KEY_COUNT = 10 + 26 + 1;			// Num numeric chars + num alpha chars + num special chars
	static const ALPHANUM_KEY_DATA m_alphanumkeys[ALPHANUMERIC_KEY_COUNT];


};

#endif