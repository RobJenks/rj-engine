#include <vector>
#include <unordered_map>
#include "DX11_Core.h"
#include "time.h"
#include "RJMain.h"
#include "Ship.h"
#include "Actor.h"
#include "GameInput.h"
#include "CentralScheduler.h"
#include "GamePhysicsEngine.h"
#include "SimulationStateManager.h"
#include "SimulationObjectManager.h"
#include "FactionManagerObject.h"
#include "LogManager.h"
#include "GameConsole.h"
#include "DebugCommandHandler.h"
class CoreEngine;
class Player;
using namespace std;
using namespace std::tr1;

#include "GameVarsExtern.h"


#ifndef __GameVarsExtern_Types_H__
#define __GameVarsExtern_Types_H__	

namespace Game {

	typedef unsigned int									ID_TYPE;
	typedef unordered_map<Game::ID_TYPE, iSpaceObject*>		SpaceObjectRegister;
	typedef float											HitPoints;
};

#endif


namespace Game {

	// Main application instance
	RJMain Application;
	std::string ExeName = "";
	std::string ExePath = "";

	// The core game engine
	CoreEngine *Engine = NULL;

	// Gameplay settings
	static const float GameSpeed = 1.0f;												// Game speed multiplier, applied to every frame
	MouseInputControlMode MouseControlMode = MouseInputControlMode::MC_MOUSE_FLIGHT;

	// Persistent timers, that progress regardless of the state of the game simulation
	float PersistentClockTime = 0.0f;													// Time (secs)
	float PersistentTimeFactor = 0.0001f;												// Time (secs) passed since the previous frame
	XMVECTOR PersistentTimeFactorV = XMVectorReplicate(PersistentTimeFactor);			// Time (secs) passed since the previous frame (vectorised form)
	unsigned int PersistentClockMs = 0U;												// System time (ms)
	unsigned int PreviousPersistentClockMs = 0U;										// System time (ms) on the previous cycle
	unsigned int PersistentClockDelta = 0U;												// Delta time (ms) since the previous frame

	// Game timers, that stop and start as the game pause state is changed
	float ClockTime = 0.0f;																// Time (secs)
	float TimeFactor = 0.0001f;															// Time (secs) passed since the previous frame
	XMVECTOR TimeFactorV = XMVectorReplicate(TimeFactor);								// Time (secs) passed since the previous frame (vectorised form)
	unsigned int ClockMs = 0U;															// System time (ms)
	unsigned int PreviousClockMs = 0U;													// System time (ms) on the previous cycle
	unsigned int ClockDelta = 0U;														// Delta time (ms) since the previous frame

	// FPS and performance data
	float FPS = 0.0f;
	bool FPSDisplay = true;

	// Display settings; default values that will be overwritten by game config on load
	int ScreenWidth = 1024;
	int ScreenHeight = 768;
	int ScreenRefresh = 0;
	INTVECTOR2 ScreenCentre = INTVECTOR2(Game::ScreenWidth / 2, Game::ScreenHeight / 2);
	INTVECTOR2 FullWindowSize = NULL_INTVECTOR2;
	INTVECTOR2 WindowPosition = NULL_INTVECTOR2;
	bool FullScreen = false;
	bool ForceWARPRenderDevice = false;

	// Global object collection (TODO: in future, maintain only local objects in a collection so we don't run unnecessary simulation)
	Game::ObjectRegister					Objects(0);

	// Short-term lists of objects waiting to be registered or unregistered with the global collection; actioned each frame
	std::vector<Game::ID_TYPE>				UnregisterList(0);		// Stored as ID in case object is being unregistered while it is being shut down
	std::vector<iObject*>					RegisterList(0);		// Store the object so it can be inserted into the global game objects list
	

	// Central scheduler for all scheduled jobs
	CentralScheduler 				Scheduler = CentralScheduler();

	// State manager, which maintains the simulation state and level for all objects/systems/processes in the game
	SimulationStateManager			StateManager = SimulationStateManager();

	// Object manager, which handles operations across the set of active objects
	SimulationObjectManager			ObjectManager = SimulationObjectManager();

	// Faction manager, which maintains the central record of all faction and the relationships between them
	FactionManagerObject			FactionManager = FactionManagerObject();

	// Central logging component
	LogManager						Log = LogManager();

	// The game console, which processes all incoming console commands
	GameConsole						Console = GameConsole();
	DebugCommandHandler				DebugCommandManager = DebugCommandHandler();

	// Central components used to periodically prune all spatial partitioning trees to remove redundant nodes
	//OctreePruner<iSpaceObject*>	TreePruner = OctreePruner<iSpaceObject*>();

	// Collision manager, which performs all collision detection and response determination each cycle
	GamePhysicsEngine				PhysicsEngine = GamePhysicsEngine();

	// The game universe
	GameUniverse *					Universe;

	// Player data
	Player *						CurrentPlayer = NULL;		// Pointer to the current player

	// Global flag indicating whether the application is paused
	bool							Paused = false;

	// System constants
	const clock_t C_SECONDS_PER_CLOCK = (const clock_t)1.0 / (const clock_t)CLOCKS_PER_SEC;

	// General application constants
	int C_LOG_FLUSH_INTERVAL = 5000;
	unsigned int C_MAX_FRAME_DELTA = 200;					// Maximum frame delta of 200ms (= minimum 5 FPS)

	// File input/output constants
	const int C_DATA_LOAD_RECURSION_LIMIT = 50;				// Maximum recursion depth when loading data files, to prevent infinite loops
	const int C_CONFIG_LOAD_RECURSION_LIMIT = 25;			// Maximum recursion depth when loading config files, to prevent infinite loops

	// Rendering constants
	const int C_INSTANCED_RENDER_LIMIT = 1000;				// The maximum number of instances that can be rendered in any one draw call by the engine
	const float C_MODEL_SIZE_LIMIT = 10000.0f;				// The maximum size of any model; prevents overflow / accidental scaling to unreasonble values
	const int C_MAX_ARTICULATED_MODEL_SIZE = 128;			// The maximum number of components within any articulated model
	const unsigned int C_DEFAULT_RENDERQUEUE_CHECK_INTERVAL = 1000U;		// Time (ms) between pre-optimisation checks of the render queue
	const unsigned int C_DEFAULT_RENDERQUEUE_OPTIMISE_INTERVAL = 10000U;	// Time (ms) between optimisation passes on the render queue

	// Physics constants
	const float C_EPSILON = 1e-6f;
	const float C_EPSILON_NEG = (-C_EPSILON);
	const XMVECTOR C_EPSILON_V = XMVectorReplicate(C_EPSILON);
	const XMVECTOR C_EPSILON_NEG_V = XMVectorReplicate(C_EPSILON_NEG);
	const float C_MIN_PHYSICS_CYCLES_PER_SEC = 30.0f;								// The minimum number of physics cycles that should be run per 
																					// second (i.e. the physics FPS)
	const float C_MAX_PHYSICS_TIME_DELTA = (1.0f / C_MIN_PHYSICS_CYCLES_PER_SEC);	// The maximum permitted physics time delta, beyond which multiple 
																					// cycles will be run per frame
	const float C_MAX_PHYSICS_FRAME_CYCLES = 5.0f;									// The maximum number of phyiscs cycles permitted per render 
																					// frame, to avoid the 'spiral-of-death'
	float C_MOVEMENT_DRAG_FACTOR = 0.025f;
	float C_ENGINE_SIMULATION_UPDATE_INTERVAL = 0.1f;		// Interval (secs) between updates of the engine simulation
	bool C_NO_MOMENTUM_LIMIT = false;						// Determines whether the momentum safety limit is enabled
	float C_ANGULAR_VELOCITY_DAMPING_FACTOR = 0.01f;		// Damping applied to angular rotation, measured in rad/sec
	float C_COLLISION_SPACE_COEFF_ELASTICITY = 0.9f;		// Coefficient of elasticity for space-based collisions
	AXMVECTOR C_COLLISION_SPACE_COEFF_ELASTICITY_V = XMVectorReplicate(C_COLLISION_SPACE_COEFF_ELASTICITY);
	float C_MAX_LINEAR_VELOCITY = 10000.0f;					// Max linear velocity that any object can attain; to avoid overflows and other physics-based errors
	float C_MAX_ANGULAR_VELOCITY = 10.0f;					// Max angular velocity that any object can attain; to avoid overflows and other physics-based errors
	float C_ENVIRONMENT_MOVE_DRAG_FACTOR = 4.0f;			// The amount of x/z velocity that is lost by environment objects per second due to 'friction'
	float C_OBJECT_FAST_MOVER_THRESHOLD = 0.75f;			// The percentage of an object's extent that it has to move per frame to qualify as a "fast mover"
	const int C_MAX_INTRA_FRAME_CCD_COLLISIONS = 5;			// The maximum number of CCD collisions we support WITHIN a frame (e.g. multiple very fast ricochets)
	float C_PROJECTILE_VELOCITY_LIMIT = 5000.0f;					// Implement a universal limit (m/sec) on the velocity of projectiles, to avoid unexpectedly large calculated velocity
	float C_PROJECTILE_VELOCITY_LIMIT_SQ = 
		C_PROJECTILE_VELOCITY_LIMIT * C_PROJECTILE_VELOCITY_LIMIT;	// Squared universal velocity limit (m/sec) for projectiles 

	// Collision-detection constants
	const float C_ACTIVE_COLLISION_DISTANCE_SHIPLEVEL = 5000.0f;		// Distance within which collision detection is performed (in a ship context)
	const float C_ACTIVE_COLLISION_DISTANCE_ACTORLEVEL = 40.0f;			// Distance within which collision detection is performed (in an actor context)
	const float C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD = 1.0f;		// Threshold momentum value, above which we apply an additional collision response
	const float C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD_SQ =										// Squared threshold momentum value, above which
		C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD * C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD;	// we apply an additional collision response
	const AXMVECTOR C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD_V = XMVectorReplicate(C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD);
	const AXMVECTOR C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD_SQ_V = XMVectorReplicate(C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD_SQ);
	const int C_MAX_OBJECT_COLLISION_EXCLUSIONS = 256;					// The maximum number of collision exclusions that can be applied to an object
	const unsigned int C_STATIC_PAIR_CD_INTERVAL = 1000U;				// The interval (ms) between 'full' collision detection checks, where we also include static/static pairs
	
	// Object management constants
	const int C_OCTREE_MAX_NODE_ITEMS = 12;					// The target object limit per octree node; can be overriden if required
															// based on current node size
	const float C_OCTREE_MIN_NODE_SIZE = 250.0f;			// Minimum acceptable octree node size.  Overrides node count limit if required

	// Camera-related constants
	float C_DEFAULT_ZOOM_TO_SHIP_SPEED = 1.75f;				// Default number of seconds to zoom the camera from its current location to a ship
	float C_DEFAULT_ZOOM_TO_SHIP_OVERHEAD_DISTANCE = 75.0f;	// Default distance to place the camera above a ship, if it cannot be determined any other way
	float C_DEBUG_CAMERA_SPEED = 500.0f;					// The movement speed of the debug camera
	float C_DEBUG_CAMERA_TURN_SPEED = 100.0f;				// The turn speed of the debug camera
	float C_DEBUG_CAMERA_FAST_MOVE_MODIFIER = 10.0f;		// Modifier to debug camera move speed when fast-travel key is held
	float C_DEBUG_CAMERA_SLOW_MOVE_MODIFIER = 0.1f;			// Modifier to debug camera move speed when slow-travel key is held
	float C_DEBUG_CAMERA_ROLL_SPEED = 2.0f;					// The speed at which the debug camera will revert its roll component

	// Player-related contants
	float C_PLAYER_MOUSE_TURN_MODIFIER_YAW = 6.0f;			// Modifier to yaw speed (about Y) when using the mouse, i.e. the mouse sensitivity
	float C_PLAYER_MOUSE_TURN_MODIFIER_PITCH = 500.0f;		// Modifier to pitch speed (about X) when using the mouse, i.e. the mouse sensitivity
	float C_PLAYER_PITCH_MIN = PI * -0.4f;					// Minimum possible pitch of the player view (i.e. furthest extent we can look down)
	float C_PLAYER_PITCH_MAX = PI * 0.4f;					// Maximum possible pitch of the player view (i.e. furthest extent we can look up)
	float C_THRUST_INCREMENT_PC = 0.2f;						// Percentage of total thrust range that will be incremented/decremented by player throttle each second
	float C_MOUSE_FLIGHT_MULTIPLIER = 1.0f;					// Multiplier to avoid mouse flight requiring the full screen bounds to achieve min/max turning

	// Ship simulation, movement and manuevering constants
	float C_AI_DEFAULT_TURN_MODIFIER_PEACEFUL = 0.7f;			// Default turn modifier for ships when not in combat.  Can be overidden per ship/pilot
	float C_AI_DEFAULT_TURN_MODIFIER_COMBAT = 1.0f;				// Default turn modifier for ships when in combat.  Can be overidden per ship/pilot
	float C_DEFAULT_SHIP_CONTACT_ANALYSIS_RANGE = 5000.0f;		// The default range within which ships will analyse nearby contacts
	unsigned int C_DEFAULT_SHIP_CONTACT_ANALYSIS_FREQ = 1000U;	// Default interval between analysis of nearby contacts (ms)

	// Immediate Region-related data
	float C_IREGION_BOUNDS = 150.0f;				// The maximum bounds in each direction that we will extend the immediate region
	float C_IREGION_MIN = 10.0f;					// Min bounds for the I-Region, within which we will not create new particles
	float C_IREGION_THRESHOLD = 25.0f;				// Update threshold for the I-Region
	float C_DUST_PARTICLE_BLEND_RATE = 0.25f;		// The percentage of total colour blended in each second as particles are created

	// Complex ship constants
	float C_CS_ELEMENT_SCALE = 10.0f;													// The physical size of each CS element in space
	float C_CS_ELEMENT_MIDPOINT = C_CS_ELEMENT_SCALE * 0.5f;							// Midpoint of an element in each dimension
	float C_CS_ELEMENT_SCALE_RECIP = (1.0f / C_CS_ELEMENT_SCALE);						// Reciprocal of the element scale (1.0f/scale)
	XMVECTOR C_CS_ELEMENT_SCALE_V = XMVectorReplicate(C_CS_ELEMENT_SCALE);				// The physical size of each CS element in space (replicated vector form)
	XMVECTOR C_CS_ELEMENT_MIDPOINT_V = XMVectorReplicate(C_CS_ELEMENT_MIDPOINT);		// Midpoint of an element in each dimension  (replicated vector form)
	XMVECTOR C_CS_ELEMENT_SCALE_RECIP_V = XMVectorReplicate(C_CS_ELEMENT_SCALE_RECIP);	// Reiprocal of the element scale (1.0f/scale)  (replicated vector form)
	float C_CS_PERIMETER_BEACON_FREQUENCY = 400.0f;										// The (approx, max) spacing between perimeter beacons on a capital ship
	XMVECTOR C_CS_PERIMETER_BEACON_FREQUENCY_V = 
		XMVectorReplicate(C_CS_PERIMETER_BEACON_FREQUENCY);								// The (approx, max) spacing between perimeter beacons on a capital ship (vectorised)

	// AI, order management and ship computer constants
	float C_DEFAULT_FLIGHT_COMPUTER_EVAL_INTERVAL = 0.1f;		// The default interval for evaluation by the ship flight computer
	float C_DEFAULT_ORDER_EVAL_FREQUENCY = 0.5f;				// The default interval for subsequent evaluations of an order by the AI
	float C_DEFAULT_ORDER_QUEUE_MAINTENANCE_FREQUENCY = 5.0f;	// Default interval for entity maintenance of its order queue
	float C_ENGINE_THRUST_DECREASE_THRESHOLD = 0.9f;			// % threshold of target speed at which we start to reduce engine thrust

	// Actor-related constants
	float C_ACTOR_DEFAULT_RUN_SPEED = 18.0f;									// Default run speed for all actors unless specified
	float C_ACTOR_DEFAULT_WALK_MULTIPLIER = 0.5f;								// Default multiplier to convert run speed to walk speed
	float C_ACTOR_DEFAULT_STRAFE_MULTIPLIER = 0.75f;							// Default multiplier to convert run speed to strafe speed
	float C_ACTOR_DEFAULT_TURN_RATE = 5.0f;										// Default turn rate in radians/sec
	float C_ACTOR_DEFAULT_HEAD_BOB_SPEED = 1.5f;								// Default speed of the player head bob when controlling an actor
	float C_ACTOR_DEFAULT_HEAD_BOB_AMOUNT = 0.2f;								// Default height that the player head bob will reach when controlling an actor
	float C_ACTOR_DEFAULT_JUMP_STRENGTH = 1.0f;									// Default jump strength, as a modifier relative to the actor mass (so resulting
																				// in a force of equal strength regardless of mass)
	XMFLOAT3 C_ACTOR_DEFAULT_VIEW_OFFSET = XMFLOAT3(0.0f, 5.0f, 0.0f);			// Default offset of the player view when controlling an actor, 
																				// if not set directly by the actor

	// Pathfinding constants
	int C_CS_ELEMENT_MIDPOINT_INT = (int)C_CS_ELEMENT_MIDPOINT;						// Integer rounded value for midpoint of an element (for efficiency)
	int C_NAVNETWORK_TRAVERSE_COST = (int)C_CS_ELEMENT_SCALE;						// Cost of moving from one element to another (non-diagonal)
	int C_NAVNETWORK_TRAVERSE_COST_DIAG = (int)(C_CS_ELEMENT_SCALE * 1.41421356f);	// Diagonal traversal cost; equal to normal cost * SQRT(2)

	// Simulation constants
	int C_SIMULATION_STATE_MANAGER_UPDATE_INTERVAL = 2000;							// Interval between each evaluation of the state manager (ms)
	float C_SPACE_SIMULATION_HUB_RADIUS = 5000.0f;									// Distance within which objects are fully simulated by a hub
	float C_SPACE_SIMULATION_HUB_RADIUS_SQ	= C_SPACE_SIMULATION_HUB_RADIUS			// Squared distance within which objects are fully simulated by a hub
											* C_SPACE_SIMULATION_HUB_RADIUS;
	AXMVECTOR C_SPACE_SIMULATION_HUB_RADIUS_SQ_V = 
						XMVectorReplicate(C_SPACE_SIMULATION_HUB_RADIUS_SQ);		// Vectorised version of the sq distance within which objects are fully simulated by a hub


	// Turret simulation and controller constants
	float C_MIN_TURRET_RANGE = 1.0f;												// Minimum distance within which a turret can fire at a target
	float C_MAX_TURRET_RANGE = 100000.0f;											// Maximum distance within which a turret can fire at a target
	unsigned int C_PROJECTILE_OWNER_DETACH_PERIOD = 1000U;							// Period within which a projectile is protected from colliding with its owner (ms)
	int C_MAX_TURRET_LAUNCHERS = 128;												// The maximum number of launchers that a single turret can hold
	float C_DEFAULT_FIRING_REGION_THRESHOLD = 0.05f;								// Deviation in pitch/yaw within which a turret will start firing at the target

	// Default tile simulation values
	unsigned int C_TILE_LIFESUPPORT_SIMULATION_INTERVAL = 100U;						// Life support tiles will be simulated every 100ms, when active

	// Translate collision mode values to/from their string representation
	CollisionMode TranslateCollisionModeFromString(const std::string & mode)
	{
		std::string s = mode; StrLowerC(s);
		if (mode == "fullcollision")
			return CollisionMode::FullCollision;
		else if (mode == "broadphasecollisiononly")
			return CollisionMode::BroadphaseCollisionOnly;
		else
			return CollisionMode::NoCollision;
	}

	// Translate collision mode values to/from their string representation
	std::string TranslateCollisionModeToString(CollisionMode mode)
	{
		switch (mode)
		{
			case CollisionMode::FullCollision:				return "fullcollision";
			case CollisionMode::BroadphaseCollisionOnly:	return "broadphasecollisiononly";
			default:										return "nocollision";
		}
	}

	// Translate collider type values to/from their string representation
	ColliderType TranslateColliderTypeFromString(const std::string & type)
	{
		std::string s = type; StrLowerC(s);
		if (s == "passivecollider")
			return ColliderType::PassiveCollider;
		else
			return ColliderType::ActiveCollider;
	}

	// Translate collision mode values to/from their string representation
	std::string TranslateColliderTypeToString(ColliderType type)
	{
		switch (type)
		{
			case ColliderType::PassiveCollider:
				return "passivecollider"; break;
			default:
				return "activecollider"; break;
		}
	}

	// Find an object based on its string code
	iObject *FindObjectInGlobalRegister(const std::string & instance_code)
	{
		// Hash the desired code (assuming it is valid) for more efficient comparisons
		if (instance_code == NullString) return NULL;
		HashVal hash = HashString(instance_code);

		// Compare hash values for more efficient comparison
		ObjectRegister::const_iterator it_end = Game::Objects.end();
		for (ObjectRegister::const_iterator it = Game::Objects.begin(); it != it_end; ++it)
			if ((it->second.Active) && (it->second.Object) && (it->second.Object)->GetInstanceCodeHash() == hash) return (it->second.Object);

		return NULL;
	}

	// Find an object based on its instance hash
	iObject *FindObjectInGlobalRegister(const HashVal & instance_hash)
	{
		// Compare hash values for more efficient comparison
		ObjectRegister::const_iterator it_end = Game::Objects.end();
		for (ObjectRegister::const_iterator it = Game::Objects.begin(); it != it_end; ++it)
			if ((it->second.Active) && (it->second.Object) && (it->second.Object)->GetInstanceCodeHash() == instance_hash) return (it->second.Object);

		return NULL;
	}
};





