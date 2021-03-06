#pragma once

#ifndef __GameInputH__
#define __GameInputH__

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include "DX11_Core.h"
#include <dinput.h>
#include "Utility.h"
#include "BasicRay.h"
#include "Ray.h"

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



// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class GameInputDevice : public ALIGN16<GameInputDevice>
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
	void ReadMouseData(void);
	void ReadKeyboardData(void);

	CMPINLINE BOOL GetKey(DWORD key) { return m_pressedKeys[key]; }
    void LockKey(DWORD key);
	void ConsumeKey(DWORD key);
	void LockAndConsumeKey(DWORD key);
	void ConsumeAllKeys(void);

	CMPINLINE long GetX() { return m_x; }
	CMPINLINE long GetY() { return m_y; }
	CMPINLINE INTVECTOR2 GetCursor() { return m_cursor; }
	CMPINLINE long GetXDelta()    { return m_xdelta; }
	CMPINLINE long GetYDelta()    { return m_ydelta; }
	CMPINLINE long GetZDelta()    { return m_zdelta; }
    
	CMPINLINE XMFLOAT2 GetNormalisedMousePos(void) const { return m_mousepos_norm; }
	CMPINLINE const XMFLOAT2 & GetNormalisedMousePositionRef(void) const { return m_mousepos_norm; }
	
	CMPINLINE XMFLOAT2 GetNormalisedMouseDelta(void) const { return m_mousedelta_norm; }
	CMPINLINE const XMFLOAT2 & GetNormalisedMouseDeltaRef(void) const { return m_mousedelta_norm; }

	CMPINLINE const BasicRay &		GetWorldSpaceMouseBasicRay(void) const		{ return m_mouse_world_basicray; }
	CMPINLINE const Ray &			GetWorldSpaceMouseRay(void) const			{ return m_mouse_world_ray; }
	
	CMPINLINE BOOL * GetKeys() { return m_pressedKeys; }

	CMPINLINE bool KeyFirstUp(DWORD key) { return m_keyfirstup[key]; }
	CMPINLINE bool KeyFirstDown(DWORD key) { return m_keyfirstdown[key]; }
	CMPINLINE BOOL * GetKeyFirstUpData(void) { return m_keyfirstup; }
	CMPINLINE BOOL * GetKeyFirstDownData(void) { return m_keyfirstdown; }

	CMPINLINE bool KeyHeld(DWORD key) { return (m_pressedKeys[key] || m_keyLock[key]); }
	
	CMPINLINE BOOL* GetButtons() { return m_pressedButtons; }
	CMPINLINE bool LMB() { return (m_pressedButtons[0] == TRUE); }
	CMPINLINE bool RMB() { return (m_pressedButtons[1] == TRUE); }

	CMPINLINE BOOL* GetButtonsHeld() { return m_isdown; }
	CMPINLINE bool LMBIsHeld() { return (m_isdown[0] == TRUE); }
	CMPINLINE bool RMBIsHeld() { return (m_isdown[1] == TRUE); }

	CMPINLINE BOOL* GetButtonsFirstDown() { return m_firstdown; }
	CMPINLINE bool LMBFirstDown() { return (m_firstdown[0] == TRUE); }
	CMPINLINE bool RMBFirstDown() { return (m_firstdown[1] == TRUE); }

	CMPINLINE BOOL* GetButtonsFirstUp() { return m_firstup; }
	CMPINLINE bool LMBFirstUp() { return (m_firstup[0] == TRUE); }
	CMPINLINE bool RMBFirstUp() { return (m_firstup[1] == TRUE); }

	CMPINLINE INTVECTOR2 *GetMouseStartPosition() { return m_startpos; }
	CMPINLINE INTVECTOR2 GetLMBStartPosition() { return m_startpos[0]; }
	CMPINLINE INTVECTOR2 GetRMBStartPosition() { return m_startpos[1]; }

	CMPINLINE bool ShiftDown(void) const		{ return (m_pressedKeys[DIK_LSHIFT] == TRUE || m_pressedKeys[DIK_RSHIFT] == TRUE); }
	CMPINLINE bool CtrlDown(void) const			{ return (m_pressedKeys[DIK_LCONTROL] == TRUE || m_pressedKeys[DIK_RCONTROL] == TRUE); }
	CMPINLINE bool AltDown(void) const			{ return (m_pressedKeys[DIK_LALT] == TRUE || m_pressedKeys[DIK_RALT] == TRUE); }

	InputKey ReadTextInputKey(void);

private:

    LPDIRECTINPUTDEVICE8  m_pDevice;
    HWND                  m_hWnd;
    DIRECTINPUTTYPE       m_type;
    char                  m_keyboardState[256];
    BOOL                  m_pressedKeys[256];
    BOOL                  m_keyLock[256];
	BOOL				  m_keyfirstup[256];
	BOOL				  m_keyfirstdown[256];
	BOOL                  m_pressedButtons[4];			// Mouse buttons that are currently pressed
    long                  m_x, m_y;						// Cursor position	
	long				  m_xdelta, m_ydelta, m_zdelta;	// Delta mouse movement this cycle
	INTVECTOR2			  m_cursor;						// Cursor position
	INTVECTOR2			  m_screencursor;				// Cursor position in global screen coordinates
	BOOL				  m_isdown[4];					// Identifies whether each mouse button is being held down
	BOOL				  m_firstdown[4];				// Identifies whether this is the first press of each mouse button
	BOOL				  m_firstup[4];					// Identifies whether this is the first release of each mouse button
	INTVECTOR2			  m_startpos[4];				// Stores the starting position of each mouse down event
	XMFLOAT2			  m_mousepos_norm;				// Normalised position of the mouse, in the range [-1.0 +1.0] with (0,0) in the screen centre
	XMFLOAT2			  m_mousedelta_norm;			// Normalised delta movement of the mouse this cycle, as a percentage of total screen bounds (range [0.0 1.0])
	long				  m_mousedelta_z;				// Current mouse z-delta, i.e. any scrolling of the mouse wheel / touchpad
	long				  m_lastmouse_z;				// Mouse z-position last frame
	BasicRay			  m_mouse_world_basicray;		// World-space ray pointing out of the camera and through the mouse position; used primarily for object picking.  Calc 1/frame
	Ray					  m_mouse_world_ray;			// World-space ray pointing out of the camera and through the mouse position; used primarily for object picking.  Calc 1/frame
	bool				  m_captured_mouse;				// Indicates whether we captured mouse input this cycle (we may not, e.g. if the window does not have focus)
	bool				  m_captured_keyboard;			// Indicates whether we captured keyboard input this cycle (we may not, e.g. if the window does not have focus)

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