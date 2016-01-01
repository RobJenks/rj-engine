#pragma once

#ifndef __GameVarsExternH__
#define __GameVarsExternH__

#include <unordered_map>
#include "HashFunctions.h"
class iObject;


#ifndef __GameVarsExtern_Types_H__
#define __GameVarsExtern_Types_H__	

// This file contains no objects with special alignment requirements
namespace Game {

	typedef long												ID_TYPE;
	typedef float												HitPoints;
};

#endif

#include "time.h"
#include <cstdlib>
#include <vector>

#include "Utility.h"
#include "GameInput.h"
#include "CentralScheduler.h"
class RJMain;
class CoreEngine;
class Ship;
class Actor;
class Player;
class GamePhysicsEngine;
class SimulationStateManager;
class FactionManagerObject;
class LogManager;
class GameConsole;
class DebugCommandHandler;
class GameUniverse;


// This file contains no objects with special alignment requirements
namespace Game {
	
	// Main application instance
	extern RJMain Application;
	extern std::string ExeName;
	extern std::string ExePath;

	// The core game engine
	extern CoreEngine *Engine;

	// Gameplay settings
	extern const float GameSpeed;						// Game speed multiplier, applied to every frame
	extern MouseInputControlMode MouseControlMode;

	// Persistent timers, that progress regardless of the state of the game simulation
	extern float PersistentClockTime;					// Time (secs)
	extern float PersistentTimeFactor;					// Time (secs) passed since the previous frame
	extern XMVECTOR PersistentTimeFactorV;				// Time (secs) passed since the previous frame (vectorised form)
	extern unsigned int PersistentClockMs;				// System time (ms)
	extern unsigned int PreviousPersistentClockMs;		// System time (ms) on the previous cycle
	extern unsigned int PersistentClockDelta;			// Delta time (ms) since the previous frame

	// Game timers, that stop and start as the game pause state is changed
	extern float ClockTime;								// Time (secs)
	extern float TimeFactor;							// Time (secs) passed since the previous frame
	extern XMVECTOR TimeFactorV;						// Time (secs) passed since the previous frame (vectorised form)
	extern unsigned int ClockMs;						// System time (ms)
	extern unsigned int PreviousClockMs;				// System time (ms) on the previous cycle
	extern unsigned int ClockDelta;						// Delta time (ms) since the previous frame

	// Overall speed & performance variables
	extern float FPS;
	extern bool FPSDisplay;

	// Display settings
	extern int ScreenWidth;
	extern int ScreenHeight;
	extern int ScreenRefresh;
	extern INTVECTOR2 ScreenCentre;
	extern INTVECTOR2 FullWindowSize;
	extern INTVECTOR2 WindowPosition;
	extern bool FullScreen;
	extern bool ForceWARPRenderDevice;

	// Central scheduler for all scheduled jobs
	extern CentralScheduler Scheduler;

	// State manager, which maintains the simulation state and level for all objects/systems/processes in the game
	extern SimulationStateManager StateManager;

	// Faction manager, which maintains the central record of all faction and the relationships between them
	extern FactionManagerObject FactionManager;

	// Central logging component
	extern LogManager Log;

	// The game console, which processes all incoming console commands
	extern GameConsole Console;
	extern DebugCommandHandler DebugCommandManager;

	// Collision manager, which performs all collision detection and response determination each cycle
	extern GamePhysicsEngine PhysicsEngine;

	// The game universe
	extern GameUniverse	*Universe;

	// The current player
	extern Player *CurrentPlayer;

	// Primary input controllers for the application
	extern GameInputDevice	Keyboard;
	extern GameInputDevice	Mouse;

	// Global flag indicating whether the application is paused
	extern bool Paused;

	// Enumeration of possible object collision modes
	enum CollisionMode { FullCollision = 0, BroadphaseCollisionOnly, NoCollision };
	extern CollisionMode TranslateCollisionModeFromString(const std::string & mode);
	extern std::string TranslateCollisionModeToString(CollisionMode mode);

	// Enumeration of possible bounding volume types for game objects
	enum BoundingVolumeType { BoundingSphere = 0, OrientedBoundingBox };

	// Enumeration of possible collider types; passive colliders will not test for collisions themselves; 
	// they can only be the candidate object in a collision test.  They will also not perform CCD, so 
	// potentially-fast moving objects should be classified as active colliders
	enum ColliderType { ActiveCollider = 0, PassiveCollider };
	extern ColliderType TranslateColliderTypeFromString(const std::string & type);
	extern std::string TranslateColliderTypeToString(ColliderType type);

	// General application constants
	extern int C_LOG_FLUSH_INTERVAL;
	extern unsigned int C_MAX_FRAME_DELTA;				// Maximum frame delta (ms)

	// File input/output constants
	extern const int C_DATA_LOAD_RECURSION_LIMIT;		// Maximum recursion depth when loading data files, to prevent infinite loops
	extern const int C_CONFIG_LOAD_RECURSION_LIMIT;		// Maximum recursion depth when loading config files, to prevent infinite loops

	// Rendering constants
	extern const int C_INSTANCED_RENDER_LIMIT;		// The maximum number of instances that can be rendered in any one draw call by the engine
	extern const float C_MODEL_SIZE_LIMIT;			// The maximum size of any model; prevents overflow / accidental scaling to unreasonble values
	extern const int C_MAX_ARTICULATED_MODEL_SIZE;	// The maximum number of components within any articulated model
	extern const unsigned int C_DEFAULT_RENDERQUEUE_CHECK_INTERVAL;		// Time (ms) between pre-optimisation checks of the render queue
	extern const unsigned int C_DEFAULT_RENDERQUEUE_OPTIMISE_INTERVAL;	// Time (ms) between optimisation passes on the render queue

	// Physics constants
	extern const float C_EPSILON;
	extern const float C_EPSILON_NEG;
	extern const XMVECTOR C_EPSILON_V;
	extern const XMVECTOR C_EPSILON_NEG_V;
	extern const double C_EPSILON_DP;
	extern const float C_MAX_PHYSICS_TIME_DELTA;		// The maximum permitted physics time delta, beyond which multiple cycles will be run per frame
	extern const float C_MIN_PHYSICS_CYCLES_PER_SEC;	// The minimum number of physics cycles that should be run per second (i.e. the physics FPS)
	extern const float C_MAX_PHYSICS_FRAME_CYCLES;		// The maximum number of phyiscs cycles permitted per render frame, to avoid the 'spiral-of-death'
	extern float C_MOVEMENT_DRAG_FACTOR;
	extern float C_ENGINE_SIMULATION_UPDATE_INTERVAL;
	extern bool C_NO_MOMENTUM_LIMIT;
	extern float C_ANGULAR_VELOCITY_DAMPING_FACTOR;
	extern float C_COLLISION_SPACE_COEFF_ELASTICITY;
	extern AXMVECTOR C_COLLISION_SPACE_COEFF_ELASTICITY_V;
	extern float C_MAX_LINEAR_VELOCITY;
	extern float C_MAX_ANGULAR_VELOCITY;
	extern float C_ENVIRONMENT_MOVE_DRAG_FACTOR;
	extern float C_OBJECT_FAST_MOVER_THRESHOLD;			// The threshold beyond which we require an object to perform swept- rather than discrete-collision detection
	extern const int C_MAX_INTRA_FRAME_CCD_COLLISIONS;	// The maximum number of CCD collisions we support WITHIN a frame (e.g. multiple very fast ricochets)
	extern const unsigned int C_STATIC_PAIR_CD_INTERVAL;// The interval (ms) between 'full' collision detection checks, where we also include static/static pairs
	extern float C_PROJECTILE_VELOCITY_LIMIT;			// Implement a universal limit on the velocity of projectiles, to avoid unexpectedly large calculated velocity
	extern float C_PROJECTILE_VELOCITY_LIMIT_SQ;		// Squared universal velocity limit for projectiles 
	
	// Collision-detection constants
	extern const float C_ACTIVE_COLLISION_DISTANCE_SHIPLEVEL;			// Distance within which collision detection is performed (in a ship context)
	extern const float C_ACTIVE_COLLISION_DISTANCE_ACTORLEVEL;			// Distance within which collision detection is performed (in an actor context)
	extern const float C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD;		// Threshold momentum value, above which we apply an additional collision response
	extern const float C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD_SQ;	// Squared threshold momentum value, above which we apply an additional collision response
	extern const AXMVECTOR C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD_V;	// Threshold momentum value, above which we apply an additional collision response (vectorised form)
	extern const AXMVECTOR C_ENVIRONMENT_COLLISION_RESPONSE_THRESHOLD_SQ_V;	// Squared threshold momentum value, above which we apply an additional collision response (vectorised form)
	extern const int C_MAX_OBJECT_COLLISION_EXCLUSIONS;					// The maximum number of collision exclusions that can be applied to an object
	extern const float C_CONSTANT_COLLISION_AVOIDANCE_RANGE_MULTIPLIER;	// Constant multiplier applied to collision range when performing collision avoidance
	extern const float C_COLLISION_AVOIDANCE_RESPONSE_SAFETY_MULTIPLIER;	// Safety multiplier on the minimum required response to avoid a collision
	extern const float C_OBB_SIZE_THRESHOLD;							// The size at which we begin considering OBB as a better alternative to bounding sphere for the object
	extern const float C_OBB_SIZE_RATIO_THRESHOLD;						// The size ratio at which we begin using OBB rather than bounding sphere, due to better accuracy

	// Object management constants
	extern const int C_OCTREE_MAX_NODE_ITEMS;							// The target object limit per octree node; can be overriden if required
																		// based on current node size
	extern const float C_OCTREE_MIN_NODE_SIZE;							// Minimum acceptable octree node size.  Overrides node count limit if required

	// Camera-related constants
	extern float C_DEFAULT_ZOOM_TO_SHIP_SPEED;				// Default number of seconds to zoom the camera from its current location to a ship
	extern float C_DEFAULT_ZOOM_TO_SHIP_OVERHEAD_DISTANCE;	// Default distance to place the camera above a ship, if it cannot be determined any other way
	extern float C_DEBUG_CAMERA_SPEED;						// The movement speed of the debug camera
	extern float C_DEBUG_CAMERA_TURN_SPEED;					// The turn speed of the debug camera
	extern float C_DEBUG_CAMERA_FAST_MOVE_MODIFIER;			// Modifier to debug camera move speed when fast-travel key is held
	extern float C_DEBUG_CAMERA_SLOW_MOVE_MODIFIER;			// Modifier to debug camera move speed when slow-travel key is held
	extern float C_DEBUG_CAMERA_ROLL_SPEED;					// The speed at which the debug camera will revert its roll component

	// Player-related contants
	extern float C_PLAYER_MOUSE_TURN_MODIFIER_YAW;		// Modifier to yaw speed (about Y) when using the mouse, i.e. the mouse sensitivity
	extern float C_PLAYER_MOUSE_TURN_MODIFIER_PITCH;	// Modifier to pitch speed (about X) when using the mouse, i.e. the mouse sensitivity
	extern float C_PLAYER_PITCH_MIN;		// Minimum possible pitch of the player view (i.e. furthest extent we can look down)
	extern float C_PLAYER_PITCH_MAX;		// Maximum possible pitch of the player view (i.e. furthest extent we can look up)
	extern float C_THRUST_INCREMENT_PC;		// Percentage of total thrust range that will be incremented/decremented by player throttle each second
	extern float C_MOUSE_FLIGHT_MULTIPLIER;	// Multiplier to avoid mouse flight requiring the full screen bounds to achieve min/max turning

	// Ship simulation, movement and manuevering constants
	extern float C_AI_DEFAULT_TURN_MODIFIER_PEACEFUL;			// Default turn modifier for ships when not in combat.  Can be overidden per ship/pilot
	extern float C_AI_DEFAULT_TURN_MODIFIER_COMBAT;				// Default turn modifier for ships when in combat.  Can be overidden per ship/pilot
	extern float C_DEFAULT_SHIP_CONTACT_ANALYSIS_RANGE;			// The default range within which ships will analyse nearby contacts
	extern unsigned int C_DEFAULT_SHIP_CONTACT_ANALYSIS_FREQ;	// Default interval between analysis of nearby contacts (ms)

	// Immediate Region-related data
	extern float C_IREGION_BOUNDS;
	extern float C_IREGION_MIN;
	extern float C_IREGION_THRESHOLD;
	extern float C_DUST_PARTICLE_BLEND_RATE;
	
	// Complex ship constants
	extern float C_CS_ELEMENT_SCALE;				// The physical size of each CS element in space
	extern float C_CS_ELEMENT_MIDPOINT;				// Midpoint of an element in each dimension
	extern float C_CS_ELEMENT_SCALE_RECIP;			// Reiprocal of the element scale (1.0f/scale)
	extern XMVECTOR C_CS_ELEMENT_SCALE_V;			// The physical size of each CS element in space (replicated vector form)
	extern XMVECTOR C_CS_ELEMENT_MIDPOINT_V;		// Midpoint of an element in each dimension  (replicated vector form)
	extern XMVECTOR C_CS_ELEMENT_SCALE_RECIP_V;		// Reiprocal of the element scale (1.0f/scale)  (replicated vector form)

	extern float C_CS_PERIMETER_BEACON_FREQUENCY;		// The (approx, max) spacing between perimeter beacons on a capital ship
	extern XMVECTOR C_CS_PERIMETER_BEACON_FREQUENCY_V;	// The (approx, max) spacing between perimeter beacons on a capital ship (vectorised)

	// AI, order management and ship computer constants
	extern unsigned int C_DEFAULT_ENTITY_AI_EVAL_INTERVAL;				// The default interval for evaluation of current situation by an entity AI
	extern unsigned int C_DEFAULT_FLIGHT_COMPUTER_EVAL_INTERVAL;		// The default interval for evaluation by the ship flight computer
	extern unsigned int C_DEFAULT_ORDER_EVAL_FREQUENCY;					// The default interval for subsequent evaluations of an order by the AI
	extern unsigned int C_DEFAULT_ORDER_QUEUE_MAINTENANCE_FREQUENCY;	// Default interval for entity maintenance of its order queue

	extern float C_ENGINE_THRUST_DECREASE_THRESHOLD;
	extern float C_DEFAULT_ATTACK_CLOSE_TIME;					// Close distance will be this many seconds at full velocity from target
	extern float C_DEFAULT_ATTACK_CLOSE_RADIUS_MULTIPLIER;		// Multiple of target collision sphere radius that we will account for when attacking, by default
	extern float C_DEFAULT_ATTACK_RETREAT_TIME;					// Retreat distance between attack runs will be this many seconds at full velocity, added to close dist
	extern float C_ATTACK_TAIL_FOLLOW_THRESHOLD;				// We will attempt to get 'on the tail' of ships travelling more than (this) % of our velocity limit
	extern float C_DEFAULT_FLEE_DISTANCE;						// Default distance we will attempt to flee from enemies, if in 'flee' mode
	extern unsigned int C_TARGET_LEADING_RECALC_INTERVAL;		// Ships will recalculate their target leading distance every interval (ms)

	// Actor-related constants
	extern float C_ACTOR_DEFAULT_RUN_SPEED;						// Default run speed for all actors unless specified
	extern float C_ACTOR_DEFAULT_WALK_MULTIPLIER;				// Default multiplier to convert run speed to walk speed
	extern float C_ACTOR_DEFAULT_STRAFE_MULTIPLIER;				// Default multiplier to convert run speed to strafe speed
	extern float C_ACTOR_DEFAULT_TURN_RATE;						// Default turn rate in radians/sec
	extern float C_ACTOR_DEFAULT_HEAD_BOB_SPEED;				// Default speed of the player head bob when controlling an actor
	extern float C_ACTOR_DEFAULT_HEAD_BOB_AMOUNT;				// Default height that the player head bob will reach when controlling an actor
	extern float C_ACTOR_DEFAULT_JUMP_STRENGTH;					// Default jump strength, as a modifier relative to the actor mass (so resulting
																// in a force of equal strength regardless of mass)
	extern XMFLOAT3 C_ACTOR_DEFAULT_VIEW_OFFSET;				// Default offset of the player view when controlling an actor, if not set directly by the actor

	// Pathfinding constants
	extern int C_CS_ELEMENT_MIDPOINT_INT;						// Integer rounded value for midpoint of an element (for efficiency)
	extern int C_NAVNETWORK_TRAVERSE_COST;						// Cost of moving from one element to another (non-diagonal)
	extern int C_NAVNETWORK_TRAVERSE_COST_DIAG;					// Diagonal traversal cost; equal to normal cost * SQRT(2)

	// Simulation constants
	extern int C_SIMULATION_STATE_MANAGER_UPDATE_INTERVAL;		// Interval between each evaluation of the state manager (ms)
	extern float C_SPACE_SIMULATION_HUB_RADIUS;					// Distance within which objects are fully simulated by a hub
	extern float C_SPACE_SIMULATION_HUB_RADIUS_SQ;				// Squared distance within which objects are fully simulated by a hub
	extern AXMVECTOR C_SPACE_SIMULATION_HUB_RADIUS_SQ_V;		// Vectorised version of the sq distance within which objects are fully simulated by a hub

	// Turret simulation and controller constants
	extern float C_MIN_TURRET_RANGE;							// Minimum distance within which a turret can fire at a target
	extern float C_MAX_TURRET_RANGE;							// Maximum distance within which a turret can fire at a target
	extern unsigned int C_PROJECTILE_OWNER_DETACH_PERIOD;		// Period within which a projectile is protected from colliding with its owner (ms)
	extern int C_MAX_TURRET_LAUNCHERS;							// The maximum number of launchers that a single turret can hold
	extern float C_DEFAULT_FIRING_REGION_THRESHOLD;				// Deviation in pitch/yaw within which a turret will start firing at the target

	// Default tile simulation values
	extern unsigned int C_TILE_LIFESUPPORT_SIMULATION_INTERVAL;

	// Convert from grid location to physical position in 3D space
	CMPINLINE float						ElementLocationToPhysicalPosition(int location) 
										{ return ((float)location * Game::C_CS_ELEMENT_SCALE); }
	
	// Convert from three dimensional element location to position in 3D space
	CMPINLINE XMVECTOR					ElementLocationToPhysicalPosition(const INTVECTOR3 & location)
	{
		// NOTE: y & z coordinates are swapped when moving between grid & physical space, due to the coordinate systems used in each
		return XMVectorMultiply(XMVectorSet((float)location.x, (float)location.z, (float)location.y, 0.0f), Game::C_CS_ELEMENT_SCALE_V);
	}

	// Convert from three dimensional element location to position in 3D space, returning a float representation of the resulting vector
	CMPINLINE XMFLOAT3					ElementLocationToPhysicalPositionF(const INTVECTOR3 & location)
	{
		// NOTE: y & z coordinates are swapped when moving between grid & physical space, due to the coordinate systems used in each
		return XMFLOAT3((float)location.x * Game::C_CS_ELEMENT_SCALE,
						(float)location.z * Game::C_CS_ELEMENT_SCALE,
						(float)location.y * Game::C_CS_ELEMENT_SCALE);
	}

	// Convert from a partial grid location (e.g. x = 6.5) to physical position in world space
	CMPINLINE float						ElementPartialLocationToPhysicalPosition(float location)
	{ 
		return (location * Game::C_CS_ELEMENT_SCALE); 
	}
	
	// Convert from three dimensional partial element location to position in 3D space
	CMPINLINE XMVECTOR					ElementPartialLocationToPhysicalPosition(const FXMVECTOR location)
	{
		// NOTE: y & z coordinates are swapped when moving between grid & physical space, due to the coordinate systems used in each
		return XMVectorScale(
			XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_W>(location),
			Game::C_CS_ELEMENT_SCALE);
	}

	// Convert from one components of a 3D position to an element location component
	CMPINLINE int						PhysicalPositionToElementLocation(float position)
	{ 
		return (int)floor(position * Game::C_CS_ELEMENT_SCALE_RECIP); 
	}

	// Convert from a position in 3D space to an element location
	CMPINLINE INTVECTOR3				PhysicalPositionToElementLocation(const FXMVECTOR position)
	{
		XMFLOAT3 posf; 
		XMStoreFloat3(&posf, position);

		// Note: y & z coordinates are swapped when moving between grid & physical space, due to the coordinate systems used in each
		return INTVECTOR3(	(int)floor(posf.x * Game::C_CS_ELEMENT_SCALE_RECIP),
							(int)floor(posf.z * Game::C_CS_ELEMENT_SCALE_RECIP),
							(int)floor(posf.y * Game::C_CS_ELEMENT_SCALE_RECIP));
	}

	// Convert from a position in 3D space to an element location
	CMPINLINE INTVECTOR3				PhysicalPositionToElementLocation(const XMFLOAT3 & position)
	{
		// Note: y & z coordinates are swapped when moving between grid & physical space, due to the coordinate systems used in each
		return INTVECTOR3(	(int)floor(position.x * Game::C_CS_ELEMENT_SCALE_RECIP),
							(int)floor(position.z * Game::C_CS_ELEMENT_SCALE_RECIP),
							(int)floor(position.y * Game::C_CS_ELEMENT_SCALE_RECIP));
	}

	// Convert from one components of a 3D position to an element location component
	CMPINLINE float						PhysicalPositionToElementPartialLocation(float position)
	{
		return (position * Game::C_CS_ELEMENT_SCALE_RECIP);
	}

	// Convert from a position in 3D space to a partial element location
	CMPINLINE XMVECTOR					PhysicalPositionToElementPartialLocation(const FXMVECTOR position)
	{
		// Note: y & z coordinates are swapped when moving between grid & physical space, due to the coordinate systems used in each
		return XMVectorScale(
			XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_W>(position),
			Game::C_CS_ELEMENT_SCALE_RECIP);
	}

	// Convert from an element position to an element index
	CMPINLINE INTVECTOR3				ElementPositionToElementIndex(const INTVECTOR3 & elementposition)
	{
		return INTVECTOR3(	elementposition.x * Game::C_CS_ELEMENT_SCALE_RECIP,
							elementposition.y * Game::C_CS_ELEMENT_SCALE_RECIP,
							elementposition.z * Game::C_CS_ELEMENT_SCALE_RECIP);
	}

};

#endif