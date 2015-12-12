#pragma once

#ifndef __RJMainH__
#define __RJMainH__

//#define DEBUG_LOGINSTANCECREATION
//#define DEBUG_LOGALLOCATEDMEMORY
//#include "vld.h"

//#pragma comment(lib, "pathcch.lib")
#include "ErrorCodes.h"
#include "GameDataExtern.h"
class SimpleShip;
class ComplexShip;
class SpaceProjectile;
class SpaceTurret;
class TurretController;


// This class has no special alignment requirements
class RJMain
{

public:

	// Default constructor
	RJMain(void);

	// Update window size details based on desired width/height parameters, recalculating for windowed mode as required
	void				UpdateWindowSizeParameters(int screenWidth, int screenHeight, bool fullscreen);

	// Creates the main application window
	HWND				CreateMainWindow(HINSTANCE hInstance, WNDPROC wndproc);

	// Retrieve data on the executable and working directory
	Result				RetrieveExecutableData(void);

	// Initialises and runs the internal clocks that are used to manage game execution 
	void				InitialiseInternalClock(void);
	void				RunInternalClockCycle(void);
	void				EndInternalClockCycle(void);

	// Initialisation methods
	Result				Initialise(HINSTANCE hinstance, WNDPROC wndproc);
	void				InitialiseComponentPointers(void);
	Result				InitialiseWindow(void);
	Result				InitialiseDirect3DApplication(void);
	Result				InitialiseDirectInput(void);
	Result				InitialiseDirect3D(void);
	Result				InitialiseRegions(void);
	Result				InitialiseUniverse(void);
	Result				InitialiseUserInterface(void);
	Result				InitialisePlayer(void);
	Result				InitialiseCoreDataStructures(void);
	Result				InitialiseStateManager(void);
	Result				InitialiseLogging(void);
	Result				InitialiseShipData(void);
	Result				InitialiseActorData(void);

	// Methods to load game data & config
	Result				LoadPlayerConfig(void);
	Result				LoadAllGameData(void);
	Result				InitialiseGameDataDependencies(void);
	Result				InitialiseLoadedGameData(void);

	// Primary display method to execute each cycle
	bool				Display(void);

	// Tracks the FPS of the application and renders it if required
	void				PerformFPSCalculations(void);

	// User input methods
	void				ReadUserInput(void);
	void				ProcessKeyboardInput(void);
	void				ProcessMouseInput(void);

	// Accept keyboard and mouse input for the debug camera
	void				AcceptDebugCameraKeyboardInput(void);
	void				AcceptDebugCameraMouseInput(void);

	// Region update methods
	void				UpdateRegions(void);

	// Termination methods
	void				TerminateApplication(void);
	void				TerminateObjectRegisters(void);
	void				TerminateCoreDataStructures(void);
	void				TerminateStateManager(void);
	void				TerminateLogging(void);
	void				TerminateRegions(void);
	void				TerminateUserInterface(void);
	void				TerminateUniverse(void);
	void				TerminateMemoryPools(void);

	// Methods to pause/unpause the application
	void				Pause(void);
	void				Unpause(void);
	CMPINLINE void		TogglePause(void)				{ SetPauseState(!Game::Paused); }
	CMPINLINE void		SetPauseState(bool pause)		{ if (pause) Pause(); else Unpause(); }


	// *** DEBUG ***
	void __CreateDebugScenario(void);
	void DEBUGDisplayInfo(void);
	void DebugRenderSpaceCollisionBoxes(void);
	void DebugRenderEnvironmentCollisionBoxes(ComplexShip *parent);
	void DebugCCDSphereTest(void);
	void DebugCCDOBBTest(void);
	void DebugFullCCDTest(void);




	// Default destructor
	~RJMain(void);


protected:

	// Window details
	HWND m_hwnd;
	HINSTANCE m_hinstance;
	WNDPROC	m_wndproc;
	DWORD m_wndstyle, m_wndstyleex;

	// Input controllers
	LPDIRECTINPUT8 m_di;
	
	// Allocate space for memory tracking, if in debug mode and have this enabled
	#if defined(_DEBUG) && defined(DEBUG_LOGALLOCATEDMEMORY)
		_CrtMemState m_memstate;
	#endif

	// Debug rendering variables
	bool m_debuginfo_flightstats;
	bool m_debuginfo_renderinfo;
	bool m_debuginfo_collisiondata;
	bool m_debug_ccdspheretest;
	bool m_debug_ccdobbtest;


public:
	// Debug objects for testing; delete after use
	SimpleShip *ss, *s2, *s3[3];
	SimpleShip *sproj;
	ComplexShip *cs;
	Actor *a1;
	SpaceProjectile *proj;

	ID3D11Buffer			*m_vertexBuffer, *m_indexBuffer;
	UINT					m_vertexcount;
};









#endif