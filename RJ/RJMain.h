#pragma once

#ifndef __RJMainH__
#define __RJMainH__

//#define DEBUG_LOGINSTANCECREATION
//#define DEBUG_LOGALLOCATEDMEMORY
//#include "vld.h"

//#pragma comment(lib, "pathcch.lib")
#include "ErrorCodes.h"
#include "GameDataExtern.h"
#include "ObjectReference.h"
class SimpleShip;
class ComplexShip;
class SpaceProjectile;
class SpaceTurret;
class TurretController;
class LightSource;


// This class has no special alignment requirements
class RJMain
{

public:

	// Application window properties
	static const char *	APPLICATION_WINDOW_CLASSNAME;
	static const char *	APPLICATION_WINDOW_WINDOWNAME;

	// Default constructor
	RJMain(void);

	// Update window size details based on desired width/height parameters, recalculating for windowed mode as required
	void				UpdateWindowSizeParameters(int screenWidth, int screenHeight, bool fullscreen);

	// Creates the main application window
	HWND				CreateMainWindow(HINSTANCE hInstance, WNDPROC wndproc);
	void				FocusApplication(void);

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
	Result				InitialiseStaticData(void);
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

	// Indicates whether the application currently has focus
	CMPINLINE bool		HasFocus(void) const { return Game::HasFocus; }

	// Accept keyboard and mouse input for the debug camera
	void				AcceptDebugCameraKeyboardInput(void);
	void				AcceptDebugCameraMouseInput(void);

	// Region update methods
	void				UpdateRegions(void);

	// Primary quit method for the application
	void				Quit(void);

	// Termination methods
	void				TerminateApplication(void);
	void				TerminateCoreDataStructures(void);
	void				TerminateObjectSearchManager(void);
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


	// *** TESTING ***
	void RunAllTests(void);

	// *** DEBUG ***
	Ship * InitialiseTemporaryPlayerShip(void);
	void __CreateDebugScenario(void);
	void DEBUGDisplayInfo(void);
	void DebugRenderSpaceCollisionBoxes(void);
	void DebugRenderEnvironmentCollisionBoxes(ComplexShip *parent);
	void DebugCCDSphereTest(void);
	void DebugCCDOBBTest(void);
	void DebugFullCCDTest(void);
	void DebugFireBasicProjectile(const BasicRay & trajectory) const;

	void ActivateDebugPortalRenderingTest(const iObject *target);
	void DebugRunPortalRenderingTest(void);
	void DeactivateDebugPortalRenderingTest(void);

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

	bool m_debug_portalrenderingtest;
	ObjectReference<const iEnvironmentObject> m_debug_portalrenderingtest_subject;
	ObjectReference<ComplexShip> m_debug_portalrenderingtest_environment;

	// Perform any application updates with respect to the OS, e.g. testing whether the application is currently in focus
	void PerformApplicationOSUpdates(void);

	// Events raised when the application gains or loses focus
	void ApplicationLostFocus(void);
	void ApplicationGainedFocus(void);

public:
	// Debug objects for testing; delete after use
	ObjectReference<SimpleShip> ss, s2, s3[3], sproj;
	ObjectReference<ComplexShip> cs, cs2;
	ObjectReference<Actor> a1;
	ObjectReference<SpaceProjectile> proj;
	ObjectReference<LightSource> lt, lt2;

	ID3D11Buffer			*m_vertexBuffer, *m_indexBuffer;
	UINT					m_vertexcount;
};









#endif