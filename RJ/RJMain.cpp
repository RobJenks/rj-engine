
#include "DX11_Core.h" //#include "FullDX11.h"

#include <stdio.h>
#include <cstdio>
#include <string.h>
#include <iostream>
#include <tchar.h>
#include "time.h"

#include "FastMath.h"
#include "Octree.h"
#include "OctreePruner.h"
#include "GameSpatialPartitioningTrees.h"

#include "iObject.h"
#include "CoreEngine.h"
#include "D3DMain.h"
#include "CameraClass.h"
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
#include "CentralScheduler.h"
#include "CollisionDetectionResultsStruct.h"
#include "GamePhysicsEngine.h"
#include "SimulationObjectManager.h"
#include "FactionManagerObject.h"
#include "LogManager.h"
#include "SpaceSystem.h"

#include "Profiler.h"

#include "SpaceEmitter.h"
#include "Ship.h"
#include "Ships.h"
#include "SimpleShip.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipElement.h"
#include "CapitalShipPerimeterBeacon.h"

#include "CSQuartersTile.h"					// DBG
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
#include "ViewFrustrum.h"

#include "Equipment.h"
#include "Engine.h"
#include "Hardpoint.h"
#include "HpEngine.h"

#include "GameUniverse.h"
#include "ImmediateRegion.h"
#include "SystemRegion.h"
#include "StaticTerrain.h"

#include "TextBlock.h"

#include "SkinnedModel.h"
#include "SkinnedNormalMapShader.h"
#include "ActorAttributes.h"
#include "ActorBase.h"
#include "Actor.h"
#include <random>
#include <stdlib.h> 

#include "RJMain.h"


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
	if (RemoveFileNameFromPathStringPathW(cpath) == FALSE) return ErrorCodes::CouldNotParseExecutablePath;
	std::wstring wsnewpath = std::wstring(cpath);
	Game::ExePath = ConvertWStringToString(wsnewpath);

	// Deallocate any temporary data and return success
	delete cexe; delete cpath;
	return ErrorCodes::NoError;
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
	// Initial validation: the engine must be operational, and we must have a current player, in order to begin rendering
	if (Game::Engine->Operational() && Game::CurrentPlayer)
	{
		// Calculate time modifiers in ms/secs based on time delta since last frame, then store them globally for use in all methods
		RunInternalClockCycle();

		// Retrieve required data
		SpaceSystem *player_system = Game::CurrentPlayer->GetSystem();

		// Begin the simulation cycle
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_BeginCycle)
		{
			Game::Logic::BeginSimulationCycle();
			Game::CurrentPlayer->BeginSimulationCycle();
			Game::ObjectManager.InitialiseFrame();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_BeginCycle)

		// Read user input from the mouse and keyboard
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
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_CentralScheduler)
		{
			Game::Scheduler.RunScheduler();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_CentralScheduler);

		// Simluate the current valid area of universe.  Note: in future, to be split between full, real-time simulation
		// and more distant simulation that is less frequent/detailed/accurate
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_SimulateSpaceObjectMovement)
		{
			// Simulate objects
			Game::Logic::SimulateAllObjects();

			// Simulate all projectiles in the current player system
			if (player_system) player_system->Projectiles.SimulateProjectiles(player_system->SpatialPartitioningTree);
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_SimulateSpaceObjectMovement)

		// Simulate all game physics
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_CollisionDetection)
		{
			Game::PhysicsEngine.SimulatePhysics();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_CollisionDetection);

		// Update all regions, particularly those centred on the player ship
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_UpdateRegions)
		{
			UpdateRegions();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_UpdateRegions);

		// Update the player state, including camera view calculation etc. for rendering
		Game::CurrentPlayer->UpdatePlayerState();

		/* *** Begin rendering process ****/

		// Calculate the current camera view matrix
		Game::Engine->GetCamera()->CalculateViewMatrix();

		// Begin generating this frame
		Game::Engine->BeginFrame();

		// Pass to the main rendering function in the core engine, to render everything required in turn
		RJ_PROFILE_START(Profiler::ProfiledFunctions::Prf_Render)
		{
			Game::Engine->Render();
		}
		RJ_PROFILE_END(Profiler::ProfiledFunctions::Prf_Render);

		// Perform FPS calculations and render if required
		PerformFPSCalculations();

		// DEBUG DISPLAY FUNCTIONS
		DEBUGDisplayInfo();

		// End the current frame
		Game::Engine->EndFrame();

		// End the cycle by storing the previous clock values, for calculating the delta time in the next frame
		EndInternalClockCycle();

		// Log all profiling information, if profiling is enabled
		RJ_PROFILE_LOG(Game::ClockDelta); 
	}
	return true;
}

void RJMain::TerminateApplication()
{
	// Perform an initial flush of the game logs, to record any recent logs in case we are terminating due to an error
	// that could potentially cause a failure before full shutdown
	Game::Log.FlushAllStreams();

	// Terminate all objects in the game
	Game::ShutdownObjectRegisters();

	// Release all standard model/geometry data
	Model::TerminateAllModelData();

	// Terminate the user interface and all UI controllers and layouts
	TerminateUserInterface();

	// Terminate and deallocate the universe & all system data
	TerminateUniverse();

	// Region termination functions
	TerminateRegions();

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
	m_framecount++;
	m_elapsedtime += Game::TimeFactor;

	// Recalculate once per second
	if (m_elapsedtime >= 1.0f)
	{
		// Calculate the new FPS value	
		Game::FPS = m_framecount / m_elapsedtime;
		m_framecount = m_elapsedtime = 0.0f;

		// If we are recording memory allocations, do so at the same time as the FPS calculation
		#if defined(_DEBUG) && defined(DEBUG_LOGALLOCATEDMEMORY)
			_CrtMemCheckpoint(&m_memstate);
		#endif

		// Update the FPS text object, if it is being rendered
		if (D::UI->IsFPSCounterDisplayed() && Game::FPS > 0 && Game::FPS < 99999)
		{
			// Two versions of this display, depending on whether we are in debug mode and logging current allocations
			#if defined(_DEBUG) && defined(DEBUG_LOGALLOCATEDMEMORY)
				sprintf(D::UI->TextStrings.C_DBG_FPSCOUNTER, "FPS: %d, Allocations: %Ld [%Ld blocks]",
					(int)Game::FPS, m_memstate.lSizes[1], m_memstate.lCounts[1]);
			#else
				sprintf(D::UI->TextStrings.C_DBG_FPSCOUNTER, "FPS: %d", (int)Game::FPS);
			#endif

			// Update the text string for rendering next frame
			Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FPSCOUNTER,
				D::UI->TextStrings.C_DBG_FPSCOUNTER, 1.0f);
		}
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
	// First, pass the keyboard state to the current player to update the player control (if we are in 'normal' camera mode)
	if (Game::Engine->GetCamera()->GetCameraState() == CameraClass::CameraState::NormalCamera)
	{
		// Pass control to the current player
		Game::CurrentPlayer->AcceptKeyboardInput(&Game::Keyboard);
	}
	else if (Game::Engine->GetCamera()->GetCameraState() == CameraClass::CameraState::DebugCamera)
	{
		// Otherwise if we are in debug camera mode, move the camera based upon keyboard input
		AcceptDebugCameraKeyboardInput();
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

	// Additional debug controls below this point
	if (b[DIK_U])
	{

	}
	if (b[DIK_5])
	{
		//s2()->AssignNewOrder(new Order_MoveToPosition(XMVectorAdd(s3[0]->GetPosition(), XMVectorSetZ(NULL_VECTOR, 400.0f)), 150.0f));
		s2()->AssignNewOrder(new Order_AttackBasic(s2(), s3[0]()));
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
			if (true)
			{
				Game::Console.ProcessRawCommand(GameConsoleCommand("debug_camera 1"));
				Game::Engine->GetCamera()->SetDebugCameraPosition(XMVectorSet(298.0f, 75.0f, 1463.0f, 0.0f));
				XMVECTOR orient = XMQuaternionNormalize(XMVectorSet(0.0546f, -0.99057f, 0.0973f, 0.4090f));
				XMFLOAT3 pos; XMStoreFloat3(&pos, cs()->GetPosition());
				XMFLOAT3 size; XMStoreFloat3(&size, cs()->GetSize());
				Game::Engine->GetCamera()->SetDebugCameraOrientation(orient);

				for (int x = 0; x < 10; ++x)
					for (int y = 0; y < 10; ++y)
					{
						SimpleShip *tmp = SimpleShip::Create("testship1");
						SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(tmp);
						XMFLOAT3 tmpsize; XMStoreFloat3(&tmpsize, tmp->GetSize());
						
						tmp->SetName("DIK_7_SPAWNED_SHIP");
						tmp->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"), XMVectorSet(
							pos.x + ((x - 5)*tmpsize.x*1.2f),
							pos.y + ((y - 5)*tmpsize.y*1.2f),
							pos.z + (size.z*0.5f) + 1000.0f, 0.0f));
						tmp->SetOrientation(ID_QUATERNION);

					}
			}
		}
		else
		{
			Game::ObjectRegister::iterator it_end = Game::Objects.end();
			for (Game::ObjectRegister::iterator it = Game::Objects.begin(); it != it_end; ++it)
			{
				if (it->second.Object && it->second.Object->GetName() == "DIK_7_SPAWNED_SHIP" && 
					it->second.Object->GetObjectType() == iObject::ObjectType::SimpleShipObject)
				{
					SimpleShip *s = (SimpleShip*)it->second.Object;
					XMFLOAT3 spos; XMStoreFloat3(&spos, s->GetPosition());
					s->CancelAllOrders();
					s->AssignNewOrder(new Order_MoveToPosition(XMLoadFloat3(&XMFLOAT3(
						spos.x + frand_lh(-2500.0f, 2500.0f),
						0.0f,
						spos.z + frand_lh(-2500.0f, 2500.0f))), 250.0f));
				}
			}
			
		}
		Game::Keyboard.LockKey(DIK_7);
	}

	if (b[DIK_8])
	{
		if (s2()->HasOrders()) s2()->CancelAllOrders();
		XMFLOAT3 tpos[10]; Order_MoveToPosition *orders[10];
		for (int i = 0; i<10; i++)
		{
			tpos[i] = XMFLOAT3(		((float)((float)rand() / (float)RAND_MAX) * 300.0f) + 200.0f,
									((float)((float)rand() / (float)RAND_MAX) * 300.0f),
									((float)((float)rand() / (float)RAND_MAX) * 3400.0f) - 400.0f);
			orders[i] = new Order_MoveToPosition(XMLoadFloat3(&tpos[i]), 100.0f);
			if (i > 0) orders[i]->Dependency = orders[i - 1]->ID;
			s2()->AssignNewOrder(orders[i]);
		}
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
		if (b[DIK_LSHIFT])
		{
			cs()->ForceRenderingOfInterior(false);
			cs()->GetSection(0)->Fade.FadeIn(1.0f);
		}
		else
		{
			cs()->ForceRenderingOfInterior(true);
			cs()->GetSection(0)->Fade.FadeToAlpha(1.0f, 0.25f);
		}
	}
	if (b[DIK_EQUALS])
	{
		if (ss()->Highlight.IsActive())
			ss()->Highlight.Deactivate();
		else
		{
			XMFLOAT4 options[6] = { XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
				XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) };
			ss()->Highlight.Activate();
			ss()->Highlight.SetColour(XMLoadFloat4(&options[(int)floor(frand_lh(0, 6))]));
		}

		Game::Keyboard.LockKey(DIK_EQUALS);
	}
	
	if (false && b[DIK_Y]) {
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
	}
	if (b[DIK_G])
	{
		CSLifeSupportTile *tile = (CSLifeSupportTile*)cs()->GetFirstTileOfType(D::TileClass::LifeSupport);
		for (int x = 0; x < cs()->GetElementSizeX(); ++x)
			for (int y = 0; y < cs()->GetElementSizeY(); ++y)
				Game::Engine->GetOverlayRenderer()->RenderElementOverlay(cs(), INTVECTOR3(x, y, 0),
				XMVectorSet(1.0f - (cs()->GetElementDirect(x, y, 0)->GetGravityStrength() / tile->Gravity.Value),
				cs()->GetElementDirect(x, y, 0)->GetGravityStrength() / tile->Gravity.Value, 0.0f, 0.0f), 0.2f);
	}

	if (false && b[DIK_U]) {
		if (b[DIK_LSHIFT])		{ a1()->Attributes[ActorAttr::A_RunSpeed].BaseValue -= 1.0f; a1()->RecalculateAttributes(); }
		else					{ a1()->Attributes[ActorAttr::A_RunSpeed].BaseValue += 1.0f; a1()->RecalculateAttributes(); }
	}
	if (false && b[DIK_I]) {
		Order_ActorMoveToPosition *o[3];
		a1()->CancelAllOrders();
		for (int i = 0; i<3; i++)
		{
			o[i] = new Order_ActorMoveToPosition(XMVectorSetY(XMVectorSetW(
				XMVectorAdd(Game::CurrentPlayer->GetPosition(), Vector3Random(-15.0f, 15.0f)), 0.0f), 0.0f),
				2.0f, false);
			if (i > 0) o[i]->Dependency = o[i - 1]->ID;
			a1()->AssignNewOrder(o[i]);
		}
		Game::Keyboard.LockKey(DIK_I);
	}

	if (b[DIK_1]) {
		Game::Keyboard.LockKey(DIK_1);
		cs()->SetTargetSpeedPercentage(1.0f);
	}
	if (b[DIK_2]) {
		Game::Keyboard.LockKey(DIK_2);
		Game::Console.ProcessRawCommand(GameConsoleCommand("render_obb 1"));
		Game::Console.ProcessRawCommand(GameConsoleCommand(concat("enter_ship_env ")(cs()->GetInstanceCode()).str()));
		Game::Console.ProcessRawCommand(GameConsoleCommand(concat("render_terrainboxes ")(cs()->GetInstanceCode())(" 1").str()));
	}
	if (b[DIK_3]) {
		Ship *parent = ss();
		for (int t = 0; t < parent->TurretController.TurretCount(); ++t)
		{
			parent->TurretController.Turrets[t]->Fire();
		}
	}
	if (b[DIK_4]) {
		Game::Engine->SetRenderFlag(CoreEngine::RenderFlag::DisableHullRendering, !b[DIK_LSHIFT]);
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
	wc.lpszClassName = TEXT("RJ-D3D11-Main");
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		int result = GetLastError();
		::MessageBox(0, TEXT("RegisterClassEx() - FAILED"), 0, 0);
		return false;
	}

	HWND hwnd = 0;
	hwnd = ::CreateWindowEx(m_wndstyleex, TEXT("RJ-D3D11-Main"), TEXT("RJ-D3D11-Main"),
		m_wndstyle, 0, 0, Game::FullWindowSize.x, Game::FullWindowSize.y,
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


Result RJMain::Initialise(HINSTANCE hinstance, WNDPROC wndproc)
{
	Result res;

	// Store the HINSTANCE and window procedures provided, for initialisation of the main application window
	m_hinstance = hinstance;
	m_wndproc = wndproc;

	// Record the name of and path to the current executable (TODO: Windows only)
	RetrieveExecutableData();

	// Initialise all component pointers to NULL so we can accurately set up and wind down the component set
	InitialiseComponentPointers();

	// Initialise the logging component first so we can output details of the initialisation.  Enable flushing after
	// every operation during the initialisation phase, to ensure that all data is output before any crash
	InitialiseLogging();
	Game::Log.EnableFlushAfterEveryOperation();

	// Initialise the central object registers
	Game::InitialiseObjectRegisters();

	// Load player config before initialising the application
	res = LoadPlayerConfig();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Could not load core configuration data [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Validate that the game data directory and all other dependencies exist before attempting to load data
	res = InitialiseGameDataDependencies();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Game data dependencies could not be initialised; check config.xml [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Create a new game window
	res = InitialiseWindow();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Could not initialise main window [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Attempt to initialise the game engine
	Game::Engine = new CoreEngine();
	res = Game::Engine->InitialiseGameEngine(m_hwnd);
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Could not initialise core game engine [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Initialise DirectInput
	res = InitialiseDirectInput();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: DirectInput initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Math functions
	InitialiseMathFunctions();
	Game::Log << LOG_INIT_START << "Math functions initialised\n";

	// Initialise the object manager; disable caching functions during initialisation while objects are being created in large numbers
	Game::ObjectManager.DisableSearchCache();

	// Initialise the universe
	res = InitialiseUniverse();
	if (res != ErrorCodes::NoError) {
		std::string errorstring = concat("Fatal Error: Universe initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Initialise key components of the user interface
	res = InitialiseUserInterface();
	if (res != ErrorCodes::NoError)
	{
		std::string errorstring = concat("Fatal Error: User interface initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
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
		Game::Log << LOG_INIT_START << "*** Error while loading game data file, hierarchy load terminated\n";
	}

	// Perform post-load initialisation of any data just loaded from game files
	InitialiseLoadedGameData();

	// Initialise the simulation state manager
	InitialiseStateManager();

	// *** DEBUG ***
	__CreateDebugScenario();

	// Initialise the active player object
	InitialisePlayer();

	// Initialise the primary region objects
	res = InitialiseRegions();
	if (res != ErrorCodes::NoError)
	{
		std::string errorstring = concat("Fatal Error: Region initialisation failed [")((int)res)("]").str();
		Game::Log << LOG_INIT_START << errorstring << "\n";
		::MessageBox(0, errorstring.c_str(), "Fatal Error", 0);
		return res;
	}

	// Enable the object manager cache now that objects have been created during initialisation
	Game::ObjectManager.EnableSearchCache();

	// Return success if we have completed all initialisation functions
	Game::Log << LOG_INIT_START << "Initialisation complete\n\n";
	Game::Log.FlushAllStreams();
	Game::Log.DisableFlushAfterEveryOperation();

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
		if (!DirectoryExists(D::DATA))
		{
			return ErrorCodes::AbsoluteDataPathIsNotValid;
		}
	}
	delete[] wcfull;

	// Derive other required application paths
	D::IMAGE_DATA_S = std::string(Game::ExePath + "\\Data\\ImageContent\\Data\\");
	D::IMAGE_DATA = D::IMAGE_DATA_S.c_str();

	// Make sure paths are valid
	if (!DirectoryExists(D::IMAGE_DATA)) 
	{
		return ErrorCodes::ImageResourcePathIsNotValid;
	}

	// Return success
	Game::Log << LOG_INIT_START << "Game data dependencies initialised successfully\n";
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
	Game::Log << LOG_INIT_START << "Game window initialised successfully\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseDirectInput()
{
	// Attempt to initialise DI, and return any error encountered
	Result result = GameInputSystem::Initialise(m_hinstance, m_hwnd, m_di, &Game::Keyboard, &Game::Mouse);
	if (result != ErrorCodes::NoError) return result;

	// Return success
	Game::Log << LOG_INIT_START << "DirectInput initialisation complete\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseRegions(void)
{
	Result res;

	// Initialise the immediate region, centred around the player
	D::Regions::Immediate = new ImmediateRegion();
	res = D::Regions::Immediate->Initialise(
		Game::Engine->GetDevice(),																		// Pointer to the D3D device
		BuildStrFilename(D::IMAGE_DATA, "Particles\\dust_particle.dds").c_str(),								// Texture to be mapped onto all dust particles
		Game::CurrentPlayer->GetPlayerShip()->GetPosition(),											// Starting centre point = player location
		XMVectorSetW(XMVectorReplicate(Game::C_IREGION_MIN), 0.0f),										// Minimum region bounds
		XMVectorSetW(XMVectorReplicate(Game::C_IREGION_BOUNDS), 0.0f),									// Maximum region bounds
		XMVectorSetW(XMVectorReplicate(Game::C_IREGION_THRESHOLD), 0.0f)								// Region update threshold
		);

	// If initialisation of the immediate region failed then return with an error here
	if (res != ErrorCodes::NoError) return res;

	// Initialise the system region, and set a default texture for now
	// TODO: THIS SHOULD COME FROM THE SYSTEM DATA FILE WHEN IT IS LOADED
	D::Regions::System = new SystemRegion();
	res = D::Regions::System->Initialise(Game::Engine->GetDevice());
	if (res != ErrorCodes::NoError) return res;

	// DEBUG: SET BACKDROP
	XMFLOAT2 texsize = XMFLOAT2(2048.0f, 1024.0f);
	if (true) D::Regions::System->SetBackdropTexture(Game::Engine->GetDevice(),
		BuildStrFilename(D::IMAGE_DATA, "Systems\\Omega\\omega_backdrop.dds").c_str(),
		texsize);


	// Return success if we have reached the end
	Game::Log << LOG_INIT_START << "Region initialisation completed\n";
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
	Game::Log << LOG_INIT_START << "Universe initialised successfully\n";
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
	Game::Log << LOG_INIT_START << "User interface initialisation complete\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialisePlayer(void)
{
	// Create a new player instance for the current player
	Game::CurrentPlayer = new Player();

	// Create a new human actor and assign it as the player character
	ActorBase *actorbase = D::Actors.Get("human_soldier_basic");
	if (!actorbase) return ErrorCodes::CouldNotCreatePlayerFromUndefinedActorBase;

	Game::CurrentPlayer->SetActor(actorbase->CreateInstance());
	Game::CurrentPlayer->GetActor()->SetName("Player actor");
	Game::CurrentPlayer->GetActor()->MoveIntoEnvironment(cs());

	// Place the player in a default (already created) ship and the current system
	Game::CurrentPlayer->SetPlayerShip(ss());
	Game::CurrentPlayer->EnterEnvironment(Game::Universe->GetSystem("AB01"));

	// Return success
	Game::Log << LOG_INIT_START << "Player initialisation completed\n";
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
	Game::Log << LOG_INIT_START << "Core data structured initialised\n";
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
	Game::Log << LOG_INIT_START << "Simulation state manager initialised\n";
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
	// Initialise complex ship element data
	ComplexShipElement::InitialiseStaticData();

	// Return success
	Game::Log << LOG_INIT_START << "Supporting ship data initialised\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseActorData(void)
{
	// Call the method for actor attribute-related setup
	ActorAttributeGeneration::InitialiseActorAttributeData();

	// Return success
	Game::Log << LOG_INIT_START << "Supporting actor data initialised\n";
	return ErrorCodes::NoError;
}

void RJMain::UpdateRegions(void)
{
	// Re-centre the immediate region on the player
	D::Regions::Immediate->RegionCentreMoved(Game::CurrentPlayer->GetPosition());
}

// Terminate all key game data structures (e.g. the space object Octree)
void RJMain::TerminateCoreDataStructures(void)
{
	// Move recursively down the tree, returning each node to the memory pool as we go.  The method to deallocate memory
	// pools must therefore be one of the final shutdown methods to ensure it incorporates items returned in other shutdown methods
	/*Game::SpatialPartitioningTree->Shutdown();
	Game::SpatialPartitioningTree = NULL;*/
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
	Octree<iSpaceObject*>::ShutdownMemoryPool();
}

// Load player config before initialising the application
Result RJMain::LoadPlayerConfig(void)
{
	// Record a log event when we begin loading game data files
	Game::Log << LOG_INIT_START << "Loading player configuration\n";

	// Attempt to load the main configuration file
	Result res = IO::Data::LoadConfigFile("./Config.xml");
	if (res != ErrorCodes::NoError)
	{
		Game::Log << LOG_INIT_START << "*** ERROR during load of configuration data [" << res << "]\n";
		return res;
	}

	// We have reached this point without a fatal error
	Game::Log << LOG_INIT_START << "All player configuration loaded successfully\n";
	return ErrorCodes::NoError;
}

Result RJMain::LoadAllGameData()
{
	Result res = ErrorCodes::NoError;

	// Record a log event when we begin loading game data files
	Game::Log << "\n" << LOG_INIT_START << "Loading game data hierarchy...\n";

	// Start at the primary index file that should already exist and work recursively outwards
	res = IO::Data::LoadGameDataFile("\\GameData.xml");
	if (res != ErrorCodes::NoError)
	{
		Game::Log << LOG_INIT_START << "*** ERROR during load of game data hierarchy [" << res << "]\n\n";
		return res;
	}

	// We have reached this point without a fatal error
	Game::Log << LOG_INIT_START << "All game data files loaded successfully\n\n";
	return ErrorCodes::NoError;
}

Result RJMain::InitialiseLoadedGameData(void)
{
	Result res;

	// Ships: Load all ship geometry, AFTER the ship data itself has been retrieved
	Game::Log << LOG_INIT_START << "Beginning load of all model geometry\n";
	res = IO::Data::LoadAllModelGeometry();
	if (res != ErrorCodes::NoError) return res;
	Game::Log << LOG_INIT_START << "Completed load of all model geometry\n";

	// Run post-processing of all model geometry as necessary
	res = IO::Data::PostProcessAllModelGeometry();
	if (res != ErrorCodes::NoError) return res;
	Game::Log << LOG_INIT_START << "Post-processing of all model geometry completed\n";

	// Resources: Run all post-load creation of the interdependencies between resources & their ingredients
	res = IO::Data::PostProcessResources();
	if (res != ErrorCodes::NoError) return res;
	Game::Log << LOG_INIT_START << "Post-processing of all resource data completed\n";

	// Ship tiles: Perform post-load initialisation of tiles and their construction requirements
	res = IO::Data::PostProcessComplexShipTileData();
	if (res != ErrorCodes::NoError) return res;
	Game::Log << LOG_INIT_START << "Post-processing of all tile data completed\n";

	// Universe: Process all systems in the universe, with data already loaded as part of LoadAllGameData
	Game::Universe->ProcessLoadedSystems(Game::Engine->GetDevice());
	Game::Log << LOG_INIT_START << "Post-processing of all system data completed\n";

	// Equipment: Process all equipment to run post-load initialisation
	Equipment::InitialiseLoadedEquipmentData();
	Game::Log << LOG_INIT_START << "Post-processing of all equipment data completed\n";

	// User interface: Build all UI layouts using the data loaded from game data files
	D::UI->BuildUILayouts();
	Game::Log << LOG_INIT_START << "Post-processing of all UI layouts completed\n";

	// Factions: initialise all faction data once game files have been processed
	Game::FactionManager.Initialise();
	Game::Log << LOG_INIT_START << "Post-processing of all faction data completed\n";

	// Return success
	return ErrorCodes::NoError;
}

void RJMain::DebugRenderSpaceCollisionBoxes(void)
{
	iSpaceObject *object;
	std::vector<iSpaceObject*> objects; int count = 0;

	// Find all active space objects around the player; take a different approach depending on whether the player is in a ship or on foot
	if (Game::CurrentPlayer->GetState() == Player::StateType::OnFoot)
	{
		// Player is on foot, so use a proximity test to the object currently considered their parent environment
		if (Game::CurrentPlayer->GetParentEnvironment() == NULL) return;
		count = 1 + Game::ObjectManager.GetAllObjectsWithinDistance(Game::CurrentPlayer->GetParentEnvironment(), 10000.0f, objects, 
			SimulationObjectManager::ObjectSearchOptions::OnlyCollidingObjects);

		// Also include the parent ship environmment(hence why we +1 to the count above)
		objects.push_back((iSpaceObject*)Game::CurrentPlayer->GetParentEnvironment());
	}
	else
	{
		// Player is in a spaceobject ship, so use the proximity test on their ship
		if (Game::CurrentPlayer->GetPlayerShip() == NULL) return;
		count = 1 + Game::ObjectManager.GetAllObjectsWithinDistance(Game::CurrentPlayer->GetPlayerShip(), 10000.0f, objects,
			SimulationObjectManager::ObjectSearchOptions::OnlyCollidingObjects);

		// Also include the player ship (hence why we +1 to the count above)
		objects.push_back((iSpaceObject*)Game::CurrentPlayer->GetPlayerShip());
	}

	// Iterate through the active objects
	XMFLOAT3 pos; float radius;
	std::vector<iSpaceObject*>::iterator it_end = objects.end();
	for (std::vector<iSpaceObject*>::iterator it = objects.begin(); it != it_end; ++it)
	{
		object = (*it);

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

void RJMain::__CreateDebugScenario(void)
{
	// Temp: Set the US/PRC factions to be hostile towards each other for testing purposes
	Game::FactionManager.FactionDispositionChanged(Game::FactionManager.GetFaction("faction_us"),
		Game::FactionManager.GetFaction("faction_prc"), Faction::FactionDisposition::Hostile);
	Game::FactionManager.FactionDispositionChanged(Game::FactionManager.GetFaction("faction_prc"),
		Game::FactionManager.GetFaction("faction_us"), Faction::FactionDisposition::Hostile);

	// Temp: Create a new ship for the player to use
	SimpleShip *ss_ship = SimpleShip::Create("testship1");
	ss_ship->SetName("Player ship ss");
	ss_ship->OverrideInstanceCode("ss");
	ss_ship->SetFaction(Game::FactionManager.GetFaction("faction_us"));
	ss_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"), XMVectorSet(600, 200, -200, 0.0f));
	ss_ship->SetOrientation(ID_QUATERNION);
	SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(ss_ship);
	ss = ss_ship;

	// Temp: Create a complex ship in this scenario
	if (true) {
		ComplexShip *cs_ship = ComplexShip::Create("testfrigate12");		// Previously 12
		cs_ship->SetName("Test frigate cs");
		cs_ship->GetSection(0)->SetName("cs section 0");
		cs_ship->SetFaction(Game::FactionManager.GetFaction("faction_prc"));
		cs_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"), XMVectorSet(-100, 0, 225, 0.0f));
		cs_ship->SetOrientation(ID_QUATERNION);
		cs = cs_ship;

		Engine *eng = (Engine*)D::Equipment.Get("FRIGATE_HEAVY_ION_ENGINE1");
		cs_ship->GetHardpoints().GetHardpointsOfType(Equip::Class::Engine).at(0)->MountEquipment(eng);
	}

	// Temp: Create a second ship in this scenario
	if (true) {
		SimpleShip *s2_ship = SimpleShip::Create("testship1");
		SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(s2_ship);
		s2_ship->SetName("Test ship s2");
		s2_ship->OverrideInstanceCode("Test ship s2");
		s2_ship->SetFaction(Game::FactionManager.GetFaction("faction_us"));
		s2_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"), XMVectorAdd(ss()->GetPosition(), XMVectorSet(0.0f, 0.0f, 120.0f, 0.0f)));
		s2_ship->SetOrientation(ID_QUATERNION);
		s2 = s2_ship;
	}


	if (true) {
		SimpleShip *s3_0_ship = SimpleShip::Create("testship1");
		SimpleShipLoadout::AssignDefaultLoadoutToSimpleShip(s3_0_ship);
		s3_0_ship->SetFaction(Game::FactionManager.GetFaction("faction_us"));
		s3_0_ship->MoveIntoSpaceEnvironment(Game::Universe->GetSystem("AB01"), XMVectorAdd(s2()->GetPosition(), XMVectorSet(0.0f, 0.0f, 100.0f, 0.0f)));
		s3_0_ship->SetOrientation(ID_QUATERNION);
		s3[0] = s3_0_ship;
	}

	// Temp: Create a new actor
	if (true)
	{
		Actor *a1_actor = D::Actors.Get("human_soldier_basic")->CreateInstance();
		a1_actor->SetName("A1");
		a1_actor->SetFaction(Game::FactionManager.GetFaction("faction_prc"));
		a1_actor->MoveIntoEnvironment(cs());
		ComplexShipTile *t = cs()->GetFirstTileOfType(D::TileClass::Corridor);
		if (t)
			a1_actor->SetEnvironmentPositionAndOrientation(XMVectorAdd(t->GetRelativePosition(),
			XMVectorSet(Game::C_CS_ELEMENT_SCALE*0.5f, Game::C_CS_ELEMENT_SCALE, Game::C_CS_ELEMENT_SCALE*0.5f, 0.0f)), ID_QUATERNION);
		else
			a1_actor->SetEnvironmentPositionAndOrientation(NULL_VECTOR, ID_QUATERNION);

		Order_ActorTravelToPosition *o = new Order_ActorTravelToPosition(cs(), a1_actor->GetEnvironmentPosition(), XMVectorSet(72.0f, 0.0f, 71.0f, 0.0f), 1.0f, 3.0f, false);
		a1_actor->AssignNewOrder(o);
		a1 = a1_actor;
	}

	SpaceProjectileDefinition *def = new SpaceProjectileDefinition();
	def->SetCode("tmp1");
	def->SetModel(Model::GetModel("unit_cone_model"));
	def->SetMass(25000.0f);
	def->SetDefaultLifetime(3.0f);

	/*turret = new SpaceTurret();
	turret->SetParent(ss);
	turret->InitialiseLaunchers(1);
	turret->SetMaxRange(1000.0f);
	turret->SetRelativePosition(D3DXVECTOR3(0.0f, 0.0f, ss()->GetCollisionSphereRadius()));	// TODO: *** FIX THIS.  Should not be in both turret & launcher ***
	turret->SetBaseRelativeOrientation(ID_QUATERNION);
	turret->SetPitchLimits(-PI / 4.0f, +PI / 4.0f);
	turret->SetPitchRate(0.1f);
	turret->SetYawLimitFlag(true);
	turret->SetYawLimits(-PI / 4.0f, +PI / 4.0f);;
	turret->SetYawRate(0.1f);


	ProjectileLauncher *l = turret->GetLauncher(0);
	l->SetProjectileDefinition(def);
	l->SetLaunchInterval(1000U);
	l->SetProjectileSpread(0.01f);
	l->SetLaunchImpulse(1000.0f);
	l->SetLaunchMethod(ProjectileLauncher::ProjectileLaunchMethod::SetVelocityDirect);
	l->SetRelativePosition(D3DXVECTOR3(0.0f, 0.0f, ss()->GetCollisionSphereRadius()));		// TODO: *** FIX THIS.  Should not be in both turret & launcher ***
	l->SetRelativeOrientation(ID_QUATERNION);
	l->SetProjectileSpin(1.0f);
	l->SetLinearVelocityDegradeState(false);
	l->SetAngularVelocityDegradeState(false);*/


	// Turret testing	
	/*dbg_turret = new SpaceTurret();
	dbg_turret->SetArticulatedModel(ArticulatedModel::GetModel("turret_basic01_model")->Copy());
	dbg_turret->SetRelativePosition(D3DXVECTOR3(0.0f, 10.0f, -5.0f));
	dbg_turret->SetBaseRelativeOrientation(ID_QUATERNION);
	dbg_turret->SetRange(1.0f, 10000.0f);									// NOT REQ
	dbg_turret->SetPitchLimits(-999.0f, 999.0f);
	dbg_turret->SetYawLimitFlag(false);
	dbg_turret->SetPitchRate(0.25f);
	dbg_turret->SetYawRate(0.5f);*/

	/*BasicProjectileDefinition *bdef = new BasicProjectileDefinition();
	bdef->SetProjectileSpeed(1000.0f);
	bdef->SetProjectileBeamLength(200.0f);
	bdef->SetProjectileBeamRadius(3.0f);
	bdef->SetProjectileColour(XMVectorSet(1.0f, 1.0f, 1.0f, 0.65f));
	bdef->SetProjectileLifetime(3000U);
	Texture *bt = new Texture();
	if (ErrorCodes::NoError == bt->Initialise(concat(D::IMAGE_DATA)("\\Rendering\\grad2.dds").str().c_str()))
	{
		bdef->SetTexture(bt);
	}
	bdef->GenerateProjectileRenderingData();*/

	D::BasicProjectiles.Get("basiclaser01")->AddDamageType(Damage(DamageType::Laser, 5.0f));
	s2()->SetMaxHealth(20.0f);
	s2()->SetHealth(20.0f);

	XMVECTOR rotleft = XMQuaternionRotationNormal(UP_VECTOR, -PI / 4.0f);
	XMVECTOR rotright = XMQuaternionRotationNormal(UP_VECTOR, PI / 4.0f);
	SpaceTurret *t = D::Turrets.Get("turret_basic01");
	XMFLOAT3 sz; XMStoreFloat3(&sz, cs()->GetSize());
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 2; ++j)
		{
			XMVECTOR pos = XMVectorSet((((float)j * (sz.x * 0.9f)) + (sz.x * 0.05f)), sz.y, (((((float)i + 1.0f) / 4.0f) * (sz.z * 0.9f)) + (sz.z * 0.05f)), 0.0f);
			pos = XMVectorSubtract(pos, XMVectorMultiply(cs()->GetSize(), HALF_VECTOR));
			SpaceTurret *nt = t->Copy();
			nt->SetRelativePosition(pos);
			nt->SetBaseRelativeOrientation((j == 0 ? rotleft : rotright));

			// Update to use basic proj
			for (int l = 0; l < nt->GetLauncherCount(); ++l)
			{
				nt->GetLauncher(l)->SetProjectileDefinition(D::BasicProjectiles.Get("basiclaser01"));
				nt->GetLauncher(l)->SetLaunchInterval(100U);
				nt->GetLauncher(l)->SetProjectileSpread(0.01f);
			}

			cs()->TurretController.AddTurret(nt);
			OutputDebugString(concat("Created turret ")(i)(" at ")(XMVectorGetX(pos))(",")(XMVectorGetY(pos))(",")(XMVectorGetZ(pos))("\n").str().c_str());
		}

	SpaceTurret *sst = D::Turrets.Get("turret_basic01")->Copy();
	for (int i = 0; i < sst->GetLauncherCount(); ++i)
	{
		sst->GetLauncher(i)->SetProjectileDefinition(D::BasicProjectiles.Get("basiclaser01"));
		sst->GetLauncher(i)->SetLaunchInterval(100U);
		sst->GetLauncher(i)->SetProjectileSpread(0.01f);
	}
	sst->SetRelativePosition(XMVectorSet(0, -20, 10.0f, 0));
	sst->SetBaseRelativeOrientation(ID_QUATERNION);
	sst->SetYawLimitFlag(true);
	sst->SetYawLimits(-0.1f, 0.1f);
	sst->SetPitchLimits(-0.1f, 0.1f);
	sst->SetYawRate(PI);
	sst->SetPitchRate(PI);
	sst->SetControlMode(SpaceTurret::ControlMode::AutomaticControl);
	ss()->TurretController.AddTurret(sst);
	SpaceTurret *sst2 = sst->Copy();
	s2()->TurretController.AddTurret(sst2);

	s2()->SetFaction(Game::FactionManager.GetFaction("faction_us"));
	//s2()->AssignNewOrder(new Order_MoveToTarget(cs, 100.0f));
	

	Game::Log << LOG_INIT_START << "--- Debug scenario created\n";
}

void RJMain::DEBUGDisplayInfo(void)
{
	// In-engine tests - display the tests if they have been activated
	if (m_debug_ccdspheretest) DebugCCDSphereTest();
	if (m_debug_ccdobbtest) DebugFullCCDTest(); // DebugCCDOBBTest();

	// Debug info line 1 - flight location & orientation
	if (m_debuginfo_flightstats)
	{
		const iActiveObject *a_obj = (Game::CurrentPlayer->GetState() == Player::StateType::OnFoot ? 
										(const iActiveObject*)Game::CurrentPlayer->GetActor() : 
										(const iActiveObject*)Game::CurrentPlayer->GetPlayerShip());
		if (a_obj)
		{
			// Get local float representations of this data
			XMFLOAT3 pos; XMStoreFloat3(&pos, Game::CurrentPlayer->GetPosition());
			XMFLOAT3 epos; XMStoreFloat3(&epos, Game::CurrentPlayer->GetEnvironmentPosition());
			const INTVECTOR3 & csepos = Game::CurrentPlayer->GetComplexShipEnvironmentElementLocation();
			XMFLOAT3 wm; XMStoreFloat3(&wm, a_obj->PhysicsState.WorldMomentum);

			// Output debug string
			sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_1, "Pos: (%.2f, %.2f, %.2f), EnvPos: (%.2f, %.2f, %.2f), El: (%d, %d, %d), LMom: (%.4f, %.4f, %.4f)",
					pos.x, pos.y, pos.z,
					epos.x, epos.y, epos.z,
					csepos.x, csepos.y, csepos.z,
					wm.x, wm.y, wm.z);
			Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_1, D::UI->TextStrings.C_DBG_FLIGHTINFO_1, 1.0f);
		}
	}

	// Debug info line 2 - engine rendering info
	if (m_debuginfo_renderinfo)
	{
		const CoreEngine::EngineRenderInfoData & renderinfo = Game::Engine->GetRenderInfo();
		sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_2, "Render Info: %d Draw Calls [%s %s %s %s %s %s ]",
			renderinfo.DrawCalls,
			(renderinfo.ShipRenderCount == 0 ? "" : concat(" S.Ship = ")(renderinfo.ShipRenderCount).str().c_str()),
			(renderinfo.ComplexShipRenderCount == 0 ? "" : concat(" C.Ship = ")(renderinfo.ComplexShipRenderCount).str().c_str()),
			(renderinfo.ComplexShipSectionRenderCount == 0 ? "" : concat(" CS.Sec = ")(renderinfo.ComplexShipSectionRenderCount).str().c_str()),
			(renderinfo.ComplexShipTileRenderCount == 0 ? "" : concat(" CS.Tile = ")(renderinfo.ComplexShipTileRenderCount).str().c_str()),
			(renderinfo.ActorRenderCount == 0 ? "" : concat(" Actor = ")(renderinfo.ActorRenderCount).str().c_str()),
			(renderinfo.TerrainRenderCount == 0 ? "" : concat(" Terrain = ")(renderinfo.TerrainRenderCount).str().c_str())
		);

		Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_2, D::UI->TextStrings.C_DBG_FLIGHTINFO_2, 1.0f);
	}

	// Debug info line 3 - object and collision data
	if (m_debuginfo_collisiondata)
	{
		const CollisionDetectionResultsStruct & coll = Game::PhysicsEngine.CollisionDetectionResults;
		sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_3, 
#			ifdef OBJMGR_DEBUG_MODE
				"Obj cache: Size = %d (Hit: %d / Miss: %d) | Collisions: Spc = (O/O:%d > B:%d > C:%d), Sp-CCD = (O/O:%d > C:%d), Env = (E:%d > O:%d > E[O]:%d > O/T:%d, O/O:%d > B:%d > C:%d)",
				Game::ObjectManager.GetCurrentCacheSize(), Game::ObjectManager.CACHE_HITS, Game::ObjectManager.CACHE_MISSES,
#			else
				"Obj cache: Size = %d | Collisions: Spc = (O/O:%d > B:%d > C:%d), Sp-CCD = (O/O:%d > C:%d), Env = (E:%d > O:%d > E[O]:%d > O/T:%d, O/O:%d > B:%d > C:%d)",
				Game::ObjectManager.GetCurrentCacheSize(), 
#			endif
			coll.SpaceCollisions.CollisionChecks, coll.SpaceCollisions.BroadphaseCollisions, coll.SpaceCollisions.Collisions,
			coll.SpaceCollisions.CCDCollisionChecks, coll.SpaceCollisions.CCDCollisions, 
			coll.EnvironmentCollisions.ElementsChecked, coll.EnvironmentCollisions.ObjectsChecked, coll.EnvironmentCollisions.ElementsCheckedAroundObjects,
			coll.EnvironmentCollisions.ObjectVsTerrainChecks, coll.EnvironmentCollisions.ObjectVsObjectChecks,
			coll.EnvironmentCollisions.BroadphaseCollisions, coll.EnvironmentCollisions.Collisions);
		Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_3, D::UI->TextStrings.C_DBG_FLIGHTINFO_3, 1.0f);
	}

	// Debug info line 4 - temporary debug data as required
	if (true)
	{
		if (s2())
		{
			std::string ordertype = "None";
			std::vector<Order*> orders;
			s2()->GetAllExecutingOrders(orders);
			if (orders.size() == 1)
				ordertype = Order::TranslateOrderTypeToString(orders[0]->GetType());
			else if (orders.size() > 1)
				ordertype = "(MULTIPLE)";

			sprintf(D::UI->TextStrings.C_DBG_FLIGHTINFO_4, "Health: %.2f / %.2f", s2()->GetHealth(), s2()->GetMaxHealth());
			Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_4, D::UI->TextStrings.C_DBG_FLIGHTINFO_4, 1.0f);
		}
	}

}




