#include "GameInput.h"
#include "GameDataExtern.h" //tmp
#include "UserInterface.h" // tmp
#include "CoreEngine.h"//tmp
#include "D3DMain.h"//tmp


// Null input key
const GameInputDevice::InputKey GameInputDevice::InputKey::None = GameInputDevice::InputKey(0, 0);

// Initialisation of alphanumeric key data
const GameInputDevice::ALPHANUM_KEY_DATA GameInputDevice::m_alphanumkeys[GameInputDevice::ALPHANUMERIC_KEY_COUNT] = 
{
	GameInputDevice::ALPHANUM_KEY_DATA( 0, '0', DIK_0 ), 
	GameInputDevice::ALPHANUM_KEY_DATA( 1, '1', DIK_1 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 2, '2', DIK_2 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 3, '3', DIK_3 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 4, '4', DIK_4 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 5, '5', DIK_5 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 6, '6', DIK_6 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 7, '7', DIK_7 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 8, '8', DIK_8 ),
	GameInputDevice::ALPHANUM_KEY_DATA( 9, '9', DIK_9 ),

	GameInputDevice::ALPHANUM_KEY_DATA( 10, 'a', DIK_A ),
	GameInputDevice::ALPHANUM_KEY_DATA( 11, 'b', DIK_B ),
	GameInputDevice::ALPHANUM_KEY_DATA( 12, 'c', DIK_C ),
	GameInputDevice::ALPHANUM_KEY_DATA( 13, 'd', DIK_D ),
	GameInputDevice::ALPHANUM_KEY_DATA( 14, 'e', DIK_E ),
	GameInputDevice::ALPHANUM_KEY_DATA( 15, 'f', DIK_F ),
	GameInputDevice::ALPHANUM_KEY_DATA( 16, 'g', DIK_G ),
	GameInputDevice::ALPHANUM_KEY_DATA( 17, 'h', DIK_H ),
	GameInputDevice::ALPHANUM_KEY_DATA( 18, 'i', DIK_I ),
	GameInputDevice::ALPHANUM_KEY_DATA( 19, 'j', DIK_J ),
	GameInputDevice::ALPHANUM_KEY_DATA( 20, 'k', DIK_K ),
	GameInputDevice::ALPHANUM_KEY_DATA( 21, 'l', DIK_L ),
	GameInputDevice::ALPHANUM_KEY_DATA( 22, 'm', DIK_M ),
	GameInputDevice::ALPHANUM_KEY_DATA( 23, 'n', DIK_N ),
	GameInputDevice::ALPHANUM_KEY_DATA( 24, 'o', DIK_O ),
	GameInputDevice::ALPHANUM_KEY_DATA( 25, 'p', DIK_P ),
	GameInputDevice::ALPHANUM_KEY_DATA( 26, 'q', DIK_Q ),
	GameInputDevice::ALPHANUM_KEY_DATA( 27, 'r', DIK_R ),
	GameInputDevice::ALPHANUM_KEY_DATA( 28, 's', DIK_S ),
	GameInputDevice::ALPHANUM_KEY_DATA( 29, 't', DIK_T ),
	GameInputDevice::ALPHANUM_KEY_DATA( 30, 'u', DIK_U ),
	GameInputDevice::ALPHANUM_KEY_DATA( 31, 'v', DIK_V ),
	GameInputDevice::ALPHANUM_KEY_DATA( 32, 'w', DIK_W ),
	GameInputDevice::ALPHANUM_KEY_DATA( 33, 'x', DIK_X ),
	GameInputDevice::ALPHANUM_KEY_DATA( 34, 'y', DIK_Y ),
	GameInputDevice::ALPHANUM_KEY_DATA( 35, 'z', DIK_Z ),

	GameInputDevice::ALPHANUM_KEY_DATA( 36, ' ', DIK_SPACE)
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Default constructor
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
GameInputDevice::GameInputDevice()
{
    m_pDevice = NULL;
    m_x = m_y = 0;
	m_cursor = m_screencursor = INTVECTOR2(0, 0);
    ZeroMemory( m_keyLock, sizeof( BOOL ) * 256 );
    ZeroMemory( &m_mouseState, sizeof( DIMOUSESTATE ) );
    ZeroMemory( m_keyboardState, 256 );
    ZeroMemory( m_pressedKeys, 256 );
    ZeroMemory( m_pressedButtons, 4 );
	ZeroMemory( m_isdown, 4);
	ZeroMemory( m_firstdown, 4);
	ZeroMemory( m_firstup, 4);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Initialises a new input device
Parameters:
[in] pDI - IDirectInput interface
[in] hWnd - Window handle
[in] type - Member of DIRECTINPUTTYPE enum.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
BOOL GameInputDevice::Initialise( LPDIRECTINPUT8 pDI, HWND hWnd, DIRECTINPUTTYPE type )
{
    // Check for a valid parent DIRECTINPUT8 interface
    if ( pDI == NULL || type == DIT_FORCE_DWORD )
    {
        return FALSE;
    }
    Release();

    DIDATAFORMAT* pDataFormat;
    m_hWnd = hWnd;
    m_type = type;
    
    // Create the device
    switch( type )
    {
    case DIT_KEYBOARD:
		if ( FAILED( pDI->CreateDevice( GUID_SysKeyboard, &m_pDevice, NULL ) ) )
        {
//            SHOWERROR( "Unable to create keyboard device.", __FILE__, __LINE__ );
            return FALSE;
        }
        pDataFormat = (DIDATAFORMAT*)&c_dfDIKeyboard;
        break;
    case DIT_MOUSE:
        if ( FAILED( pDI->CreateDevice( GUID_SysMouse, &m_pDevice, NULL ) ) )
        {
//            SHOWERROR( "Unable to create mouse device.", __FILE__, __LINE__ );
            return FALSE;
        }
        pDataFormat = (DIDATAFORMAT*)&c_dfDIMouse;
        break;
    default: 
        return FALSE;
    }

    // Set the data format
    if( FAILED( m_pDevice->SetDataFormat( pDataFormat ) ) )
    {
//        SHOWERROR( "Unable to set input data format.", __FILE__, __LINE__ );
        return FALSE;
    }

    // Set the cooperative level
	if( FAILED( m_pDevice->SetCooperativeLevel( hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) ) )
    {
//        SHOWERROR( "Unable to set input cooperative level.", __FILE__, __LINE__ );
        return FALSE;
    }

    // Acquire the device
    if( FAILED( m_pDevice->Acquire() ) )
    {
//        SHOWERROR( "Unable to acquire the input device.", __FILE__, __LINE__ );
        return FALSE;
    }

    return TRUE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Get the current device state.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GameInputDevice::Read()
{
    if ( !m_pDevice )
    {
        return;
    }
    
    // Grab the data 
    if ( m_type == DIT_MOUSE )
    {
        HRESULT hr = m_pDevice->GetDeviceState( sizeof( DIMOUSESTATE ), (LPVOID)&m_mouseState );
        if ( FAILED( hr )  )
        {
            if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
            {
                // Device is lost, try to reaquire it
                m_pDevice->Acquire();
            }
            return;
        }
        
		// Store cursor position in client (default) and screen coordinates
		POINT pos;
        GetCursorPos( &pos );
		m_screencursor.x = pos.x;
		m_screencursor.y = pos.y;
		ScreenToClient(m_hWnd, &pos); 
		m_cursor.x = m_x = pos.x;
		m_cursor.y = m_y = pos.y;
		
        // Get pressed buttons
        for ( int i = 0; i < 4; i++ )
        {
            if ( m_mouseState.rgbButtons[i] & 0x80 )
            {
				// Record whether the button is down, and whether this is the first press of the button
                m_pressedButtons[i] = TRUE;
				m_firstdown[i] = !m_isdown[i];

				// If the button wasn't previously held down, record the start position and button-held flag
				if (!m_isdown[i]) {
					m_isdown[i] = true;
					m_startpos[i] = m_cursor;
				}
            }
            else
            {
				// If the button has been held down to this point then we have a mouse up event
				if (m_isdown[i]) m_firstup[i] = TRUE; else m_firstup[i] = FALSE;

				// Reset flags now the button is not being held down
                m_pressedButtons[i] = m_firstdown[i] = m_isdown[i] = FALSE;
            }
        }
    }
    else if ( m_type == DIT_KEYBOARD )
    {
        HRESULT hr = m_pDevice->GetDeviceState( 256, (LPVOID)&m_keyboardState );
        if ( FAILED( hr )  )
        {
            if ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
            {
                // Device is lost, try to reaquire it
                m_pDevice->Acquire();
            }
            return;
        }
        // Get pressed keys and release locks on key up
        for ( int i = 0; i < 256; i++ )
        {
            if ( !(m_keyboardState[i] & 0x80) )
            {
                // Key is up so release lock
                m_keyLock[i] = FALSE;
                m_pressedKeys[i] = FALSE;
            }
            else
            {
                // Key is pressed if it isn't locked
                m_pressedKeys[i] = !(m_keyLock[i]);
            }
        }
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Determines the alphanumeric key that is currently pressed, for text input, and also 
locks it to prevent multiple repeated characters appearing due to a high frame rate.  Also handles
capitalisation via modifier keys
Returns: The alphanumeric key (if any) being pressed for text input.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
GameInputDevice::InputKey GameInputDevice::ReadTextInputKey(void)
{
	// Account for modifier keys when returning input
	bool shift = (m_pressedKeys[DIK_LSHIFT] || m_pressedKeys[DIK_RSHIFT]);

	// Loop through the alphanumeric key array and look for the first one that is pressed
	for (int i=0; i<GameInputDevice::ALPHANUMERIC_KEY_COUNT; i++)
	{
		// Check whether this key is pressed
		if (m_pressedKeys[m_alphanumkeys[i].DIKEY]) 
		{
			// If it is, get the character in questions
			char key = m_alphanumkeys[i].character;

			// We store lower case char by default; if shift is pressed, convert this to upper case before returning
			if ( shift && (key >= 'a' && key <= 'z') )
			{
				// Capitalise this character
				key += ('A' - 'a');
			}

			// Lock the key to prevent multiple repeats due to a high frame rate
			LockKey(m_alphanumkeys[i].DIKEY);

			// Return the character that was pressed
			return GameInputDevice::InputKey(m_alphanumkeys[i].DIKEY, key);
		}
	}
	
	// Also check for some special keys that are not alphanumeric, but are still valid text input
	if (m_pressedKeys[DIK_MINUS])		{ LockKey(DIK_MINUS);			return GameInputDevice::InputKey(DIK_MINUS, (shift ? '_' : '-')); }
	if (m_pressedKeys[DIK_SEMICOLON])	{ LockKey(DIK_SEMICOLON);		return GameInputDevice::InputKey(DIK_SEMICOLON, (shift ? ':' : ';')); }
	if (m_pressedKeys[DIK_LBRACKET])	{ LockKey(DIK_LBRACKET);		return GameInputDevice::InputKey(DIK_LBRACKET, '{'); }
	if (m_pressedKeys[DIK_RBRACKET])	{ LockKey(DIK_RBRACKET);		return GameInputDevice::InputKey(DIK_RBRACKET, '}'); }
	if (m_pressedKeys[DIK_APOSTROPHE])  { LockKey(DIK_APOSTROPHE);		return GameInputDevice::InputKey(DIK_APOSTROPHE,'\''); }
	if (m_pressedKeys[DIK_BACKSLASH])	{ LockKey(DIK_BACKSLASH);		return GameInputDevice::InputKey(DIK_BACKSLASH, '\\'); }
	if (m_pressedKeys[DIK_SLASH])		{ LockKey(DIK_SLASH);			return GameInputDevice::InputKey(DIK_SLASH, (shift ? '?' : '/')); }

	// None of the alphanumeric keys are being pressed, so return 0 to signify no user input
	return GameInputDevice::InputKey::None;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Locks a key so it is only read once per key down.
Parameters:
[in] key - Key to lock.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GameInputDevice::LockKey( DWORD key )
{
    m_keyLock[key] = TRUE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Consumes a key so it is not read by any other consumer than the current one.
Parameters:
[in] key - Key to consume.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GameInputDevice::ConsumeKey(DWORD key)
{
	m_pressedKeys[key] = FALSE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Consumes all non-system keys so they are not read by any other consumer than the current one.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GameInputDevice::ConsumeAllKeys(void)
{
	// Record the state of keys that we will NOT consume; these are 'system' keys or those that we
	// otherwise want to ensure still pass through to the rest of the application
	BOOL STATE_ESC = m_pressedKeys[DIK_ESCAPE];
	BOOL STATE_GRAVE = m_pressedKeys[DIK_GRAVE];

	// Zero the entire keystate array to 'consume' all keys
	memset(m_pressedKeys, 0, sizeof(m_pressedKeys[0]) * 256);

	// Now replace the state of any reserved keys, so that they are not consumed
	m_pressedKeys[DIK_ESCAPE] = STATE_ESC;
	m_pressedKeys[DIK_GRAVE] = STATE_GRAVE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Summary: Free resources
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void GameInputDevice::Release()
{
    if( m_pDevice )
    {
        m_pDevice->Unacquire();
        m_pDevice->Release();
    }
}