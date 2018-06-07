
#include "DX11_Core.h" //#include "FullDX11.h"

#include <stdio.h>
#include <cstdio>
#include <string.h>
#include <iostream>
#include <tchar.h>
#include <functional>
#include <algorithm>
#include "time.h"


#include "FastMath.h"
#include "FileSystem.h"
#include "Logging.h"
#include "Octree.h"
#include "OctreePruner.h"
#include "GameSpatialPartitioningTrees.h"

#include "iObject.h"
#include "CoreEngine.h"
#include "CameraClass.h"
#include "AudioManager.h"
#include "LightShader.h"

#include "ParticleEngine.h"
#include "ParticleEmitter.h"

#include "GameInputSystem.h"

#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "ConditionalCompilation.h"		// DBG
#include "Utility.h"
#include "GameVarsExtern.h"
#include "GameDataExtern.h"
#include "GameObjects.h"
#include "RJDebug.h"
#include "Player.h"
#include "GameInput.h"
#include "MovementLogic.h"
#include "CameraClass.h"
#include "DataInput.h"
#include "FileInput.h"
#include "Model.h"
#include "UserInterface.h"
#include "UI_ShipBuilder.h"
#include "CentralScheduler.h"
#include "CollisionDetectionResultsStruct.h"
#include "GamePhysicsEngine.h"
#include "ObjectSearch.h"
#include "LightingManagerObject.h"
#include "FactionManagerObject.h"
#include "LogManager.h"
#include "SpaceSystem.h"

#include "Profiler.h"
#include "FrameProfiler.h"
#include "MemDebug.h"

#include "SpaceEmitter.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipElement.h"
#include "CapitalShipPerimeterBeacon.h"

#include "CSQuartersTile.h"					// DBG
#include "CSPowerGeneratorTile.h"			// DBG
#include "ProductionProgress.h"				// DBG
#include "ProductionCost.h"					// DBG
#include "NavNetwork.h"						// DBG
#include "OverlayRenderer.h"				// DBG

#include "ActorAttributes.h"				// DBG
#include "Order_MoveToPosition.h"			// DBG
#include "Order_MoveToTarget.h"				// DBG
#include "Order_AttackBasic.h"				// DBG
#include "Order_ActorMoveToPosition.h"		// DBG
#include "Order_ActorMoveToTarget.h"		// DBG
#include "Order_ActorTravelToPosition.h"	// DBG
#include "CameraPath.h"						// DBG
#include "CSLifeSupportTile.h"				// DBG
#include "AABB.h"							// DBG
#include "SpaceProjectileDefinition.h"		// DBG
#include "SpaceProjectile.h"				// DBG
#include "ProjectileLauncher.h"				// DBG
#include "Attachment.h"						// DBG
#include "ArticulatedModel.h"				// DBG
#include "SpaceTurret.h"					// DBG
#include "TurretController.h"				// DBG
#include "BasicProjectile.h"				// DBG
#include "BasicProjectileDefinition.h"		// DBG
#include "BasicProjectileSet.h"				// DBG
#include "VolLineShader.h"					// DBG
#include "VolumetricLine.h"					// DBG
#include "Ray.h"							// DBG
#include "Modifier.h"						// DBG
#include "ModifiedValue.h"					// DBG
#include "DebugTest.h"						// DBG
#include "ObjectReference.h"				// DBG
#include "EnvironmentTree.h"				// DBG
#include "CopyObject.h"						// DBG
#include "LightSource.h"					// DBG
#include "ObjectPicking.h"					// DBG
#include "DynamicTileSet.h"					// DBG
#include "Modifiers.h"						// DBG
#include "StandardModifiers.h"				// DBG
#include "ElementStateDefinition.h"			// DBG
#include "EnvironmentMap.h"					// DBG
#include "EnvironmentMapBlendMode.h"		// DBG
#include "EnvironmentPowerMap.h"			// DBG
#include "Weapon.h"							// DBG
#include "TerrainDefinition.h"				// DBG
#include "DynamicTerrain.h"					// DBG
#include "DynamicTerrainDefinition.h"		// DBG
#include "DataObjectRelay.h"				// DBG
#include "DataObjectSwitch.h"				// DBG
#include "DataObjectContinuousSwitch.h"		// DBG
#include "DataObjectDebugLogger.h"			// DBG
#include "DataObjectEngineThrustController.h"	// DBG
#include "DataObjectEngineHeadingController.h"	// DBG
#include "TextRenderer.h"			// DBG
#include "DecalRenderingManager.h"	// DBG
#include "Frustum.h"

#include "Equipment.h"
#include "Engine.h"
#include "Hardpoint.h"
#include "HpEngine.h"

#include "GameUniverse.h"
#include "ImmediateRegion.h"
#include "SystemRegion.h"
#include "Terrain.h"

#include "TextBlock.h"

#include "SkinnedModel.h"
#include "SkinnedNormalMapShader.h"
#include "ActorAttributes.h"
#include "ActorBase.h"
#include "Actor.h"
#include "DebugInvocation.h"
#include <random>
#include <stdlib.h> 

#include "RJMain.h"


// TODO: Several VC++ safety measures added to STL even in default release builds, should disable for release builds 
// of this application.  Should set "_HAS_ITERATOR_DEBUGGING=0"  "_SECURE_SCL = 0".  See 
// http://assimp.sourceforge.net/lib_html/install.html for details


// Application window properties
const char * RJMain::APPLICATION_WINDOW_CLASSNAME = "RJ-D3D11-Main";
const char * RJMain::APPLICATION_WINDOW_WINDOWNAME = "RJ-D3D11-Main";

// Default constructor
RJMain::RJMain(void)
{
	// Define window styles for the application
	m_wndstyle = WS_OVERLAPPEDWINDOW | WS_EX_TOPMOST;
	m_wndstyleex = WS_EX_CLIENTEDGE;

	// Set default field values
	m_hwnd = 0;
	m_hinstance = 0;
	memset(&m_wndproc, 0, sizeof(WNDPROC));
	m_di = 0;

	// Enable debug text output
	m_debuginfo_flightstats = true;
	m_debuginfo_renderinfo = true;
	m_debuginfo_collisiondata = true;

	// Debug fields and settings
	m_debug_ccdspheretest = m_debug_ccdobbtest = false;
	m_debug_portalrenderingtest = false;
	m_debug_portalrenderingtest_subject = NULL;
}

// Retrieve data on the executable and working directory
Result RJMain::RetrieveExecutableData(void)
{
	// Determine the full path (including filename) of the executable and store it
	const char *cexe = new char[4096];
	if (0 == GetModuleFileName(NULL, (LPSTR)cexe, 4096)) return ErrorCodes::CouldNotDetermineExecutablePath;
	Game::ExeName = std::string(cexe);

	// Convert to a wstring for input into the subsequent cch function
	std::wstring wspath = ConvertStringToWString(Game::ExeName);
	const WCHAR *wpath = wspath.c_str();
	WCHAR *cpath = new WCHAR[4096];
	wcsncpy(cpath, wpath, 4096);

	// Parse the executable path to remove the filename, and store the resulting executable directory
	//if (FAILED(CchRemoveFileSpec((PWSTR)cpath, 4096))) return ErrorCodes::CouldNotParseExecutablePath;
	if (FileSystem::RemoveFileNameFromPathStringPathW(cpath) == FALSE) return ErrorCodes::CouldNotParseExecutablePath;
	std::wstring wsnewpath = std::wstring(cpath);
	Game::ExePath = ConvertWStringToString(wsnewpath);

	// Deallocate any temporary data and return success
	SafeDeleteArray(cexe); SafeDeleteArray(cpath);
	return ErrorCodes::NoError;
}

// Perform any application updates with respect to the OS, e.g. testing whether the application is currently in focus
void RJMain::PerformApplicationOSUpdates()
{
	// Check whether this window is now in focus.  GetFGW may return NULL temporarily during context switches so we also treat
	// this as out of focus.  
	// TODO [Awareness]: MSDN documentation for GetFocus, which is similar to GetFGW, implies that the
	// HWND will only be returned if the currently-focused window is within the current thread's message queue.  If this is the case
	// it may encounter issues when multithreaded.  However defaulting NULL -> FALSE may automatically handle this
	// TODO [XPlatform]: Windows-specific
	HWND fgw = GetForegroundWindow();
	bool had_focus = Game::HasFocus;
	Game::HasFocus = (fgw != NULL && fgw == m_hwnd);

	if (Game::HasFocus)
	{
		if (!had_focus) ApplicationGainedFocus();
	}
	else
	{
		if (had_focus) ApplicationLostFocus();
	}

}

// Event raised when the application loses focus
void RJMain::ApplicationLostFocus(void)
{
	if (Game::PauseOnLoseFocus) Pause();
}

// Event raised when the application gains focus
void RJMain::ApplicationGainedFocus(void)
{
	
}

// Begins a new internal cycle, calculating frame deltas and the new internal clock values
void RJMain::RunInternalClockCycle(void)
{
	/* Persistent clock */

	// Calculate time modifiers in ms/secs based on time delta since last frame, then store them globally for use in all methods
	// These are the 'persistent' timers that continue regardless of whether the game itself is paused
	Game::PersistentClockDelta = ((unsigned int)timeGetTime() - Game::PreviousPersistentClockMs);

	// Apply a limit on maximum frame delta time to prevent e.g. temporary system hangs from throwing off the simulation
	Game::PersistentClockDelta = min(Game::PersistentClockDelta, Game::C_MAX_FRAME_DELTA);

	// Calculate the new persistent clock time based on this delta
	Game::PersistentClockMs += Game::PersistentClockDelta;
	Game::PersistentTimeFactor = (float)Game::PersistentClockDelta * 0.001f;
	Game::PersistentTimeFactorV = XMVectorReplicate(Game::PersistentTimeFactor);
	Game::PersistentClockTime += Game::PersistentTimeFactor;


	/* In-game clock */

	// If we are not paused, we also want to update the 'game' timers that are used for in-game events
	if (!Game::Paused)
	{
		// Take the delta calculated by the persistent timer.  We cannot compare to the previous value since that would be affected by the pause
		Game::ClockDelta = Game::PersistentClockDelta;

		// Update the clocks based on this delta value
		Game::ClockMs += Game::ClockDelta;
		Game::TimeFactor = (float)Game::ClockDelta * 0.001f;
		Game::TimeFactorV = XMVectorReplicate(Game::TimeFactor);
		Game::ClockTime += Game::TimeFactor;
	}
}

// Ends an internal clock cycle, setting clock values ready for the next frame to begin
void RJMain::EndInternalClockCycle(void)
{
	// Store the previous clock value, for the purpose of calculating the clock delta next frame
	Game::PreviousPersistentClockMs = Game::PersistentClockMs;

	// If we are not paused, we also want to update the previous 'game' clock value as well
	if (!Game::Paused)
	{
		Game::PreviousClockMs = Game::ClockMs;
	}
}

// Pauses the application
void RJMain::Pause(void)
{
	// Set the flag to indicate the game is paused
	Game::Paused = true;

	// Reset the game timers (the persistent timers will continue).  Setting the deltas to zero means that any 
	// game operation that would use them is now suspended, since it believes no time is passing
	Game::ClockDelta = 0U;
	Game::TimeFactor = 0.0f;
}

// Unpauses the application
void RJMain::Unpause(void)
{
	// Clear the flag to indicate the game is now running again
	Game::Paused = false;

	// We do not need to take any action to bring the game timer back online.  It will automatically sync up in the next cycle
}


bool RJMain::Display(void)
{
	// Initial validation: the engine must be operational in order to begin rendering
	if (Game::Engine->Operational())
	{
		// Perform any application updates with respect to the OS, e.g. testing whether the application is currently in focus
		PerformApplicationOSUpdates();

		// Calculate time modifiers in ms/secs based on time delta since last frame, then store them globally for use in all methods
		RunInternalClockCycle();

		// Notify the frame profiler that a new frame is starting (if the profiler is enabled)
		RJ_FRAME_PROFILER_NEW_FRAME

		// Retrieve and validate required data
		if (Game::CurrentPlayer == NULL) return false;

		// Begin the simulation cycle
		RJ_FRAME_PROFILER_CHECKPOINT("Initialising simulation cycle");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_BeginCycle)
		{
			Game::Engine->BeginFrame();
			Game::Logic::BeginSimulationCycle();
			Game::CurrentPlayer->BeginSimulationCycle();
			Game::ObjectSearchManager::InitialiseFrame();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_BeginCycle)

		// Read user input from the mouse and keyboard
		RJ_FRAME_PROFILER_CHECKPOINT("Processing user input");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_ProcessInput)
		{
			// Read the current state of all input devices
			ReadUserInput();

			// Process all user inputs through the various UI components that require them.  Process through the 
			// UI first so that components with 'focus' can consume the keys before they reach the game controls
			D::UI->ProcessUserEvents(&Game::Keyboard, &Game::Mouse);

			// Perform immediate processing of mouse & keyboard input
			ProcessMouseInput();
			ProcessKeyboardInput();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_ProcessInput)

		// Run the central scheduler to process all scheduled jobs this frame
		RJ_FRAME_PROFILER_CHECKPOINT("Running central scheduler");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_CentralScheduler)
		{
			Game::Scheduler.RunScheduler();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_CentralScheduler);

		// Simluate the current valid area of universe.  Note: in future, to be split between full, real-time simulation
		// and more distant simulation that is less frequent/detailed/accurate
		RJ_FRAME_PROFILER_CHECKPOINT("Simulating all objects");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_SimulateSpaceObjectMovement)
		{
			// Simulate objects
			Game::Logic::SimulateAllObjects();

			// Simulate all projectiles in the current player system
			Game::Universe->GetCurrentSystem().Projectiles.SimulateProjectiles(Game::Universe->GetCurrentSystem().SpatialPartitioningTree);
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_SimulateSpaceObjectMovement)

		// Simulate all game physics
		RJ_FRAME_PROFILER_CHECKPOINT("Simulating physics and collision detection");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_CollisionDetection)
		{
			Game::PhysicsEngine.SimulatePhysics();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_CollisionDetection);

		// Update all regions, particularly those centred on the player ship
		RJ_FRAME_PROFILER_CHECKPOINT("Updating regions");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_UpdateRegions)
		{
			UpdateRegions();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_UpdateRegions);

		// Perform any required audio updates
		RJ_FRAME_PROFILER_CHECKPOINT("Performing audio update");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_UpdateAudio)
		{
			Game::Engine->GetAudioManager()->Update();
		}

		// Update the player state, including camera view calculation etc. for rendering
		RJ_FRAME_PROFILER_CHECKPOINT("Updating player state");
		Game::CurrentPlayer->UpdatePlayerState();

		/* *** Begin rendering process ****/

		// Calculate the current camera view matrix
		Game::Engine->GetCamera()->CalculateViewMatrix();

		// Clear the register of all visible objects.  This is generated during rendering so should immediately 
		// precede the call to CoreEngine::Render()
		Game::ClearVisibleObjectCollection();

		// Perform FPS calculations and render if required
		PerformFPSCalculations();

		// DEBUG DISPLAY FUNCTIONS
		RJ_FRAME_PROFILER_CHECKPOINT("Rendering debug info");
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_DebugInfoRendering)
		{
			DEBUGDisplayInfo();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_DebugInfoRendering);

		// Pass to the main rendering function in the core engine, to render everything required in turn
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_Render)
		{
			// Perform all rendering
			Game::Engine->Render();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_Render);

		// End the current frame
		Game::Engine->EndFrame();

		// Notify the frame profiler that the frame has ended (if the profiler is enabled)
		RJ_FRAME_PROFILER_CHECKPOINT("Frame complete");
		RJ_FRAME_PROFILER_END_FRAME
			
		// End the cycle by storing the previous clock values, for calculating the delta time in the next frame
		EndInternalClockCycle();

		// Log all profiling information, if profiling is enabled
		RJ_PROFILE_LOG; 
	}
	return true;
}

void RJMain::TerminateApplication()
{
	// Perform an initial flush of the game logs, to record any recent logs in case we are terminating due to an error
	// that could potentially cause a failure before full shutdown
	Game::Log.FlushAllStreams();

	// Terminate all objects in the game
	//Game::ShutdownObjectRegisters();

	// Release all standard model/geometry data
	Model::TerminateAllModelData();

	// Terminate the user interface and all UI controllers and layouts
	TerminateUserInterface();

	// Terminate and deallocate the universe & all system data
	TerminateUniverse();

	// Region termination functions
	TerminateRegions();

	// Shutdown all object search components and any associated cache data; must be performed 
	// before the central object registers themselves are destructed (due to cache references into the registers)
	TerminateObjectSearchManager();

	// Terminate all key game data structures (e.g. the space object Octree)
	TerminateCoreDataStructures();

	// Terminate the simulation state manager
	TerminateStateManager();

	// Termination functions for other core components outside of the game engine
	TerminateMathFunctions();								// Optimised high-speed math functions & cache

	// Terminate any memory-pooled objects we are maintaining
	TerminateMemoryPools();
	
	// Run termination functions for the core game engine, which will in turn terminate all components
	Game::Engine->ShutdownGameEngine();
	delete Game::Engine; Game::Engine = NULL;

	// Terminate logging components last, so we can use them to output details of the shutdown
	TerminateLogging();
}

// Initialises the internal clocks that are used to manage game execution 
void RJMain::InitialiseInternalClock(void)
{
	// Initialise the main timers and the frame-zero times to the current system ms count.
	Game::PersistentClockMs = 
	Game::PreviousPersistentClockMs = 
	Game::ClockMs = 
	Game::PreviousClockMs =					(unsigned int)timeGetTime();
}


// Calculates FPS data; executed once per frame
void RJMain::PerformFPSCalculations(void)
{
	// Calculate the FPS rate
	static float m_framecount = 0.0f; static float m_elapsedtime = 0.0f;
	++m_framecount;
	m_elapsedtime += Game::PersistentTimeFactor;

	// Recalculate once per second
	if (m_elapsedtime >= 1.0f)
	{
		// Calculate the new FPS value	
		Game::FPS = m_framecount / m_elapsedtime;
		m_framecount = m_elapsedtime = 0.0f;

		// If we are recording memory allocations, do so at the same time as the FPS calculation
#		if defined(_DEBUG) && defined(DEBUG_LOGALLOCATEDMEMORY)
			_CrtMemCheckpoint(&m_memstate);
#		endif
	}
		// Update the FPS text object, if it is being rendered
	if (Game::FPSDisplay && Game::FPS > 0.0f && Game::FPS < 99999.0f)
	{
		// Two versions of this display, depending on whether we are in debug mode and logging current allocations
		XMFLOAT4 font_colour(255.0f / 255.0f, 196.0f / 255.0f, 104.0f / 255.0f, 0.75f);
#		if defined(_DEBUG) && defined(DEBUG_LOGALLOCATEDMEMORY)
			Game::Engine->GetTextRenderer()->RenderString(concat("FPS: ")((int)Game::FPS)
				(", Allocations: ")(m_memstate.lSizes[1])(" [")(m_memstate.lCounts[1])(" blocks]").str(),
				Game::Engine->GetTextRenderer()->GetDefaultFontId(), DecalRenderingMode::ScreenSpace, 
				XMVectorSet(0.0f, -8.0f, 0.0, 0.0f), 10.0f, font_colour, font_colour, 0.6f);
#		else
			Game::Engine->GetTextRenderer()->RenderString(concat("FPS: ")((int)Game::FPS).str(), Game::Engine->GetTextRenderer()->GetDefaultFontId(), 
				DecalRenderingMode::ScreenSpace, XMVectorSet(0.0f, -8.0f, 0.0, 0.0f), 10.0f, font_colour, font_colour, TextAnchorPoint::TopLeft, 0.6f);
#		endif

	}
	
}

// Reads the current state of all input controllers
void RJMain::ReadUserInput(void)
{
	// Read the current keyboard state from the DirectInput device
	Game::Keyboard.Read();

	// Read new mouse information from the DirectInput device
	Game::Mouse.Read();
}

void RJMain::ProcessKeyboardInput(void)
{
	// Pass input to a different primary controller based upon the current user mode
	if (Game::Engine->GetCamera()->GetCameraState() == CameraClass::CameraState::DebugCamera)
	{
		// If we are in debug camera mode, intercept mouse commands for the debug camera
		AcceptDebugCameraKeyboardInput();
	}
	else /* if (Game::Engine->GetCamera()->GetCameraState() == CameraClass::CameraState::NormalCamera) */
	{
		// Otherwise, and by default, pass keyboard state to the current player to update the player control
		Game::CurrentPlayer->AcceptKeyboardInput(&Game::Keyboard);
	}

	// Get a reference to the keyboard state 
	BOOL *b = Game::Keyboard.GetKeys();

	// Accept game controls
	if (b[DIK_P])
	{
		TogglePause();
		Game::Keyboard.LockKey(DIK_P);
	}
	if (b[DIK_GRAVE])
	{
		D::UI->ToggleConsole();
		Game::Keyboard.LockKey(DIK_GRAVE);
	}

	// Debug rendering of GBuffer data
	if (b[DIK_F1])
	{
		Game::Engine->GetRenderDevice()->RepointBackbufferRenderTargetAttachment("none");
		Game::Keyboard.LockKey(DIK_F1);
	}
	if (b[DIK_F2])
	{
		Game::Engine->GetRenderDevice()->RepointBackbufferRenderTargetAttachment("diffuse");
		Game::Keyboard.LockKey(DIK_F2);
	}
	if (b[DIK_F3])
	{
		Game::Engine->GetRenderDevice()->RepointBackbufferRenderTargetAttachment("specular");
		Game::Keyboard.LockKey(DIK_F3);
	}
	if (b[DIK_F4])
	{
		Game::Engine->GetRenderDevice()->RepointBackbufferRenderTargetAttachment("normal");
		Game::Keyboard.LockKey(DIK_F4);
	}
	if (b[DIK_F5])
	{
		Game::Engine->GetRenderDevice()->RepointBackbufferRenderTargetAttachment("depth");
		Game::Keyboard.LockKey(DIK_F5);
	}

	// Debug resource loading
	if (b[DIK_F6])
	{
		Game::Engine->GetRenderDevice()->ReloadAllShaders();
		Game::Keyboard.LockKey(DIK_F6);
	}
	if (b[DIK_F7])
	{
		Game::Engine->GetRenderDevice()->ReloadAllMaterials();
		Game::Keyboard.LockKey(DIK_F7);
	}
	if (b[DIK_F8])
	{
		Model::ReloadAllModels();
		Game::Keyboard.LockKey(DIK_F8);
	}

	// Additional debug controls below this point
	if (b[DIK_U])
	{
		auto & objects = Game::CurrentPlayer->GetPlayerSystem()->Objects;
		for (auto & obj : objects)
		{
			if (obj()->GetObjectType() == iObject::ObjectType::LightSourceObject)
			{
				LightSource *ls = (LightSource*)obj();
				if (ls->GetLight().GetType() == LightType::Directional)
				{
					if (b[DIK_LALT])
					{
						ls->LightObject().Toggle();
						Game::Log << "Directional light is now " << (ls->GetLight().IsActive() ? "enabled" : "disabled") << "\n";
						break;
					}
					else if (b[DIK_LSHIFT])
					{
						ls->LightObject().ChangeIntensity(+0.1f);
						Game::Log << "Directional light intensity now has intensity " << ls->GetLight().GetIntensity() << "\n";
						break;
					}
					else if (b[DIK_LCONTROL])
					{
						ls->LightObject().ChangeIntensity(-0.1f);
						Game::Log << "Directional light intensity now has intensity " << ls->GetLight().GetIntensity() << "\n";
						break;
					}
				}
			}
		}

		Game::Keyboard.LockKey(DIK_U);
	}
	if (b[DIK_J])
	{
		if (!b[DIK_LSHIFT])	ActivateDebugPortalRenderingTest(Game::CurrentPlayer->GetActor());
		else				DeactivateDebugPortalRenderingTest();

		Game::Keyboard.LockKey(DIK_J);
	}

	if (b[DIK_5])
	{
		if (b[DIK_LSHIFT])
		{
			// System light
			const auto & objects = Game::CurrentPlayer->GetPlayerSystem()->Objects;
			auto it_end = objects.end();
			for (auto it = objects.begin(); it != it_end; ++it)
			{
				if ((*it)() && (*it)()->GetObjectType() == iObject::ObjectType::LightSourceObject)
				{
					LightSource *ls = (LightSource*)(*it)();
					ls->LightObject().SetIsActive(!ls->GetLight().IsActive());
				}
			}
		}
		else
		{
			// Actor-attached light
			iObject::AttachmentSet::iterator it_end = a1()->GetChildObjects().end();
			for (iObject::AttachmentSet::iterator it = a1()->GetChildObjects().begin(); it != it_end; ++it)
			{
				if ((*it).Child && (*it).Child->GetObjectType() == iObject::ObjectType::LightSourceObject)
				{
					Light & light = ((LightSource*)(*it).Child)->LightObject();
					light.SetIsActive(!light.IsActive());
					break;
				}
			}
		}

		Game::Keyboard.LockKey(DIK_5);
	}
	if (b[DIK_6])
	{
		D::UI->ActivateUIState("UI_MODELBUILDER");
		Game::Keyboard.LockKey(DIK_6);
	}
	if (b[DIK_7])
	{
		if (!b[DIK_LSHIFT])
		{
			cs()->RenderEnvironmentOxygenOverlay();
		}
		else
		{
			cs()->UpdateEnvironment();
			Game::Keyboard.LockKey(DIK_7);
		}
		//Game::Keyboard.LockKey(DIK_7);
	}

	if (b[DIK_8])
	{
		if (b[DIK_LSHIFT])			((CSPowerGeneratorTile*)cs()->GetTilesOfType(D::TileClass::PowerGenerator).at(0).value)->SetPowerOutputTargetPc(1.0f);
		else if (b[DIK_LCONTROL])	((CSPowerGeneratorTile*)cs()->GetTilesOfType(D::TileClass::PowerGenerator).at(0).value)->SetPowerOutputTargetPc(0.0f);
		else						cs()->RenderEnvironmentPowerOverlay();
	}

	if (b[DIK_9])
	{
		if (!b[DIK_LCONTROL] && !m_debug_ccdspheretest)
			m_debug_ccdobbtest = (!b[DIK_LSHIFT]);
	}
	if (b[DIK_0])
	{
		if (!b[DIK_LCONTROL] && !m_debug_ccdobbtest)
			m_debug_ccdspheretest = (!b[DIK_LSHIFT]);
	}

	if (b[DIK_MINUS])
	{
		if (b[DIK_LSHIFT])			cs()->OverridePortalBasedRenderingSupport(true);
		else if (b[DIK_LCONTROL])	cs()->RemoveOverrideOfPortalBasedRenderingSupport();

		else						Game::Engine->SetDebugPortalRenderingTarget(cs()->GetID());

		//Game::Keyboard.LockKey(DIK_MINUS);
	}

	if (b[DIK_EQUALS])
	{
		cs()->SetTargetSpeedPercentage(b[DIK_LCONTROL] ? 0.0f : 1.0f);

		Game::Keyboard.LockKey(DIK_EQUALS);
	}

	if (b[DIK_SEMICOLON]) {
		if (b[DIK_LSHIFT])
		{
			cs()->ForceRenderingOfInterior(false);
			cs()->GetSection(0)->Fade.FadeIn(2.5f);
			Game::Engine->GetCamera()->ReleaseCamera();
		}
		else
		{
			cs()->ForceRenderingOfInterior(true);
			cs()->GetSection(0)->Fade.FadeToAlpha(2.5f, 0.25f);
			Game::Engine->GetCamera()->ZoomToOverheadShipView(cs());
		}

		Game::Keyboard.LockKey(DIK_SEMICOLON);
	}
	if (b[DIK_PERIOD])
	{
		XMVECTOR pos = XMVector3TransformCoord(XMVectorSet(-(cs()->GetSizeF().x + 50.0f), -4.0f, -24.0f, 0.0f), cs()->GetWorldMatrix());
		XMVECTOR dir = XMVectorSet(200.0f, 7.0f, 50.0f, 0.0f);
		DebugFireBasicProjectile(BasicRay(pos, dir));
	}
	if (b[DIK_COMMA])
	{
		XMVECTOR pos = XMVector3TransformCoord(XMVectorSet(-(cs()->GetSizeF().x + 50.0f), -4.0f, -24.0f, 0.0f), cs()->GetWorldMatrix());
		XMVECTOR dir = XMVectorSet(200.0f, 7.0f, 50.0f, 0.0f);
		cs()->DebugRenderRayIntersectionTest(BasicRay(pos, dir));
	}

	if (b[DIK_H])
	{
		cs()->Fade.SetFadeAlpha(0.1f);
		cs()->Fade.FadeIn(1.0f);

		if (b[DIK_LSHIFT])
		{
			Game::Engine->GetOverlayRenderer()->RenderEnvironment3DOverlay(*(cs()), 0, [](iSpaceObjectEnvironment & env, int id)
			{
				bool dst = env.GetElementDirect(id).IsDestroyed();
				bool outer = env.GetElementDirect(id).IsOuterHullElement();
				if (dst) {
					if (outer) {
						// THIS SHOULD NOT HAPPEN
						return XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
					}
					else {
						// Destroyed
						return XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
					}
				}
				else {
					if (outer) {
						// Outer element
						return XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
					}
					else {
						// Regular element
						float v = env.GetElementDirect(id).GetHealth();
						return XMFLOAT4(1.0f - v, v, 0.0f, 0.75f);
					}
				}
			});
		}
		else
		{
			Game::Engine->GetOverlayRenderer()->RenderEnvironmentOverlay(*(cs()), 0, [](iSpaceObjectEnvironment & env, int id)
			{
				bool dst = env.GetElementDirect(id).IsDestroyed();
				bool outer = env.GetElementDirect(id).IsOuterHullElement();
				if (dst) {
					if (outer) {
						// THIS SHOULD NOT HAPPEN
						return XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
					}
					else {
						// Destroyed
						return XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
					}
				}
				else {
					if (outer) {
						// Outer element
						return XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
					}
					else {
						// Regular element
						float v = env.GetElementDirect(id).GetHealth();
						return XMFLOAT4(1.0f - v, v, 0.0f, 0.75f);
					}
				}
			});
		}
	}

	static float g_coord = 0.0f;
	g_coord += Game::TimeFactor;
	if (b[DIK_G])
	{
		if (b[DIK_LCONTROL])			Game::Log << LOG_DEBUG << ((LightSource*)Game::GetObjectByInstanceCode("clight"))->DebugString();
		else if (b[DIK_LSHIFT])			((LightSource*)Game::GetObjectByInstanceCode("clight"))->LightObject().Toggle();
		else							lt2()->LightObject().Toggle();

		Game::Keyboard.LockKey(DIK_G);
	}

	/*Game::Engine->GetDecalRenderer()->SetBaseColour(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	Game::Engine->GetDecalRenderer()->SetOutlineColour(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	Game::Engine->GetDecalRenderer()->SetTexture(Game::Engine->GetAssets().GetTexture("debug_texture"));
	Game::Engine->GetDecalRenderer()->RenderDecalScreen(XMVectorSet(g_coord, g_coord, 0.0f, 0.0f), XMVectorSet(300.0f, 300.0f, 1.0f, 0.0f));*/

	/*Game::Engine->GetTextRenderer()->RenderString("Hello World", Game::Engine->GetTextRenderer()->GetFontID("font_tahoma"), DecalRenderingMode::ScreenSpace,
		XMVectorSet(100.0f, 100.0f, 0.0f, 0.0f), 72.0f, XMQuaternionRotationAxis(FORWARD_VECTOR, g_coord),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), TextAnchorPoint::TopLeft, TextAnchorPoint::Centre, 0.5f, 1.0f / 4.0f);
	Game::Engine->GetTextRenderer()->RenderString("This is a test including some special characters \\!#'{}[]()*&%$", Game::Engine->GetTextRenderer()->GetFontID("font_tahoma"),
		DecalRenderingMode::ScreenSpace, XMVectorSet(100.0f, 400.0f, 0.0f, 0.0f), 18.0f, XMQuaternionRotationAxis(UP_VECTOR, g_coord),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), TextAnchorPoint::TopLeft, TextAnchorPoint::Centre, 0.5f, 1.0f / 4.0f);
	Game::Engine->GetTextRenderer()->RenderString("Hello World", Game::Engine->GetTextRenderer()->GetFontID("font_tahoma"), DecalRenderingMode::WorldSpace,
		XMVectorSet(300, 225, 200, 0), 72.0f, XMQuaternionRotationAxis(FORWARD_VECTOR, g_coord),
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), TextAnchorPoint::TopLeft, TextAnchorPoint::Centre, 0.5f, 1.0f / 4.0f);*/


	/*Game::Engine->GetDecalRenderer()->SetTexture(Game::Engine->GetAssets().GetTexture("debug_texture"));
	Game::Engine->GetDecalRenderer()->SetBaseColour(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	Game::Engine->GetDecalRenderer()->SetOutlineColour(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	Game::Engine->GetDecalRenderer()->SetOutlineWidthFactor(0.5f);
	Game::Engine->GetDecalRenderer()->SetSmoothingFactor(0.25f);
	Game::Engine->GetDecalRenderer()->RenderDecalWorld(XMVectorMultiplyAdd(Game::Engine->GetCamera()->GetCameraHeading(), XMVectorReplicate(1.0f),
		Game::Engine->GetCamera()->GetPosition()), Game::Engine->GetCamera()->GetOrientation(), XMVectorReplicate(10.0f));*/

	// TODO [textrender]: Update for new text rendering component
	/*static SentenceType **dbg_b_sentences = NULL;
	static const unsigned int dbg_b_text_limit = 32U;
	if (!b[DIK_B])
	{
		if (dbg_b_sentences)
			for (unsigned int i = 0; i < dbg_b_text_limit; ++i)
				dbg_b_sentences[i]->render = false;
	}
	else
	{
		static int dbg_z = 0;
		static BOOL ctrl_g_down = FALSE;
		if (b[DIK_LCONTROL] && !ctrl_g_down)
		{
			if (++dbg_z >= cs()->GetElementSize().z) dbg_z = 0;
		}
		ctrl_g_down = b[DIK_LCONTROL];

		if (dbg_b_sentences == NULL)
		{
			dbg_b_sentences = new SentenceType*[dbg_b_text_limit];
			for (int i = 0; i < dbg_b_text_limit; ++i)
			{
				dbg_b_sentences[i] = Game::Engine->GetTextManager()->CreateSentence(Font::DEFAULT_FONT_ID, 256);
				dbg_b_sentences[i]->render = false;
			}
		}

		cs()->Fade.SetFadeAlpha(0.1f);
		cs()->Fade.FadeIn(1.0f);
		cs()->SetWorldMomentum(NULL_VECTOR);
		cs()->PhysicsState.AngularVelocity = NULL_VECTOR;

		std::unordered_map<bitstring, BasicColourDefinition> legend;
		if (b[DIK_LSHIFT])
			cs()->DebugRenderElementState(dbg_z, legend);
		else
			cs()->DebugRenderElementState(legend);

		unsigned int index = 0U;
		INTVECTOR2 location = INTVECTOR2(20, 128);
		std::unordered_map<bitstring, BasicColourDefinition>::const_iterator it_end = legend.end();
		for (std::unordered_map<bitstring, BasicColourDefinition>::const_iterator it = legend.begin(); it != it_end; ++it)
		{
			if (index >= dbg_b_text_limit) break;
			std::string s = concat(it->second.name)(": ")(ComplexShipElement::DeterminePropertyStringDescription(it->first)).str();

			SentenceType *sentence = dbg_b_sentences[index++];
			if (!sentence) break;
			Game::Engine->GetTextManager()->UpdateSentence(sentence, s.c_str(), location.x, location.y, true, ONE_FLOAT4, 1.0f);

			location.y += ((int)sentence->sentenceheight + 4);
		}

		for (unsigned int i = index; i < dbg_b_text_limit; ++i) dbg_b_sentences[i]->render = false;

	}*/

	if (b[DIK_I]) {

		if (b[DIK_LSHIFT])
		{
			static int tmpi = 0;
			XMVECTOR pos = XMVector3TransformCoord(XMVectorSetX(NULL_VECTOR,
				(++tmpi % 2 != 0 ? -100.0f : +1000.0f)), ss()->GetWorldMatrix());
			XMFLOAT3 fpos;
			XMStoreFloat3(&fpos, pos);

			Game::Engine->GetAudioManager()->Create3DInstance("test1", s2()->GetPositionF(), 1.0f, 1.0f);
		}
		else
			Game::Engine->GetAudioManager()->CreateInstance("test1", 1.0f, 1.0f);
			

		Game::Keyboard.LockKey(DIK_I);
	}

	if (b[DIK_TAB]) {

		if (!Game::Keyboard.AltDown())		// Don't capture Alt-Tab
		{
			if (D::UI->GetActiveUIControllerCode() == UserInterface::UI_SHIPBUILDER)
			{
				D::UI->DeactivateAllUIComponents();
			}
			else
			{
				D::UI->DeactivateAllUIComponents();
				D::UI->ActivateUIState(UserInterface::UI_SHIPBUILDER);
				D::UI->ShipBuilderUI()->SetShip(cs());
			}

			Game::Keyboard.LockKey(DIK_TAB);
		}
	}
	if (b[DIK_2]) {
		if (b[DIK_LSHIFT])
		{
			Game::Console.ProcessRawCommand(GameConsoleCommand("enter_system_env AB01"));
		}
		else
		{
			//Game::Console.ProcessRawCommand(GameConsoleCommand("render_obb 1"));
			//Game::Console.ProcessRawCommand(GameConsoleCommand(concat("terrain_debug_render_mode solid").str()));
			//Game::Console.ProcessRawCommand(GameConsoleCommand(concat("render_terrainboxes ")(cs()->GetInstanceCode())(" true").str()));

			Game::Console.ProcessRawCommand(GameConsoleCommand("obj cs1 OverrideLocalGravity 9.8"));
			Game::Console.ProcessRawCommand(GameConsoleCommand(concat("enter_ship_env ")(cs()->GetInstanceCode()).str()));
			Game::CurrentPlayer->GetActor()->SetWorldMomentum(NULL_VECTOR);
			//Game::Console.ProcessRawCommand(GameConsoleCommand(concat("render_terrainboxes ")(cs()->GetInstanceCode())(" true").str()));

			((CSPowerGeneratorTile*)cs()->GetTilesOfType(D::TileClass::PowerGenerator).at(0).value)->SetPowerOutputTargetPc(1.0f);

			if (!cs()->GetTilesOfType(D::TileClass::Quarters).empty())
			{
				ComplexShipTile *tile = cs()->GetTilesOfType(D::TileClass::Quarters).at(0).value;
				XMVECTOR centre = tile->GetRelativePosition();

				Game::Log << LOG_DEBUG << "Terrain count = " << cs()->TerrainObjects.size() << "\n";

				DataObjectContinuousSwitch *t1 = (DataObjectContinuousSwitch*)DynamicTerrain::Create("switch_continuous_lever_vertical_01");
				t1->SetPosition(XMVectorSet(63.74f, 1.4f, 64.25f, 0.0f)); // XMVectorAdd(centre, XMVectorSet(-12.0f, 0.0f, -20.0f, 0.0f)));
				t1->SetOrientation(XMQuaternionRotationAxis(UP_VECTOR, PI));
				cs()->AddTerrainObject(static_cast<Terrain*>(t1));

				DataObjectContinuousSwitch *t2 = (DataObjectContinuousSwitch*)DynamicTerrain::Create("switch_continuous_lever_horizontal_01");
				t2->SetPosition(XMVectorSet(67.2f, 1.4f, 64.25f, 0.0f)); // XMVectorAdd(centre, XMVectorSet(-3.0f, 0.0f, -20.0f, 0.0f)));
				t2->SetOrientation(XMQuaternionRotationAxis(UP_VECTOR, PI));
				cs()->AddTerrainObject(static_cast<Terrain*>(t2));

				DataObjectEngineHeadingController *t_head = (DataObjectEngineHeadingController*)DynamicTerrain::Create("BasicEngineHeadingController");
				t_head->SetPosition(XMVectorAdd(t2->GetPosition(), XMVectorSet(0.0f, 0.5f, 15.0f, 0.0f)));
				cs()->AddTerrainObject(static_cast<Terrain*>(t_head));


				DynamicTerrain *target = NULL;
				Game::ID_TYPE engine_room = cs()->GetTilesOfType(D::TileClass::EngineRoom).at(0).value->GetID();
				for (auto * terrain : cs()->TerrainObjects)
				{
					if (terrain && terrain->GetParentTileID() == engine_room && terrain->IsDynamic())
					{
						target = terrain->ToDynamicTerrain();
						break;
					}
				}

				Game::Log << LOG_DEBUG << "Connecting thrust controller: " <<
					t1->ConnectPort(t1->OutputPort(), target, ((DataObjectEngineThrustController*)target)->Ports.PercentageThrustTargetInput()) << "\n";

				Game::Log << LOG_DEBUG << "Connecting heading controller: " <<
					t2->ConnectPort(t2->OutputPort(), t_head, ((DataObjectEngineHeadingController*)target)->Ports.TargetYawPercentageInput()) << "\n";


				Game::Log << LOG_DEBUG << "Terrain count = " << cs()->TerrainObjects.size() << "\n";
			}
		}

		Game::Keyboard.LockKey(DIK_2);
	}
	if (b[DIK_3]) {
		
		if (b[DIK_LSHIFT])
		{
			Game::Console.ProcessRawCommand(GameConsoleCommand("debug_camera 0"));
		}
		else
		{
			Game::Console.ProcessRawCommand(GameConsoleCommand("debug_camera 1"));
		}

		Game::Keyboard.LockKey(DIK_3);
	}
	if (b[DIK_4]) 
	{
		Game::Log << LOG_DEBUG << "Pos: " << Vector3ToString(Game::CurrentPlayer->GetActivePlayerObject()->GetPositionF()) << ", EnvPos: " <<
			Vector3ToString(Game::CurrentPlayer->GetActor()->GetEnvironmentPosition()) << "\n";

		Game::Keyboard.LockKey(DIK_4);
		
	}
}

void RJMain::ProcessMouseInput(void)
{
	// We will usually be accepting input in 'normal' camera mode, with the camera attached to the player
	if (Game::Engine->GetCamera()->GetCameraState() == CameraClass::CameraState::NormalCamera)
	{
		// Process the mouse input differently depending on the current player state
		if (Game::CurrentPlayer->GetState() == Player::StateType::ShipPilot)
		{
			// If piloting a ship then adjust orientation based on % movement from centre in the X and Y axis
			float x_mv = (float)(Game::Mouse.GetX() - Game::ScreenCentre.x) / (float)Game::ScreenCentre.x;
			float y_mv = (float)(Game::Mouse.GetY() - Game::ScreenCentre.y) / (float)Game::ScreenCentre.y;

			// Adjust calibration of the mouse range [-1.0 1.0] so that it does not necessarily correspond to the full screen bounds
			x_mv = min(max(x_mv * Game::C_MOUSE_FLIGHT_MULTIPLIER, -1.0f), 1.0f);
			y_mv = min(max(y_mv * Game::C_MOUSE_FLIGHT_MULTIPLIER, -1.0f), 1.0f);

			// Take different action depending on whether we are in mouse flight mode or not
			if (Game::MouseControlMode == MouseInputControlMode::MC_MOUSE_FLIGHT)
			{
				// Apply this mouse input to adjust the current ship vector
				Game::Logic::Move::UpdateShipMovementViaMouseFlightData(Game::CurrentPlayer->GetPlayerShip(), x_mv, y_mv);
			}
			else if (Game::MouseControlMode == MouseInputControlMode::MC_COCKPIT_CONTROL)
			{
				// Perform cockpit mode actions
			}
		}
		else
		{
			// Else if the player is on foot (or otherwise) we adjust the view vector based on mouse delta % of total
			float x_delta = ((float)Game::Mouse.GetXDelta() / (float)Game::ScreenWidth);
			float y_delta = ((float)Game::Mouse.GetYDelta() / (float)Game::ScreenHeight);

			// Update the player view rotation based on any mouse delta this frame
			Game::Logic::Move::UpdatePlayerViewViaMouseData(x_delta, y_delta);
		}
	}
	else if (Game::Engine->GetCamera()->GetCameraState() == CameraClass::CameraState::DebugCamera)
	{
		AcceptDebugCameraMouseInput();
	}
}

// Accepts keyboard input for the debug camera
void RJMain::AcceptDebugCameraKeyboardInput(void)
{
	// Get a reference to the pressed keys array
	BOOL *b = Game::Keyboard.GetKeys();

	// Determine any camera movement required
	bool move = false;
	XMFLOAT3 delta = NULL_FLOAT3;

	// Forward/back movement
	if (b[DIK_W])		{ delta.z = (Game::C_DEBUG_CAMERA_SPEED * Game::PersistentTimeFactor); move = true; }
	else if (b[DIK_S])	{ delta.z = (-Game::C_DEBUG_CAMERA_SPEED * Game::PersistentTimeFactor); move = true; }

	// Left/right movement
	if (b[DIK_A])		{ delta.x = (-Game::C_DEBUG_CAMERA_SPEED * Game::PersistentTimeFactor); move = true; }
	else if (b[DIK_D])	{ delta.x = (Game::C_DEBUG_CAMERA_SPEED * Game::PersistentTimeFactor); move = true; }

	// Up/down movement
	if (b[DIK_Q])		{ delta.y = (Game::C_DEBUG_CAMERA_SPEED * Game::PersistentTimeFactor); move = true; }
	else if (b[DIK_Z])	{ delta.y = (-Game::C_DEBUG_CAMERA_SPEED * Game::PersistentTimeFactor); move = true; }

	// If we have any delta movement required, transform into a translation in world space
	if (move)
	{
		// Scale movement speed if we have the fast- or slow-travel keys held
		if (b[DIK_LSHIFT] == TRUE || b[DIK_RSHIFT] == TRUE) delta = Float3MultiplyScalar(delta, Game::C_DEBUG_CAMERA_FAST_MOVE_MODIFIER);
		else if (b[DIK_LCONTROL] == TRUE || b[DIK_RCONTROL] == TRUE) delta = Float3MultiplyScalar(delta, Game::C_DEBUG_CAMERA_SLOW_MOVE_MODIFIER);

		// Build a transform matrix for the camera orientation
		XMMATRIX rot = XMMatrixRotationQuaternion(Game::Engine->GetCamera()->GetDebugCameraOrientation());

		// Transform the delta movement by this rotation, and apply it to the current debug camera position
		XMVECTOR vdelta = XMVector3TransformCoord(XMLoadFloat3(&delta), rot);
		Game::Engine->GetCamera()->AddDeltaDebugCameraPosition(vdelta);
	}

	// Apply camera roll if required
	if (b[DIK_E])		Game::Engine->GetCamera()->DebugCameraRoll(+Game::C_DEBUG_CAMERA_ROLL_SPEED * Game::PersistentTimeFactor);
	else if (b[DIK_R])	Game::Engine->GetCamera()->DebugCameraRoll(-Game::C_DEBUG_CAMERA_ROLL_SPEED * Game::PersistentTimeFactor);
}

// Accepts keyboard input for the debug camera
void RJMain::AcceptDebugCameraMouseInput(void)
{
	// In debug camera mode we simply want to rotate the camera based upon mouse delta location this frame
	float x_delta = ((float)Game::Mouse.GetXDelta() / (float)Game::ScreenWidth);
	float y_delta = ((float)Game::Mouse.GetYDelta() / (float)Game::ScreenHeight);

	// Scale delta mouse movement by the debug camera rotation speed, and by the time factor to ensure smooth movement
	x_delta *= (Game::C_DEBUG_CAMERA_TURN_SPEED * Game::PersistentTimeFactor);
	y_delta *= (Game::C_DEBUG_CAMERA_TURN_SPEED * Game::PersistentTimeFactor);

	// Apply these delta pitch and yaw values to the debug camera
	if (fabs(x_delta) > Game::C_EPSILON) Game::Engine->GetCamera()->DebugCameraYaw(x_delta);
	if (fabs(y_delta) > Game::C_EPSILON) Game::Engine->GetCamera()->DebugCameraPitch(y_delta);
}

// Update window size details based on these parameters, recalculating for windowed mode as required
void RJMain::UpdateWindowSizeParameters(int screenWidth, int screenHeight, bool fullscreen)
{
	// Store parameters
	Game::FullScreen = fullscreen;

	// Take different action depending on whether we want windowed mode or not
	if (fullscreen)
	{
		// Simply store these window parameters; no difference between window and screen sizes since we are in fullscreen mode
		Game::ScreenWidth = Game::FullWindowSize.x = screenWidth;
		Game::ScreenHeight = Game::FullWindowSize.y = screenHeight;
		Game::ScreenCentre = INTVECTOR2(Game::ScreenWidth / 2, Game::ScreenHeight / 2);
		Game::WindowPosition = NULL_INTVECTOR2;
	}
	else
	{
		// Otherwise, in windowed mode we need to adjust the desired window size to include a margin for non-client areas.  This ensures that
		// the client area matches the desired screen width/height

		// Store the desired client size as normal
		Game::ScreenWidth = screenWidth;
		Game::ScreenHeight = screenHeight;
		Game::ScreenCentre = INTVECTOR2(Game::ScreenWidth / 2, Game::ScreenHeight / 2);

		// Adjust the window size to get the size of the full application window, including non-client areas
		RECT win = { 0, 0, screenWidth, screenHeight };
		AdjustWindowRectEx(&win, m_wndstyle, FALSE, m_wndstyleex);
		Game::FullWindowSize = INTVECTOR2(win.right - win.left, win.bottom - win.top);

		// Window should begin in the top-left of the screen
		Game::WindowPosition = NULL_INTVECTOR2;
	}

	// Notify the game engine of this change, if it is initialised at this point
	if (Game::Engine) Game::Engine->WindowResized();
}


HWND RJMain::CreateMainWindow(HINSTANCE hInstance, WNDPROC wndproc)
{
	//
	// Create the main application window.
	//

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = TEXT(APPLICATION_WINDOW_CLASSNAME);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		int result = GetLastError();
		::MessageBox(0, TEXT("RegisterClassEx() - FAILED"), 0, 0);
		return false;
	}

	HWND hwnd = 0;
	hwnd = ::CreateWindowEx(m_wndstyleex, TEXT(APPLICATION_WINDOW_CLASSNAME), TEXT(APPLICATION_WINDOW_WINDOWNAME),
		m_wndstyle, 250, 0, Game::FullWindowSize.x, Game::FullWindowSize.y,
		0 /*parent hwnd*/, 0 /* menu */, hInstance, 0 /*extra*/);

	if (!hwnd)
	{
		int result = GetLastError();
		::MessageBox(0, TEXT("CreateWindowEx() - FAILED"), 0, 0);
		return false;
	}

	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	// Return a handle to the new window
	return hwnd;
}

// Primary quit method for the application
void RJMain::Quit(void)
{
	// Post a WM_DESTROY message.  This will be caught by the WinApi callback and invoke all relevant game termination 
	// functions before correctly exiting
	::DestroyWindow(m_hwnd);
}


Result RJMain::Initialise(HINSTANCE hinstance, WNDPROC wndproc)
{
	Result res;

	// Store the HINSTANCE and window procedures provided, for initialisation of the main application window
	m_hinstance = hinstance;
	m_wndproc = wndproc;

	// Set COM initialisation method for the current thread
	HRESULT com_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(com_result)) return ErrorCodes::CouldNotSetCOMInitialisationMethod;

	// Record the name of and path to the current executable (TODO: Windows only)
	RetrieveExecutableData();

	// Initialise all component pointers to NULL so we can accurately set up and wind down the component set
	InitialiseComponentPointers();

	// Initialise the logging component first so we can output details of the initialisation.  Enable flushing after
	// every operation during the initialisation phase, to ensure that all data is output before any crash
	InitialiseLogging();
	Game::Log.EnableFlushAfterEveryOperation();

	// Initialise the high-resolution timer 
	Timers::Initialise();

	// Initialise the profiler (if it is active)
	Profiler::InitialiseProfiler();

	// Initialise the central object registers
	Game::InitialiseObjectRegisters();

	// Load player config before initialising the application
	res = LoadPlayerConfig();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Could not load core configuration data [")((int)res)("]").str();
		Game::Log << LOG_ERROR << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Validate that the game data directory and all other dependencies exist before attempting to load data
	res = InitialiseGameDataDependencies();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Game data dependencies could not be initialised; check config.xml [")((int)res)("]").str();
		Game::Log << LOG_ERROR << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Create a new game window
	res = InitialiseWindow();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Could not initialise main window [")((int)res)("]").str();
		Game::Log << LOG_ERROR << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Attempt to initialise the game engine
	Game::Engine = new CoreEngine();
	res = Game::Engine->InitialiseGameEngine(m_hwnd);
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Could not initialise core game engine [")((int)res)("]").str();
		Game::Log << LOG_ERROR << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Initialise DirectInput
	res = InitialiseDirectInput();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: DirectInput initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_ERROR << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Math functions
	InitialiseMathFunctions();
	Game::Log << LOG_INFO << "Math functions initialised\n";

	// Run all static initialisation logic
	res = InitialiseStaticData();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Static data initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_ERROR << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Initialise object search capabilities, and also disable search caching during initialisation while objects are being created in large numbers
	Game::ObjectSearchManager::Initialise();
	Game::ObjectSearchManager::DisableSearchCache();

	// Initialise all standard modifiers
	StandardModifiers::InitialiseStandardModifiers();

	// Initialise the universe
	res = InitialiseUniverse();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Universe initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_INFO << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Initialise key components of the user interface
	res = InitialiseUserInterface();
	if (res != ErrorCodes::NoError)
	{
		std::string errorstring = concat("Fatal Error: User interface initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_INFO << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Initialise the data structures & overlays used to manage objects in the world
	InitialiseCoreDataStructures();

	// Initialise ship-related data
	InitialiseShipData();

	// Initialise actor-related data
	InitialiseActorData();

	// Now load all game data
	res = LoadAllGameData();
	if (res != ErrorCodes::NoError) {
		Game::Log << LOG_ERROR << "*** Error while loading game data file, hierarchy load terminated\n";
	}

	// Perform post-load initialisation of any data just loaded from game files
	InitialiseLoadedGameData();

	// All game data has now been loaded & initialised
	Game::GameDataLoaded = true;

	// Allow the core engine to perform any post-data-load activities, e.g. retrieving models that have now been loaded
	Game::Engine->PerformPostDataLoadInitialisation();

	// Initialise the simulation state manager
	InitialiseStateManager();

	// Initialise the active player object
	InitialisePlayer();

	// *** DEBUG ***
	__CreateDebugScenario();

	// Initialise the primary region objects
	res = InitialiseRegions();
	if (res != ErrorCodes::NoError)
	{
		std::string errorstring = concat("Fatal Error: Region initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_ERROR << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Enable the object manager cache now that objects have been created during initialisation
	Game::ObjectSearchManager::EnableSearchCache();

	// Return success if we have completed all initialisation functions
	Game::Log << LOG_INFO << "Initialisation complete\n\n";
	Game::Log.FlushAllStreams();
	Game::Log.DisableFlushAfterEveryOperation();

	// Bring the application window into focus now that we have initialised
	FocusApplication();

	return ErrorCodes::NoError;
}

// Initialise all game data dependencies and make sure that all required dependencies are available
Result RJMain::InitialiseGameDataDependencies(void)
{
	// Attempt to resolve the provided data path to its full absolute directory string
	std::wstring wdata = ConvertStringToWString(D::DATA_S);
	wchar_t *wcfull = new wchar_t[4096];
	if (0 == GetFullPathNameW(wdata.c_str(), 4096, wcfull, NULL))
	{
		return ErrorCodes::CouldNotDetermineAbsoluteDataPath;
	}
	else
	{
		// Retrieve full directory name and store it
		std::wstring wfull = std::wstring(wcfull);
		D::DATA_S = ConvertWStringToString(wfull);
		D::DATA = D::DATA_S.c_str();

		// Make sure the directory is valid
		if (!FileSystem::DirectoryExists(D::DATA))
		{
			return ErrorCodes::AbsoluteDataPathIsNotValid;
		}
	}
	delete[] wcfull;

	// Derive other required application paths
	// TODO: Reverted back to main data directory for now; copy to ImageContent to be set up via VStudio image pipeline
	//D::IMAGE_DATA_S = std::string(Game::ExePath + "\\Data\\ImageContent\\Data\\");
	D::IMAGE_DATA_S =  std::string("C:\\Users\\robje\\Documents\\Visual Studio 2017\\Projects\\RJ\\RJ\\Data\\");
	D::IMAGE_DATA = D::IMAGE_DATA_S.c_str();

	// Make sure paths are valid
	if (!FileSystem::DirectoryExists(D::IMAGE_DATA))
	{
		return ErrorCodes::ImageResourcePathIsNotValid;
	}

	// Return success
	Game::Log << LOG_INFO << "Game data dependencies initialised successfully\n";
	return ErrorCodes::NoError;
}

void RJMain::InitialiseComponentPointers()
{
	// Initialise all component pointers to NULL.  This is important so that the setup and wind-down processes
	// deal with each component in the sequence correctly
	Game::Engine = NULL;
	D::UI = NULL;
}

// Initialises the main application window
Result RJMain::InitialiseWindow()
{
	// Derive window parameters based on the desired screen width & height (Game::Screen{Width|Height})
	UpdateWindowSizeParameters(Game::ScreenWidth, Game::ScreenHeight, Game::FullScreen);

	// Attempt to create the main window with these parameters
	m_hwnd = CreateMainWindow(m_hinstance, m_wndproc);
	if (!m_hwnd) return ErrorCodes::CouldNotInitialiseWindow;

	// Return success
	Game::Log << LOG_INFO << "Game window initialised successfully\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseDirectInput()
{
	// Attempt to initialise DI, and return any error encountered
	Result result = GameInputSystem::Initialise(m_hinstance, m_hwnd, m_di, &Game::Keyboard, &Game::Mouse);
	if (result != ErrorCodes::NoError) return result;

	// Return success
	Game::Log << LOG_INFO << "DirectInput initialisation complete\n";
	return ErrorCodes::NoError;
}

// Run all static data initialisation methods upon initialisation
Result RJMain::InitialiseStaticData(void)
{
	// Run each initialisation method in turn
	ComplexShipElement::InitialiseStaticData();

	// Return success
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseRegions(void)
{
	Result res;

	// Initialise the immediate region, centred around the player
	D::Regions::Immediate = new ImmediateRegion();
	res = D::Regions::Immediate->Initialise(								// Pointer to the D3D device
		"dust_particle", 													// Texture to be mapped onto all dust particles
		Game::CurrentPlayer->GetPlayerShip()->GetPosition(),				// Starting centre point = player location
		XMVectorSetW(XMVectorReplicate(Game::C_IREGION_MIN), 0.0f),			// Minimum region bounds
		XMVectorSetW(XMVectorReplicate(Game::C_IREGION_BOUNDS), 0.0f),		// Maximum region bounds
		XMVectorSetW(XMVectorReplicate(Game::C_IREGION_THRESHOLD), 0.0f)	// Region update threshold
		);

	// If initialisation of the immediate region failed then return with an error here
	if (res != ErrorCodes::NoError) return res;

	// Initialise the system region, and set a default texture for now
	// TODO: THIS SHOULD COME FROM THE SYSTEM DATA FILE WHEN IT IS LOADED
	D::Regions::System = new SystemRegion();
	res = D::Regions::System->Initialise();
	if (res != ErrorCodes::NoError) return res;

	// DEBUG: SET BACKDROP
	XMFLOAT2 texsize = XMFLOAT2(2048.0f, 1024.0f);
	if (true) D::Regions::System->SetBackdropTexture("omega_backdrop", texsize);


	// Return success if we have reached the end
	Game::Log << LOG_INFO << "Region initialisation completed\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseUniverse(void)
{
	// Create a new universe object and initialise it
	Game::Universe = new GameUniverse();
	Result res = Game::Universe->InitialiseUniverse();

	// If universe initialisation failed then return an error now
	if (res != ErrorCodes::NoError) return res;

	// Return success
	Game::Log << LOG_INFO << "Universe initialised successfully\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseUserInterface(void)
{
	// Create a new UI manager instance
	D::UI = new UserInterface();
	Result res = D::UI->Initialise();

	// If UI initialisation failed then return an error now
	if (res != ErrorCodes::NoError) return res;

	// Enable the display of debug text, for example the FPS counter and flight info
	D::UI->SetFPSCounterDisplay(true);
	D::UI->SetDebugFlightInfoDisplay(true);

	// Return success
	Game::Log << LOG_INFO << "User interface initialisation complete\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialisePlayer(void)
{
	// Create a new player instance for the current player
	Game::CurrentPlayer = new Player();

	// Create a new human actor and assign it as the player character
	ActorBase *actorbase = D::Actors.Get("human_soldier_basic");
	if (!actorbase) return ErrorCodes::CouldNotCreatePlayerFromUndefinedActorBase;
	Actor *actor = actorbase->CreateInstance();
	if (!actor) return ErrorCodes::CouldNotCreatePlayerActor;

	// Set the player actor to full simulation now so that we can correctly make a reference to it during initialisation
	actor->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
	Game::CurrentPlayer->SetActor(actor);
	if (Game::CurrentPlayer->GetActor() == NULL) return ErrorCodes::CouldNotCreatePlayerActor;
	Game::CurrentPlayer->GetActor()->SetName("Player actor");
	Game::CurrentPlayer->GetActor()->OverrideInstanceCode("a1");
	Game::CurrentPlayer->GetActor()->MoveIntoEnvironment(cs());

	// Temporary: create a player ship.  Also assign to an application variable so we can reference it in global scope
	Ship *ship = InitialiseTemporaryPlayerShip();
	ss = ship;

	// Place the player in a default (already created) ship and the current system
	Game::CurrentPlayer->SetPlayerShip(ship);
	Game::CurrentPlayer->EnterEnvironment(Game::Universe->GetSystem("AB01"));

	// Return success
	Game::Log << LOG_INFO << "Player initialisation completed\n";
	return ErrorCodes::NoError;
}

// Initialise the data structures & overlays used to manage objects in the world
Result RJMain::InitialiseCoreDataStructures(void)
{
	// Zero the global memstate structure, which may be used for tracking allocations depending on game settings
#	if defined(_DEBUG) && defined(DEBUG_LOGALLOCATEDMEMORY)
		memset(&m_memstate, 0, sizeof(m_memstate));
#	endif

	// Schedule the central tree-pruning component to run on a regular basis.  It will maintain the tree for 
	// each system that is added to the universe
	Game::Scheduler.ScheduleInfrequentUpdates(&(Game::TreePruner), 2000);

	// We have initialised all core data structures so return success
	Game::Log << LOG_INFO << "Core data structured initialised\n";
	return ErrorCodes::NoError;
}

// Initialise the data structures & overlays used to manage objects in the world
Result RJMain::InitialiseStateManager(void)
{
	// Add the state manager to the central scheduler
	CentralScheduler::ID_TYPE id = Game::Scheduler.ScheduleFrequentUpdates(
		(ScheduledObject*)(&Game::StateManager),
		Game::C_SIMULATION_STATE_MANAGER_UPDATE_INTERVAL);

	// Return success
	Game::Log << LOG_INFO << "Simulation state manager initialised\n";
	return ErrorCodes::NoError;
}

// Initialise the central logging components
Result RJMain::InitialiseLogging(void)
{
	// Logging is initialised upon construction of the log component, so simply check here that it was successful
	if (!Game::Log.LoggingActive()) return ErrorCodes::CouldNotInitialiseCentralLoggingComponent;

	// Schedule a periodic update for the log, to ensure it is always flushed at regular intervals
	Game::Scheduler.ScheduleInfrequentUpdates(&Game::Log, Game::C_LOG_FLUSH_INTERVAL);

	// Output a log message that the component was successfully initialised
	Game::Log	<< "------------------------------------------------------------\n"
				<< "Initialising main application - " << GetLocalDateTimeString() << " [" << (unsigned int)timeGetTime() << "]\n"
				<< "------------------------------------------------------------\n\n";

	// Return success
	return ErrorCodes::NoError;
}


Result RJMain::InitialiseShipData(void)
{
	// Nothing to do at this point

	// Return success
	Game::Log << LOG_INFO << "Supporting ship data initialised\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseActorData(void)
{
	// Call the method for actor attribute-related setup
	ActorAttributeGeneration::InitialiseActorAttributeData();

	// Return success
	Game::Log << LOG_INFO << "Supporting actor data initialised\n";
	return ErrorCodes::NoError;
}

void RJMain::UpdateRegions(void)
{
	// Re-centre the immediate region on the player
	D::Regions::Immediate->RegionCentreMoved(Game::CurrentPlayer->GetPosition());
}

void RJMain::FocusApplication(void)
{
	// Bring the application window into focus, if it isn't already
	// TODO: Windows only
	SetFocus(m_hwnd);
	SetActiveWindow(m_hwnd);
	SetForegroundWindow(m_hwnd);
}

// Terminate all key game data structures (e.g. the space object Octree)
void RJMain::TerminateCoreDataStructures(void)
{
	// Move recursively down the tree, returning each node to the memory pool as we go.  The method to deallocate memory
	// pools must therefore be one of the final shutdown methods to ensure it incorporates items returned in other shutdown methods
	/*Game::SpatialPartitioningTree->Shutdown();
	Game::SpatialPartitioningTree = NULL;*/
}

// Shutdown all object search components and any associated cache data
void RJMain::TerminateObjectSearchManager(void)
{
	// Shutdown all object search components and any associated cache data; must be performed 
	// before the central object registers themselves are destructed (due to cache references into the registers)
	Game::ObjectSearchManager::Shutdown();
}

// Terminate the simulation state manager and release any resources
void RJMain::TerminateStateManager(void)
{
	// Nothing to do at this point
}

// Terminate the central logging components and attempt to flush any pending data
void RJMain::TerminateLogging(void)
{
	// Attempt to flush and close all the streams here.  This will also automatically happen upon
	// destruction, but this gives us more chance of flushing everything in case of a crash
	Game::Log.ShutdownLogging();
}

void RJMain::TerminateRegions(void)
{
	// Terminate the immediate region
	if (D::Regions::Immediate)
	{
		D::Regions::Immediate->Terminate();
		delete D::Regions::Immediate;
		D::Regions::Immediate = NULL;
	}

	// Terminate the system region
	if (D::Regions::System)
	{
		D::Regions::System->Terminate();
		delete D::Regions::System;
		D::Regions::System = NULL;
	}
}

void RJMain::TerminateUserInterface(void)
{
	// Terminate the user interface object & release resources
	if (D::UI)
	{
		D::UI->Terminate();
		delete D::UI;
		D::UI = NULL;
	}
}

void RJMain::TerminateUniverse(void)
{
	// Terminate the universe and all systems within it
	if (Game::Universe)
	{
		Game::Universe->TerminateUniverse();
		delete Game::Universe;
		Game::Universe = NULL;
	}
}

// Terminate any memory-pooled objects we are maintaining
void RJMain::TerminateMemoryPools(void)
{
	// Run the shutdown function for each central static memory pool in turn.  Method should be called
	// for each templated class in use by the application
	Octree<iObject*>::ShutdownMemoryPool();
}

// Load player config before initialising the application
Result RJMain::LoadPlayerConfig(void)
{
	// Record a log event when we begin loading game data files
	Game::Log << LOG_INFO << "Loading player configuration\n";

	// Attempt to load the main configuration file
	Result res = IO::Data::LoadConfigFile("./Config.xml");
	if (res != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "*** ERROR during load of configuration data [" << res << "]\n";
		return res;
	}

	// We have reached this point without a fatal error
	Game::Log << LOG_INFO << "All player configuration loaded successfully\n";
	return ErrorCodes::NoError;
}

Result RJMain::LoadAllGameData()
{
	Result res = ErrorCodes::NoError;

	// Record a log event when we begin loading game data files
	Game::Log << "\n" << LOG_INFO << "Loading game data hierarchy...\n";

	// Start at the primary index file that should already exist and work recursively outwards
	res = IO::Data::LoadGameDataFile("\\GameData.xml");
	if (res != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "*** ERROR during load of game data hierarchy [" << res << "]\n\n";
		return res;
	}

	// We have reached this point without a fatal error
	Game::Log << LOG_INFO << "All game data files loaded successfully\n\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseLoadedGameData(void)
{
	Result res; bool failures = false;

	// Load all audio resources.  TODO: in future, we may not want to load everything on startup
	Game::Log << LOG_INFO << "Beginning load of all audio resource data\n";
	res = Game::Engine->GetAudioManager()->LoadAllAudioResources();
	Game::Log << LOG_INFO << (res == ErrorCodes::NoError ? "Load of all audio resource data completed\n" : "ERRORS encountered during load of audio resource data\n");

	// Resources: Run all post-load creation of the interdependencies between resources & their ingredients
	res = IO::Data::PostProcessResources();
	if (res != ErrorCodes::NoError) failures = true;
	Game::Log << LOG_INFO << (res == ErrorCodes::NoError ? "Post-processing of all resource data completed\n" : "ERRORS encountered during post-processing of resource data\n");

	// Ship tiles: Perform post-load initialisation of tiles and their construction requirements
	res = IO::Data::PostProcessComplexShipTileData();
	if (res != ErrorCodes::NoError) failures = true;
	Game::Log << LOG_INFO << (res == ErrorCodes::NoError ? "Post-processing of all tile data completed\n" : "ERRORS encountered during post-processing of tile data\n");

	// Universe: Process all systems in the universe, with data already loaded as part of LoadAllGameData
	res = Game::Universe->ProcessLoadedSystems(Game::Engine->GetDevice());
	if (res != ErrorCodes::NoError) failures = true;
	Game::Log << LOG_INFO << (res == ErrorCodes::NoError ? "Post-processing of all system data completed\n" : "ERRORS encountered during post-processing of system data\n");

	// Equipment: Process all equipment to run post-load initialisation
	Equipment::InitialiseLoadedEquipmentData();
	res = ErrorCodes::NoError;
	Game::Log << LOG_INFO << (res == ErrorCodes::NoError ? "Post-processing of all equipment data completed\n" : "ERRORS encountered during post-processing of equipment data\n");

	// User interface: Build all UI layouts using the data loaded from game data files
	res = D::UI->BuildUILayouts();
	if (res != ErrorCodes::NoError) failures = true;
	Game::Log << LOG_INFO << (res == ErrorCodes::NoError ? "Post-processing of all UI layouts completed\n" : "ERRORS encountered during post-processing UI layout data\n");

	// Factions: initialise all faction data once game files have been processed
	res = Game::FactionManager.Initialise();
	if (res != ErrorCodes::NoError) failures = true;
	Game::Log << LOG_INFO << (res == ErrorCodes::NoError ? "Post-processing of all faction data completed\n" : "ERRORS encountered during post-processing of faction data\n");

	// Report any failures
	if (failures) Game::Log << LOG_ERROR << "Errors encountered during post-processing of loaded game data\n";

	// Return success
	return ErrorCodes::NoError;
}

void RJMain::DebugRenderSpaceCollisionBoxes(void)
{
	iSpaceObject *object;
	std::vector<iObject*> objects; int count = 0;

	// Find all active space objects around the player; take a different approach depending on whether the player is in a ship or on foot
	if (Game::CurrentPlayer->GetState() == Player::StateType::OnFoot)
	{
		// Player is on foot, so use a proximity test to the object currently considered their parent environment
		if (Game::CurrentPlayer->GetParentEnvironment() == NULL) return;
		count = 1 + Game::Search<iObject>().GetAllObjectsWithinDistance(	Game::CurrentPlayer->GetParentEnvironment(), 10000.0f, objects, 
																				Game::ObjectSearchOptions::OnlyCollidingObjects);

		// Also include the parent ship environmment(hence why we +1 to the count above)
		objects.push_back(Game::CurrentPlayer->GetParentEnvironment());
	}
	else
	{
		// Player is in a spaceobject ship, so use the proximity test on their ship
		if (Game::CurrentPlayer->GetPlayerShip() == NULL) return;
		count = 1 + Game::Search<iObject>().GetAllObjectsWithinDistance(	Game::CurrentPlayer->GetPlayerShip(), 10000.0f, objects,
																				Game::ObjectSearchOptions::OnlyCollidingObjects);

		// Also include the player ship (hence why we +1 to the count above)
		objects.push_back(Game::CurrentPlayer->GetPlayerShip());
	}

	// Iterate through the active objects
	XMFLOAT3 pos; float radius;
	std::vector<iObject*>::iterator it_end = objects.end();
	for (std::vector<iObject*>::iterator it = objects.begin(); it != it_end; ++it)
	{
		object = (iSpaceObject*)(*it);

		/* If we want to render collision sphere extents */
		if (false)
		{
			XMStoreFloat3(&pos, object->GetPosition());
			radius = object->GetCollisionSphereRadius();

			// Render the collision sphere extent in each axis
			Game::Engine->GetOverlayRenderer()->RenderLine(	XMVectorSet(pos.x - radius, pos.y, pos.z, 0.0f), XMVectorSet(pos.x + radius, pos.y, pos.z, 0.0f), 
															OverlayRenderer::RenderColour::RC_Green, 1.0f, radius*2.0f);
			Game::Engine->GetOverlayRenderer()->RenderLine(XMVectorSet(pos.x, pos.y - radius, pos.z, 0.0f), XMVectorSet(pos.x, pos.y + radius, pos.z, 0.0f), 
															OverlayRenderer::RenderColour::RC_Green, 1.0f, radius*2.0f);
			Game::Engine->GetOverlayRenderer()->RenderLine(XMVectorSet(pos.x, pos.y, pos.z - radius, 0.0f), XMVectorSet(pos.x, pos.y, pos.z + radius, 0.0f), 
															OverlayRenderer::RenderColour::RC_Green, 1.0f, radius*2.0f);
		}

		/* If we want to render the OBBs themselves */
		if (true)
		{
			// Render the oriented bounding box(es) used for narrowphase collision detection, if applicable for this object
			if (object->GetCollisionMode() == Game::CollisionMode::FullCollision)
			{
				Game::Engine->GetOverlayRenderer()->RenderOBB(object->CollisionOBB, true, OverlayRenderer::RenderColour::RC_LightBlue, 0.4f);
			}
		}
	}
}


// Default destructor
RJMain::~RJMain(void)
{
}

// Run the continuous collision detection (CCD) test in the main system environment
void RJMain::DebugCCDSphereTest(void)
{
	static AXMVECTOR sspos = XMVectorSet(50.0f, 0.0f, 50.0f, 0.0f);
	static AXMVECTOR ssorient = ID_QUATERNION;
	static const float movespeed = 25.0f;
	static const float turnspeed = 1.0f;
	BOOL *k = Game::Keyboard.GetKeys();

	if (k[DIK_LCONTROL] && k[DIK_0])
	{
		sspos = XMVectorSet(50.0f, 0.0f, 50.0f, 0.0f);
		ss()->SetOrientation(ID_QUATERNION);
	}

	if (Game::Engine->GetCamera()->GetCameraState() != CameraClass::CameraState::DebugCamera || 
		k[DIK_LCONTROL] && k[DIK_0])
	{
		Game::Console.ProcessRawCommand(GameConsoleCommand("debug_camera 1"));
		Game::Engine->GetCamera()->SetDebugCameraPosition(XMVectorSet(0.0f, 200.0f, 0.0f, 0.0f));
		Game::Engine->GetCamera()->SetDebugCameraOrientation(XMQuaternionRotationNormal(RIGHT_VECTOR, PIOVER2));
	}

	ss()->SetPosition(sspos);
	ss()->SetOrientation(ssorient);
	ss()->SetLocalMomentum(XMVectorSet(0.0f, 0.0f, 200.0f, 0.0f));
	ss()->RefreshPositionImmediate();
	ss()->CollisionOBB.UpdateFromObject(*ss());
	s2()->SetPosition(NULL_VECTOR);
	s2()->SetOrientation(ID_QUATERNION);
	s2()->SetLocalMomentum(XMVectorSet(0.0f, 0.0f, 200.0f, 0.0f));
	s2()->RefreshPositionImmediate();
	s2()->RefreshPositionImmediate();
	s2()->CollisionOBB.UpdateFromObject(*s2());

	float restore_timefactor = Game::TimeFactor;
	Game::TimeFactor = 1.0f;

	if (!k[DIK_LCONTROL])
	{
		if (k[DIK_LEFTARROW]) sspos = XMVectorSubtract(sspos, XMVectorSet(movespeed * restore_timefactor, 0.0f, 0.0f, 0.0f));
		if (k[DIK_RIGHTARROW]) sspos = XMVectorAdd(sspos, XMVectorSet(movespeed * restore_timefactor, 0.0f, 0.0f, 0.0f));
		if (k[DIK_UPARROW]) sspos = XMVectorAdd(sspos, XMVectorSet(0.0f, 0.0f, movespeed * restore_timefactor, 0.0f));
		if (k[DIK_DOWNARROW]) sspos = XMVectorSubtract(sspos, XMVectorSet(0.0f, 0.0f, movespeed * restore_timefactor, 0.0f));
	}
	else
	{
		if (k[DIK_LEFTARROW])  {
			XMVECTOR delta = XMQuaternionRotationNormal(UP_VECTOR, -turnspeed * restore_timefactor);
			ss()->SetOrientation(XMQuaternionMultiply(ssorient, delta));
		}
		if (k[DIK_RIGHTARROW])  {
			XMVECTOR delta = XMQuaternionRotationNormal(UP_VECTOR, turnspeed * restore_timefactor);
			ss()->SetOrientation(XMQuaternionMultiply(ssorient, delta));
		}
	}

	bool b = Game::PhysicsEngine.TestContinuousSphereCollision(ss(), s2());
	GamePhysicsEngine::CollisionDetectionResult collision = Game::PhysicsEngine.LastCollisionTest();
	OverlayRenderer::RenderColour col = (b ? OverlayRenderer::RenderColour::RC_Red : OverlayRenderer::RenderColour::RC_Green);

	XMMATRIX world; XMVECTOR move;
	ss()->CollisionOBB.GenerateWorldMatrix(world);
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, col, ss()->GetSize());
	s2()->CollisionOBB.GenerateWorldMatrix(world);
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, col, s2()->GetSize());

	move = XMVectorNegate(collision.ContinuousTestResult.InterimCalculations.wm0);
	world = XMMatrixMultiply(ss()->GetWorldMatrix(), XMMatrixTranslationFromVector(move));
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, OverlayRenderer::RenderColour::RC_LightBlue, ss()->GetSize());

	move = XMVectorNegate(collision.ContinuousTestResult.InterimCalculations.wm1);
	world = XMMatrixMultiply(s2()->GetWorldMatrix(), XMMatrixTranslationFromVector(move));
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, OverlayRenderer::RenderColour::RC_LightBlue, s2()->GetSize());

	Game::Engine->GetOverlayRenderer()->RenderLine(collision.ContinuousTestResult.InterimCalculations.pos0,
		ss()->GetPosition(), col, 5.0f, -1.0f);
	Game::Engine->GetOverlayRenderer()->RenderLine(collision.ContinuousTestResult.InterimCalculations.pos1,
		s2()->GetPosition(), col, 5.0f, -1.0f);

	if (b)
	{
		world = XMMatrixTranslationFromVector(collision.ContinuousTestResult.ContactPoint);
		Game::Engine->GetOverlayRenderer()->RenderBox(world, OverlayRenderer::RenderColour::RC_LightBlue, 3.0f, 10.0f);
	}

	ss()->SetWorldMomentum(NULL_VECTOR);
	s2()->SetWorldMomentum(NULL_VECTOR);

	Game::TimeFactor = restore_timefactor;
}

// Run the continuous collision detection (CCD) test between specific objects in the main system environment
void RJMain::DebugCCDOBBTest(void)
{
	static AXMVECTOR sspos = XMVectorSet(50.0f, 0.0f, 50.0f, 0.0f);
	static AXMVECTOR ssorient = ID_QUATERNION;
	static const float movespeed = 25.0f;
	static const float turnspeed = 1.0f;
	BOOL *k = Game::Keyboard.GetKeys();

	if (k[DIK_LCONTROL] && k[DIK_0])
	{
		sspos = XMVectorSet(50.0f, 0.0f, 50.0f, 0.0f);
		ss()->SetOrientation(ID_QUATERNION);
	}

	if (Game::Engine->GetCamera()->GetCameraState() != CameraClass::CameraState::DebugCamera ||
		k[DIK_LCONTROL] && k[DIK_0])
	{
		Game::Console.ProcessRawCommand(GameConsoleCommand("debug_camera 1"));
		Game::Engine->GetCamera()->SetDebugCameraPosition(XMVectorSet(0.0f, 200.0f, 0.0f, 0.0f));
		Game::Engine->GetCamera()->SetDebugCameraOrientation(XMQuaternionRotationNormal(RIGHT_VECTOR, PIOVER2));
	}

	XMVECTOR restore_size = ss()->GetSize();
	ss()->SetPosition(sspos);
	ss()->SetOrientation(ssorient);
	ss()->SetSize(XMVectorSet(5.0f, 5.0f, 5.0f, 0.0f));
	ss()->SetLocalMomentum(XMVectorSet(0.0f, 0.0f, 200.0f, 0.0f));
	ss()->RefreshPositionImmediate();

	s2()->SetPosition(NULL_VECTOR);
	s2()->SetOrientation(ID_QUATERNION);
	s2()->SetLocalMomentum(XMVectorSet(0.0f, 0.0f, 200.0f, 0.0f));
	s2()->RefreshPositionImmediate();
	s2()->RefreshPositionImmediate();
	s2()->CollisionOBB.UpdateFromObject(*s2());

	float restore_timefactor = Game::TimeFactor;
	Game::TimeFactor = 1.0f;

	if (!k[DIK_LCONTROL])
	{
		if (k[DIK_LEFTARROW]) sspos = XMVectorSubtract(sspos, XMVectorSet(movespeed * restore_timefactor, 0.0f, 0.0f, 0.0f));
		if (k[DIK_RIGHTARROW]) sspos = XMVectorAdd(sspos, XMVectorSet(movespeed * restore_timefactor, 0.0f, 0.0f, 0.0f));
		if (k[DIK_UPARROW]) sspos = XMVectorAdd(sspos, XMVectorSet(0.0f, 0.0f, movespeed * restore_timefactor, 0.0f));
		if (k[DIK_DOWNARROW]) sspos = XMVectorSubtract(sspos, XMVectorSet(0.0f, 0.0f, movespeed * restore_timefactor, 0.0f));
	}
	else
	{
		if (k[DIK_LEFTARROW])  {
			XMVECTOR delta = XMQuaternionRotationNormal(UP_VECTOR, -turnspeed * restore_timefactor);
			ss()->SetOrientation(XMQuaternionMultiply(ssorient, delta));
		}
		if (k[DIK_RIGHTARROW])  {
			XMVECTOR delta = XMQuaternionRotationNormal(UP_VECTOR, turnspeed * restore_timefactor);
			ss()->SetOrientation(XMQuaternionMultiply(ssorient, delta));
		}
	}

	bool b = Game::PhysicsEngine.TestContinuousSphereVsOBBCollision(ss(), s2());
	GamePhysicsEngine::CollisionDetectionResult collision = Game::PhysicsEngine.LastCollisionTest();
	OverlayRenderer::RenderColour col = (b ? OverlayRenderer::RenderColour::RC_Red : OverlayRenderer::RenderColour::RC_Green);

	XMMATRIX world; XMVECTOR move;
	ss()->CollisionOBB.GenerateWorldMatrix(world);
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, col, ss()->GetSize());
	s2()->CollisionOBB.GenerateWorldMatrix(world);
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, col, s2()->GetSize());

	move = XMVectorNegate(collision.ContinuousTestResult.InterimCalculations.wm0);
	world = XMMatrixMultiply(ss()->GetWorldMatrix(), XMMatrixTranslationFromVector(move));
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, OverlayRenderer::RenderColour::RC_LightBlue, ss()->GetSize());

	/*move = -collision.ContinuousTestResult.InterimCalculations.wm1;
	D3DXMatrixTranslation(&trans, move.x, move.y, move.z);
	world = (*(s2()->GetWorldMatrix()) * trans);
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, OverlayRenderer::RenderColour::RC_LightBlue, s2()->GetSize().x, s2()->GetSize().y, s2()->GetSize().z);*/

	Game::Engine->GetOverlayRenderer()->RenderLine(collision.ContinuousTestResult.InterimCalculations.pos0,
		ss()->GetPosition(), col, ss()->GetCollisionSphereRadius() * 2.0f, -1.0f);
	//Game::Engine->GetOverlayRenderer()->RenderLine(collision.ContinuousTestResult.InterimCalculations.pos1,
	//	s2()->GetPosition(), col, 5.0f, -1.0f);

	if (b)
	{
		world = XMMatrixTranslationFromVector(collision.ContinuousTestResult.ContactPoint);
		Game::Engine->GetOverlayRenderer()->RenderBox(world, OverlayRenderer::RenderColour::RC_LightBlue, 3.0f, 10.0f);
	}

	ss()->SetWorldMomentum(NULL_VECTOR);
	s2()->SetWorldMomentum(NULL_VECTOR);

	ss()->SetSize(restore_size);
	Game::TimeFactor = restore_timefactor;
}


// Run the full continuous collision detection (CCD) test on all nearby objects in the main system environment
void RJMain::DebugFullCCDTest(void)
{
	static AXMVECTOR sspos = XMVectorSet(50.0f, 0.0f, 50.0f, 0.0f);
	static AXMVECTOR ssorient = ID_QUATERNION;
	static const float movespeed = 25.0f;
	static const float turnspeed = 1.0f;
	BOOL *k = Game::Keyboard.GetKeys();

	if (k[DIK_LCONTROL] && k[DIK_0])
	{
		sspos = XMVectorSet(50.0f, 0.0f, 50.0f, 0.0f);
		ss()->SetOrientation(ID_QUATERNION);
	}

	if (Game::Engine->GetCamera()->GetCameraState() != CameraClass::CameraState::DebugCamera ||
		k[DIK_LCONTROL] && k[DIK_0])
	{
		Game::Console.ProcessRawCommand(GameConsoleCommand("debug_camera 1"));
		Game::Engine->GetCamera()->SetDebugCameraPosition(XMVectorSet(0.0f, 200.0f, 0.0f, 0.0f));
		Game::Engine->GetCamera()->SetDebugCameraOrientation(XMQuaternionRotationNormal(RIGHT_VECTOR, PIOVER2));
	}

	XMVECTOR restore_size = ss()->GetSize();
	ss()->SetPosition(sspos);
	ss()->SetOrientation(ssorient);
	ss()->SetSize(XMVectorSet(5.0f, 5.0f, 5.0f, 0.0f));
	ss()->SetLocalMomentum(XMVectorSet(0.0f, 0.0f, 200.0f, 0.0f));
	ss()->RefreshPositionImmediate();

	s2()->SetPosition(NULL_VECTOR);
	s2()->SetOrientation(ID_QUATERNION);
	s2()->RefreshPositionImmediate();
	s2()->CollisionOBB.UpdateFromObject(*s2());

	float restore_timefactor = Game::TimeFactor;
	float restore_physicstime = Game::PhysicsEngine.PhysicsClock.TimeFactor;
	Game::TimeFactor = 1.0f;
	Game::PhysicsEngine.PhysicsClock.TimeFactor = 1.0f;

	if (!k[DIK_LCONTROL])
	{
		if (k[DIK_LEFTARROW])	sspos = XMVectorSubtract(sspos, XMVectorSet(movespeed * restore_timefactor, 0.0f, 0.0f, 0.0f));
		if (k[DIK_RIGHTARROW])	sspos = XMVectorAdd(sspos, XMVectorSet(movespeed * restore_timefactor, 0.0f, 0.0f, 0.0f));
		if (k[DIK_UPARROW])
			if (!k[DIK_LSHIFT]) sspos = XMVectorAdd(sspos, XMVectorSet(0.0f, 0.0f, movespeed * restore_timefactor, 0.0f));
			else				sspos = XMVectorAdd(sspos, XMVectorSet(0.0f, movespeed * restore_timefactor, 0.0f, 0.0f));
		if (k[DIK_DOWNARROW])
			if (!k[DIK_LSHIFT])	sspos = XMVectorSubtract(sspos, XMVectorSet(0.0f, 0.0f, movespeed * restore_timefactor, 0.0f));
			else				sspos = XMVectorSubtract(sspos, XMVectorSet(0.0f, movespeed * restore_timefactor, 0.0f, 0.0f));
	}
	else
	{
		if (k[DIK_LEFTARROW])  {
			XMVECTOR delta = XMQuaternionRotationNormal(UP_VECTOR, -turnspeed * restore_timefactor);
			ss()->SetOrientation(XMQuaternionMultiply(ssorient, delta));
		}
		if (k[DIK_RIGHTARROW])  {
			XMVECTOR delta = XMQuaternionRotationNormal(UP_VECTOR, turnspeed * restore_timefactor);
			ss()->SetOrientation(XMQuaternionMultiply(ssorient, delta));
		}
	}

	XMVECTOR wm0 = XMVectorMultiply(ss()->PhysicsState.WorldMomentum, Game::TimeFactorV);
	XMVECTOR pos0 = XMVectorSubtract(ss()->GetPosition(), wm0);
	XMVECTOR posn = ss()->GetPosition();
	XMVECTOR rotn = ss()->GetOrientation();

	// Do collision detection & handling
	iSpaceObject *collider = Game::PhysicsEngine.PerformContinuousSpaceCollisionDetection(ss());
	GamePhysicsEngine::CollisionDetectionResult collision = Game::PhysicsEngine.LastCollisionTest();
	OverlayRenderer::RenderColour col = (collider ? OverlayRenderer::RenderColour::RC_Red : OverlayRenderer::RenderColour::RC_Green);

	// Render box at origin (pos0)
	XMMATRIX rot, trans, world;
	rot = XMMatrixRotationQuaternion(rotn);
	trans = XMMatrixTranslationFromVector(pos0);
	world = XMMatrixMultiply(rot, trans);
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, OverlayRenderer::RenderColour::RC_LightBlue, ss()->GetSize());

	// Render box at end (ss()->pos, before any collision handling)
	trans = XMMatrixTranslationFromVector(posn);
	world = XMMatrixMultiply(rot, trans);
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, col, ss()->GetSize());

	// Render line from pos0 to end pos (before any collision handling)
	Game::Engine->GetOverlayRenderer()->RenderLine(pos0, posn, col, ss()->GetCollisionSphereRadius() * 2.0f, -1.0f);

	if (collider)
	{
		// Render box at the object we collided with
		rot = XMMatrixRotationQuaternion(collider->GetOrientation());
		trans = XMMatrixTranslationFromVector(collider->GetPosition());
		world = XMMatrixMultiply(rot, trans);
		Game::Engine->GetOverlayRenderer()->RenderCuboid(world, col, collider->GetSize());

		// Render contact point
		world = XMMatrixTranslationFromVector(collision.ContinuousTestResult.ContactPoint);
		Game::Engine->GetOverlayRenderer()->RenderBox(world, OverlayRenderer::RenderColour::RC_LightBlue, 3.0f, 10.0f);

		// If we deflected off the collider, render a line to our resulting position
		XMVECTOR endwm = ss()->PhysicsState.WorldMomentum;
		if (true || !XMVector3NearEqual(wm0, endwm, Game::C_EPSILON_V))
		{
			Game::Engine->GetOverlayRenderer()->RenderLine(
				collision.ContinuousTestResult.ContactPoint, 
				XMVectorAdd(collision.ContinuousTestResult.ContactPoint, endwm), 
				OverlayRenderer::RenderColour::RC_LightBlue, ss()->GetCollisionSphereRadius(), -1.0f);
		}

		// Remove any momentum added as part of the collision
		collider->SetWorldMomentum(NULL_VECTOR);
		collider->PhysicsState.AngularVelocity = NULL_VECTOR;
		if (collider->GetObjectType() == iObject::ObjectType::ComplexShipSectionObject) {
			ComplexShipSection *sec = (ComplexShipSection*)collider;
			if (sec->GetParent()) { sec->GetParent()->SetWorldMomentum(NULL_VECTOR); sec->GetParent()->PhysicsState.AngularVelocity = NULL_VECTOR; }
		}
	}

	ss()->SetWorldMomentum(NULL_VECTOR);
	ss()->PhysicsState.AngularVelocity = NULL_VECTOR;
	ss()->SetSize(restore_size);

	Game::TimeFactor = restore_timefactor;
	Game::PhysicsEngine.PhysicsClock.TimeFactor = restore_physicstime;
}

// Fires a basic projectile along the structural test path for debug testing
void RJMain::DebugFireBasicProjectile(const BasicRay & trajectory) const
{
	static const unsigned int PROJ_INTERVAL = 250U;
	static unsigned int last_proj = 0U;

	if (Game::PersistentClockMs < (last_proj + PROJ_INTERVAL)) return;
	last_proj = Game::PersistentClockMs;

	// Make sure we have all required data
	BasicProjectileDefinition *def = D::BasicProjectiles.Get("basiclaser01");
	if (!def) return;

	SpaceSystem *sys = Game::CurrentPlayer->GetPlayerSystem();
	if (!sys) return;

	// Create the projectile and set it on a path between the two test markers
	XMVECTOR vel_n = XMVector3NormalizeEst(trajectory.Direction);
	sys->Projectiles.AddProjectile(def, 0U, trajectory.Origin, 
		QuaternionBetweenVectors(FORWARD_VECTOR, vel_n), trajectory.Direction);
}

Ship * RJMain::InitialiseTemporaryPlayerShip(void)
{
	// Temp: Create a new ship for the player to use
	SimpleShip *ss_ship = SimpleShip::Create("testship1");
	ss_ship->SetName("Player ship ss");
	ss_ship->OverrideInstanceCode("ss");
	ss_ship->ChangeEntityAIState(EntityAIStates::EntityAIState::NoAI);
	ss_ship->SetFaction(Game::FactionManager.GetFactionIDByCode("faction_us"));
	
	ss_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
	ss_ship->SetPosition(XMVectorSet(300, 225, 100, 0));
	ss_ship->SetOrientation(ID_QUATERNION);
	
	SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(ss_ship);

	// Also assign to a temporary application variable so we can reference in global scopes
	ss = ss_ship;

	return ss_ship;
}

void RJMain::__CreateDebugScenario(void)
{
	// Temp: Set the US/PRC factions to be hostile towards each other for testing purposes
	Game::FactionManager.FactionDispositionChanged(Game::FactionManager.GetFactionIDByCode("faction_us"),
		Game::FactionManager.GetFactionIDByCode("faction_prc"), Faction::FactionDisposition::Hostile);
	Game::FactionManager.FactionDispositionChanged(Game::FactionManager.GetFactionIDByCode("faction_prc"),
		Game::FactionManager.GetFactionIDByCode("faction_us"), Faction::FactionDisposition::Hostile);

	// Temp: Create two complex ships in this scenario
	if (true) {
		ComplexShip *css[2];
		Faction::F_ID factions[2] = { Game::FactionManager.GetFactionIDByCode("faction_us"), Game::FactionManager.GetFactionIDByCode("faction_us") };
		XMVECTOR positions[2] = { XMVectorSet(150, 225, 100, 0), XMVectorSet(950, 200, 120, 0) };
		XMVECTOR orients[2] = { ID_QUATERNION, XMQuaternionRotationAxis(UP_VECTOR, DegToRad(15.0f)) };
		bool is_armed[2] = { true, false };
		bool has_engine_control[2] = { false, false };
		int create_count = 1; // 2

		css[0] = NULL; css[1] = NULL;
		for (int c = 0; c < create_count; ++c)
		{
			css[c] = ComplexShip::Create(false ? "testfrigate2" : "enginetest");
			css[c]->SetName(concat("Test frigate cs ")(c + 1).str().c_str());
			css[c]->OverrideInstanceCode(concat("cs")(c + 1).str());
			css[c]->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
			css[c]->SetPosition(positions[c]);
			css[c]->SetOrientation(orients[c]);
			css[c]->SetInvulnerabilityFlag(true);
			css[c]->SetFaction(factions[c]);
			css[c]->SetShipEngineControl(has_engine_control[c]);

			Engine *eng = (Engine*)D::Equipment.Get("FRIGATE_HEAVY_ION_ENGINE1");
			css[c]->GetHardpoints().GetHardpointsOfType(Equip::Class::Engine).at(0)->MountEquipment(eng->Clone());

			if (is_armed[c])
			{
				XMVECTOR rotleft = XMQuaternionRotationNormal(UP_VECTOR, -PI / 4.0f);
				XMVECTOR rotright = XMQuaternionRotationNormal(UP_VECTOR, PI / 4.0f);
				XMFLOAT3 sz; XMStoreFloat3(&sz, css[c]->GetSize());
				for (int i = 0; i < 4; ++i)
				{
					for (int j = 0; j < 2; ++j)
					{
						SpaceTurret *nt = SpaceTurret::Create("turret_basic01"); if (!nt) continue;

						XMVECTOR pos = XMVectorSet((((float)j * (sz.x * 0.9f)) + (sz.x * 0.05f)), sz.y, (((((float)i + 1.0f) / 4.0f) * (sz.z * 0.9f)) + (sz.z * 0.05f)), 0.0f);
						pos = XMVectorSubtract(pos, XMVectorMultiply(css[c]->GetSize(), HALF_VECTOR));
						nt->SetRelativePosition(pos);
						nt->SetBaseRelativeOrientation((j == 0 ? rotleft : rotright));

						nt->SetYawRate(PI);
						nt->SetPitchRate(PI);
						nt->SetYawLimitFlag(false);
						nt->SetPitchLimits(-PIOVER2, PIOVER2);
						
						for (int l = 0; l < nt->GetLauncherCount(); ++l)
							nt->GetLauncher(0)->SetProjectileSpread(0.1f);

						nt->RecalculateTurretStatistics();
						css[c]->TurretController.AddTurret(nt);

						// *** TODO: temporarily required since CS objects are currently defaulting to "tactical" simulation
						css[c]->SetAsSimulationHub();
					}
				}
			}
		}

		cs = css[0];
		cs2 = css[1];
	}


	// Temp: Create a second ship in this scenario
	if (true) {
		SimpleShip *s2_ship = SimpleShip::Create("testship1");
		SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(s2_ship);
		s2_ship->SetName("Test ship s2");
		s2_ship->OverrideInstanceCode("Test ship s2");
		s2_ship->SetFaction(Game::FactionManager.GetFactionIDByCode("faction_us"));
		s2_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
		s2_ship->SetPosition(XMVectorAdd(ss()->GetPosition(), XMVectorSet(0.0f, 0.0f, 120.0f, 0.0f)));
		s2_ship->SetOrientation(ID_QUATERNION);
		s2 = s2_ship;
	}


	if (true) {
		SimpleShip *s3_0_ship = SimpleShip::Create("testship1");
		SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(s3_0_ship);
		s3_0_ship->SetFaction(Game::FactionManager.GetFactionIDByCode("faction_us"));
		s3_0_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
		s3_0_ship->SetPosition(XMVectorAdd(s2()->GetPosition(), XMVectorSet(0.0f, 0.0f, 100.0f, 0.0f)));
		s3_0_ship->SetOrientation(ID_QUATERNION);
		s3[0] = s3_0_ship;
	}

	if (true) {
		SimpleShip *s3_1_ship = SimpleShip::Create("testship1");
		SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(s3_1_ship);
		s3_1_ship->SetFaction(Game::FactionManager.GetFactionIDByCode("faction_us"));
		s3_1_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
		s3_1_ship->SetPosition(XMVectorAdd(s2()->GetPosition(), XMVectorSet(-10000.0f, 0.0f, 100.0f, 0.0f)));
		s3_1_ship->SetOrientation(ID_QUATERNION);
		s3[1] = s3_1_ship;
	}

	// Temp: Create a new actor
	Actor *a1_actor = D::Actors.Get("human_soldier_basic")->CreateInstance();
	a1_actor->SetName("Other actor");
	a1_actor->OverrideInstanceCode("otheractor");
	a1_actor->SetFaction(Game::FactionManager.GetFactionIDByCode("faction_prc"));
	a1_actor->MoveIntoEnvironment(cs());
	ComplexShipTile *other_actor_tile = cs()->GetElement(4, 4, 0)->GetTile();
	XMVECTOR other_actor_env_position = XMVectorAdd(other_actor_tile->GetElementPosition(), Game::C_CS_ELEMENT_MIDPOINT_V);
	a1_actor->SetEnvironmentPositionAndOrientation(other_actor_env_position, ID_QUATERNION);
			
	a1 = a1_actor; 
	

	Ship *_s[3] = { ss(), s2(), s3[0]() };
	for (int i = 0; i < 3; ++i)
	{
		const float xoffset = 12.0f;
		SpaceTurret *left = SpaceTurret::Create("turret_laser01");
		SpaceTurret *right = SpaceTurret::Create("turret_laser01");
		left->SetRelativePosition(XMVectorSet(-xoffset, -0.1f, 0.25f, 0.0f));
		right->SetRelativePosition(XMVectorSet(xoffset, -0.1f, 0.25f, 0.0f));

		_s[i]->TurretController.AddTurret(left);
		_s[i]->TurretController.AddTurret(right);
	}

	ss()->TurretController.SetControlModeOfAllTurrets(SpaceTurret::ControlMode::ManualControl);
	s2()->TurretController.SetControlModeOfAllTurrets(SpaceTurret::ControlMode::AutomaticControl);
	s3[0]()->TurretController.SetControlModeOfAllTurrets(SpaceTurret::ControlMode::AutomaticControl);
	
	/*ss()->TurretController.AddTurret(sst);
	SpaceTurret *sst2 = sst->Copy();
	s2()->TurretController.AddTurret(sst2);
	SpaceTurret *sst3 = sst->Copy();
	s3[0]()->TurretController.AddTurret(sst3);
	*/
	OutputDebugString(cs()->SpatialPartitioningTree->DebugOutput().c_str());

	bitstring up = DirectionBS::Up_BS;				// == 2
	bitstring left = DirectionBS::Left_BS;			// == 1
	bitstring down = DirectionBS::Down_BS;			// == 8
	bitstring right = DirectionBS::Right_BS;		// == 4

	bitstring straight = (up | down);				// == 10
	bitstring ne_corner = (up | right);				// == 6
	bitstring nse_t = (up | right | down);			// == 14
	bitstring nsew = (up | left | down | right);	// == 15

	// Temp: Create a directional light source for the system
	LightSource *l = LightSource::Create(Game::Engine->LightingManager->GetDefaultDirectionalLightData());
	l->OverrideInstanceCode("syslight");
	l->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
	l->SetPositionAndOrientation(NULL_VECTOR, QuaternionBetweenVectors(FORWARD_VECTOR, 
		XMVector3Normalize(XMVectorSubtract(cs()->GetPosition(), ss()->GetPosition()))));	// System light direction is initial vector from (ss -> cs)
	l->LightObject().SetColour(XMFLOAT4(1.0f, 224.0f / 255.0f, 163.0f / 255.0f, 1.0f));
	l->LightObject().SetIntensity(0.25f);

	// Temp: Create a point light source near the player
	LightSource *l2 = LightSource::Create(Game::Engine->LightingManager->GetDefaultPointLightData());
	l2->SetRange(600.0f);
	l2->LightObject().SetIntensity(0.75f);
	//l2->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
	l2->SetPosition(XMVectorSet(400, 300, 100, 0));
	lt = l2;

	// Add a spotlight to the player actor
	LightSource *player_light = LightSource::Create(Game::Engine->LightingManager->GetDefaultSpotLightData());
	player_light->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
	player_light->SetPosition(NULL_VECTOR);
	player_light->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
	player_light->LightObject().SetColour(Float4MultiplyScalar(XMFLOAT4(213, 242, 241, 244), (1.0f / 255.0f)));
	player_light->LightObject().SetIntensity(1.0f);
	//player_light->LightObject().Deactivate();
	lt2 = player_light;

	LightSource *clight = LightSource::Create(Game::Engine->LightingManager->GetDefaultPointLightData());
	clight->OverrideInstanceCode("clight");
	clight->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"));
	clight->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
	clight->SetPosition(XMVectorSet(145.0f, 223.0f, 0.0f, 0.0f));
	clight->LightObject().SetColour(Float4MultiplyScalar(XMFLOAT4(213, 242, 241, 244), (1.0f / 255.0f)));
	clight->LightObject().SetRange(25.0f);
	clight->LightObject().SetIntensity(0.5f);
	
	Game::Log << LOG_INFO << "--- Debug scenario created\n";
}

void RJMain::ActivateDebugPortalRenderingTest(const iObject *target)
{
	// Make sure this is a valid target environment object, that exists within some parent environment
	if (!target ||
		target->GetObjectClass() != iObject::ObjectClass::EnvironmentObjectClass ||
		((iEnvironmentObject*)target)->GetParentEnvironment() == NULL || 
		((iEnvironmentObject*)target)->GetParentEnvironment()->GetObjectType() != iObject::ObjectType::ComplexShipObject)
	{
		DeactivateDebugPortalRenderingTest();
		return;
	}

	// Transition to an overhead view of the subject's parent environment and force rendering of the interior
	iEnvironmentObject *target_eo = (iEnvironmentObject*)target;
	ComplexShip *environment = (ComplexShip*)target_eo->GetParentEnvironment();
	environment->ForceRenderingOfInterior(true);
	environment->FadeHullToAlpha(1.5f, 0.2f);
	environment->FadeAllTiles(2.5f, 0.3f);
	Game::Engine->GetCamera()->ZoomToOverheadShipView(environment);

	// Set the test subject and enable this debug mode
	m_debug_portalrenderingtest_subject = target_eo;
	m_debug_portalrenderingtest_environment = environment;
	m_debug_portalrenderingtest = true;
}

void RJMain::DebugRunPortalRenderingTest(void)
{
	// Deactivate the debug mode if our test subject or its environment ceases to exist, or if the former leaves the latter
	const iEnvironmentObject *target = m_debug_portalrenderingtest_subject();
	ComplexShip *environment = m_debug_portalrenderingtest_environment();
	if (!target || !environment || ((ComplexShip*)target->GetParentEnvironment()) != environment)
	{
		DeactivateDebugPortalRenderingTest();
		return;
	}

	// Set the per-frame viewer override in the core engine
	Game::Engine->DebugOverrideInitialPortalRenderingViewer(target);

	// Render an overlay icon at the position of the test subject, and a projection along its heading vector
	static const float heading_render_distance = 5.0f;
	static const float heading_render_breadth = 1.0f;
	XMVECTOR heading = XMVector3NormalizeEst(target->GetHeading());
	XMVECTOR heading_vector = XMVectorAdd(target->GetPosition(), XMVectorScale(heading, heading_render_distance));
	Game::Engine->GetOverlayRenderer()->RenderNodeFlat(target->GetPosition(), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	Game::Engine->GetOverlayRenderer()->RenderLineFlat(target->GetPosition(), heading_vector, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), heading_render_breadth, -1.0f);

	// Allow additional keyboard controls within this debug mode, IF the target is an actor object that we can control
	if (target->GetObjectType() == iObject::ObjectType::ActorObject)
	{
		static const float yaw_rate = 1.0f;
		Actor *actor = (Actor*)target;

		if (Game::Keyboard.GetKey(DIK_Q)) actor->Turn(-yaw_rate * Game::TimeFactor);
		else if (Game::Keyboard.GetKey(DIK_E)) actor->Turn(+yaw_rate * Game::TimeFactor);
	}
	
}

void RJMain::DeactivateDebugPortalRenderingTest(void)
{
	// Deactivate any modifications currently in place on the parent environment
	ComplexShip *environment = m_debug_portalrenderingtest_environment();
	if (environment)
	{
		environment->ForceRenderingOfInterior(false);
		environment->FadeHullToAlpha(2.5f, 1.0f);
		environment->FadeAllTiles(1.5f, 1.0f);
	}

	// Release the camera back to normal operation
	Game::Engine->GetCamera()->ReleaseCamera();

	// Reset any parameters and deactivate the debug mode
	m_debug_portalrenderingtest = false;
	m_debug_portalrenderingtest_subject = NULL;
	m_debug_portalrenderingtest_environment = NULL;
}


void RJMain::DEBUGDisplayInfo(void)
{
	// In-engine tests - display the tests if they have been activated
	if (m_debug_ccdspheretest) DebugCCDSphereTest();
	if (m_debug_ccdobbtest) DebugFullCCDTest(); // DebugCCDOBBTest();
	if (m_debug_portalrenderingtest) DebugRunPortalRenderingTest();

	// Debug info line 1 - basic location data
	if (m_debuginfo_flightstats)
	{
		const XMFLOAT3 & cam_pos = Game::Engine->GetCamera()->GetPositionF();
		XMFLOAT3 player_pos; INTVECTOR3 player_el; 
		if (Game::CurrentPlayer)
		{
			XMStoreFloat3(&player_pos, Game::CurrentPlayer->GetPosition());
			if (Game::CurrentPlayer->GetState() == Player::StateType::OnFoot)
			{
				player_el = Game::CurrentPlayer->GetComplexShipEnvironmentElementLocation();
			}
			else
			{
				player_el = INTVECTOR3(-1, -1, -1);
			}
		}
		else
		{
			player_pos = XMFLOAT3(-1.0f, -1.0f, -1.0f);
			player_el = INTVECTOR3(-1, -1, -1);
		}

		// Output debug string
		sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_1, "Cam: (%.2f, %.2f, %.2f), Player: (%.2f, %.2f, %.2f), PlayerEl: (%d, %d, %d)",
			cam_pos.x, cam_pos.y, cam_pos.z,
			player_pos.x, player_pos.y, player_pos.z,
			player_el.x, player_el.y, player_el.z
		);
		// TODO [textrender]: Update for new text rendering component
		//Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_1, D::UI->TextStrings.C_DBG_FLIGHTINFO_1, 1.0f);
	}

	// Debug info line 2 - engine rendering info
	if (m_debuginfo_renderinfo)
	{
		CoreEngine::EngineRenderInfoData renderinfo = Game::Engine->GetRenderInfo();
		sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_2, "Render Info: Draw Calls: %zu, Instances: %zu, ZSortedInstances: %zu, SkinnedInstances: %zu [%s %s %s %s %s %s ]",
			renderinfo.DrawCalls, renderinfo.InstanceCount, renderinfo.InstanceCountZSorted, renderinfo.InstanceCountSkinnedModel, 
			(renderinfo.ShipRenderCount == 0 ? "" : concat(" S.Ship = ")(renderinfo.ShipRenderCount).str().c_str()),
			(renderinfo.ComplexShipRenderCount == 0 ? "" : concat(" C.Ship = ")(renderinfo.ComplexShipRenderCount).str().c_str()),
			(renderinfo.ComplexShipSectionRenderCount == 0 ? "" : concat(" CS.Sec = ")(renderinfo.ComplexShipSectionRenderCount).str().c_str()),
			(renderinfo.ComplexShipTileRenderCount == 0 ? "" : concat(" CS.Tile = ")(renderinfo.ComplexShipTileRenderCount).str().c_str()),
			(renderinfo.ActorRenderCount == 0 ? "" : concat(" Actor = ")(renderinfo.ActorRenderCount).str().c_str()),
			(renderinfo.TerrainRenderCount == 0 ? "" : concat(" Terrain = ")(renderinfo.TerrainRenderCount).str().c_str())
		);

		// TODO [textrender]: Update for new text rendering component
		//Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_2, D::UI->TextStrings.C_DBG_FLIGHTINFO_2, 1.0f);
	}

	// Debug info line 3 - object and collision data
	if (m_debuginfo_collisiondata)
	{
		const CollisionDetectionResultsStruct & coll = Game::PhysicsEngine.CollisionDetectionResults;
		sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_3, 
#			ifdef OBJMGR_DEBUG_MODE
				"Obj cache: Size = %d (Hit: %d / Miss: %d) | Collisions: Spc = (O/O:%d > B:%d > C:%d), Sp-CCD = (O/O:%d > C:%d), Env = (E:%d > O:%d > E[O]:%d > O/T:%d, O/O:%d > B:%d > C:%d)",
				Game::ObjectSearchManager::DetermineTotalCurrentCacheSize(), Game::ObjectSearchManager::DetermineTotalCurrentCacheHits(),
				Game::ObjectSearchManager::DetermineTotalCurrentCacheMisses(),
#			else
				"Obj cache: Size = %d | Collisions: Spc = (O/O:%d > B:%d > C:%d), Sp-CCD = (O/O:%d > C:%d), Env = (E:%d > O:%d > E[O]:%d > O/T:%d, O/O:%d > B:%d > C:%d)",
				Game::ObjectSearchManager::DetermineTotalCurrentCacheSize(),
#			endif
			coll.SpaceCollisions.CollisionChecks, coll.SpaceCollisions.BroadphaseCollisions, coll.SpaceCollisions.Collisions,
			coll.SpaceCollisions.CCDCollisionChecks, coll.SpaceCollisions.CCDCollisions, 
			coll.EnvironmentCollisions.ElementsChecked, coll.EnvironmentCollisions.ObjectsChecked, coll.EnvironmentCollisions.ElementsCheckedAroundObjects,
			coll.EnvironmentCollisions.ObjectVsTerrainChecks, coll.EnvironmentCollisions.ObjectVsObjectChecks,
			coll.EnvironmentCollisions.BroadphaseCollisions, coll.EnvironmentCollisions.Collisions);
		
		// TODO [textrender]: Update for new text rendering component
		//Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_3, D::UI->TextStrings.C_DBG_FLIGHTINFO_3, 1.0f);
	}

	// Debug info line 4 - temporary debug data as required
	if (true)
	{	
		DataObjectContinuousSwitch *t = NULL;
		for (const auto & terrain : cs()->TerrainObjects)
		{
			if (!terrain->IsDynamic()) continue;
			DynamicTerrain *dt = terrain->ToDynamicTerrain();
			if (dt->GetDynamicTerrainDefinition()->GetCode() == "switch_continuous_lever_vertical_01") t = (DataObjectContinuousSwitch*)dt;
		}

		if (t)
		sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_4, "Pos: %s, Dist: %.2f", Vector3ToString(t->GetPosition()).c_str(),
			XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(Game::CurrentPlayer->GetActor()->GetPosition(), t->GetPosition()))));
		
			/*(Game::CurrentPlayer->GetViewDirectionObject() ? Game::CurrentPlayer->GetViewDirectionObject()->GetInstanceCode().c_str() : "-"),
			(Game::CurrentPlayer->GetViewDirectionNonPlayerObject() ? Game::CurrentPlayer->GetViewDirectionNonPlayerObject()->GetInstanceCode().c_str() : "-"),
			(Game::CurrentPlayer->GetViewDirectionTerrain() ? Game::CurrentPlayer->GetViewDirectionTerrain()->GetID() : -1),
			(Game::CurrentPlayer->GetViewDirectionUsableTerrain() ? Game::CurrentPlayer->GetViewDirectionUsableTerrain()->GetID() : -1),

			(Game::CurrentPlayer->GetMouseSelectedObject() ? Game::CurrentPlayer->GetMouseSelectedObject()->GetInstanceCode().c_str() : "-"),
			(Game::CurrentPlayer->GetMouseSelectedNonPlayerObject() ? Game::CurrentPlayer->GetMouseSelectedNonPlayerObject()->GetInstanceCode().c_str() : "-"),
			(Game::CurrentPlayer->GetMouseSelectedTerrain() ? Game::CurrentPlayer->GetMouseSelectedTerrain()->GetID() : -1),
			(Game::CurrentPlayer->GetMouseSelectedUsableTerrain() ? Game::CurrentPlayer->GetMouseSelectedUsableTerrain()->GetID() : -1)
		);*/

		// TODO [textrender]: Update for new text rendering component
		//Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_4, D::UI->TextStrings.C_DBG_FLIGHTINFO_4, 1.0f);


		// Tmp: Update player spotlight position and orientation to match camera
		LightSource *lights[2] = { lt(), lt2() };
		for (LightSource *active : lights)
		{
			active->SetPosition(Game::Engine->GetCamera()->GetPosition());
			active->SetOrientation(Game::Engine->GetCamera()->DetermineAdjustedOrientation());
			if (Game::Keyboard.GetKey(DIK_CAPSLOCK))
			{
				active->LightObject().SetRange(active->LightObject().GetRange() < 100.0f ? 200.0f : 20.0f);
				Game::Keyboard.LockKey(DIK_CAPSLOCK);
			}
		}

		Game::Engine->GetTextRenderer()->RenderString("*", 0U, DecalRenderingMode::WorldSpace, Game::GetObjectByInstanceCode("clight")->GetPosition(),
			14.0f, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f), TextAnchorPoint::Centre);


		/*Game::Engine->RenderMaterialToScreen(*Game::Engine->GetAssets().GetMaterial("debug_material"), XMFLOAT2(0.0f, (float)Game::ScreenHeight), XMFLOAT2(300, 300),
			((float)(Game::ClockMs % 750)) * (TWOPI / 750.0f), ((float)((Game::ClockMs % 1000)) / 1000.0f));*/
		
		


		/* DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY */
/*		auto renderinfo = Game::Engine->GetRenderInfo();
		sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_2, "Render Info: Draw Calls: %zu, Instances: %zu, ZSortedInstances: %zu, SkinnedInstances: %zu [%s %s %s %s %s %s ]",
			renderinfo.DrawCalls, renderinfo.InstanceCount, renderinfo.InstanceCountZSorted, renderinfo.InstanceCountSkinnedModel,
			(renderinfo.ShipRenderCount == 0 ? "" : concat(" S.Ship = ")(renderinfo.ShipRenderCount).str().c_str()),
			(renderinfo.ComplexShipRenderCount == 0 ? "" : concat(" C.Ship = ")(renderinfo.ComplexShipRenderCount).str().c_str()),
			(renderinfo.ComplexShipSectionRenderCount == 0 ? "" : concat(" CS.Sec = ")(renderinfo.ComplexShipSectionRenderCount).str().c_str()),
			(renderinfo.ComplexShipTileRenderCount == 0 ? "" : concat(" CS.Tile = ")(renderinfo.ComplexShipTileRenderCount).str().c_str()),
			(renderinfo.ActorRenderCount == 0 ? "" : concat(" Actor = ")(renderinfo.ActorRenderCount).str().c_str()),
			(renderinfo.TerrainRenderCount == 0 ? "" : concat(" Terrain = ")(renderinfo.TerrainRenderCount).str().c_str())
		);
		Game::Log << LOG_INFO << D::UI->TextStrings.C_DBG_FLIGHTINFO_2 << "\n";*/
		/* DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY DEBUG ONLY */

		//lt2()->SetModel(Model::GetModel("spotlight_cone_model"));
		//lt()->SetPosition(XMVector3TransformCoord(XMVectorSetZ(NULL_VECTOR, 150.0f), ss()->GetWorldMatrix()));
		//lt()->SetSize(XMVectorReplicate(45.0f));
		//Game::Engine->RenderObject(lt2());

	}

	// 1. Add idea of maneuvering thrusters that are used to Brake(), rather than simple universal decrease to momentum today, and which will counteract e.g. CS impact momentum? ***

}

