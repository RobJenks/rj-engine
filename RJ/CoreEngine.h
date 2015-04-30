#pragma once

#ifndef __CoreEngineH__
#define __CoreEngineH__

#include <unordered_map>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "GlobalFlags.h"
#include "D3DMain.h"
#include "DXLocaliser.h"
#include "Profiler.h"
#include "CameraClass.h"
#include "iAcceptsConsoleCommands.h"
class iShader;
class LightShader;
class LightFadeShader;
class LightHighlightShader;
class LightHighlightFadeShader;
class ParticleShader;
class TextureShader;
class TexcubeShader;
class FireShader;
class SkinnedNormalMapShader;
class Light;
class ViewFrustrum;
class FontShader;
class TextManager;
class EffectManager;
class Model;
class SimpleShip;
class ComplexShip;
class ComplexShipSection;
class ComplexShipSectionDetails;
class ComplexShipTile;
class ParticleEngine;
class Render2DManager;
class SkinnedModelInstance;
class OverlayRenderer;
struct GameConsoleCommand;

using namespace std;
using namespace std::tr1;

// Constant engine rendering values
const float SCREEN_DEPTH = 5000.0f;
const float SCREEN_NEAR = 0.1f;

class CoreEngine : public iAcceptsConsoleCommands
{
public:
	int tmprot;

	// Enumeration of all rendering stages
	enum RenderStage
	{
		Render_ALL = 0,
		Render_SystemRegion,
		Render_ImmediateRegion,
		Render_SystemObjects,
		Render_Effects,
		Render_ParticleEmitters,
		Render_UserInterface,
		Render_DebugData,
		Render_STAGECOUNT
	};

	// Enumeration of instancing-enabled shaders; ordered to minimise the number of rendering state changes required.  Also
	// aim to have all render states back at defaults by the final shader; this avoids having to reset them after processing the queue
	enum RenderQueueShader
	{
		RM_LightShader = 0,					// Requires: none
		RM_LightHighlightShader,			// Requires: none

		/* Alpha blending cutoff; perform all alpha blend-enabled operations after this point */

		RM_LightFadeShader,					// Requires: alpha blending
		RM_LightHighlightFadeShader,		// Requires: alpha blending

		RM_RENDERQUEUESHADERCOUNT			// Count of shaders that can be used within the render queue	
	};
		
	// Constructor/destructor/copy constructor/assignment operator
	CoreEngine(void);
	~CoreEngine(void);

	// Initialise all components of the game engine
	Result					InitialiseGameEngine(HWND hwnd);

	// Release all components of the game engine as part of a controlled shutdown
	void					ShutdownGameEngine();

	
	// Accessor/modifier methods for key game engine components
	CMPINLINE D3DMain		*GetDirect3D()				{ return m_D3D; }
	CMPINLINE CameraClass	*GetCamera()				{ return m_camera; }
	CMPINLINE DXLocaliser	*GetDXLocaliser()			{ return m_dxlocaliser; }
	CMPINLINE ViewFrustrum	*GetViewFrustrum()			{ return m_frustrum; }
	CMPINLINE TextManager	*GetTextManager()			{ return m_textmanager; }
	CMPINLINE EffectManager *GetEffectManager()			{ return m_effectmanager; }
	CMPINLINE ParticleEngine *GetParticleEngine()		{ return m_particleengine; }
	CMPINLINE Render2DManager *Get2DRenderManager()		{ return m_render2d; }
	CMPINLINE OverlayRenderer *GetOverlayRenderer()		{ return m_overlayrenderer; }

	// Methods to retrieve a reference to key shaders implemented by the engine
	CMPINLINE	FireShader *				GetFireShader(void)						{ return m_fireshader; }
	CMPINLINE	FontShader *				GetFontShader(void)						{ return m_fontshader; }
	CMPINLINE	LightShader *				GetLightShader(void)					{ return m_lightshader; }
	CMPINLINE	LightFadeShader *			GetLightFadeShader(void)				{ return m_lightfadeshader; }
	CMPINLINE	LightHighlightShader *		GetLightHighlightShader(void)			{ return m_lighthighlightshader; }
	CMPINLINE	LightHighlightFadeShader *	GetLightHighlightFadeShader(void)		{ return m_lighthighlightfadeshader; }
	CMPINLINE	ParticleShader *			GetParticleShader(void)					{ return m_particleshader; }
	CMPINLINE	TexcubeShader *				GetTexcubeShader(void)					{ return m_texcubeshader; }
	CMPINLINE	TextureShader *				GetTextureShader(void)					{ return m_textureshader; }	

	// Methods to retrieve the key render matrices from the engine
	CMPINLINE void GetRenderViewMatrix(D3DXMATRIX &m)				{ m = r_view; }
	CMPINLINE void GetRenderInverseViewMatrix(D3DXMATRIX &m)		{ m = r_invview; }
	CMPINLINE void GetRenderProjectionMatrix(D3DXMATRIX &m)			{ m = r_projection; }
	CMPINLINE void GetRenderOrthographicMatrix(D3DXMATRIX &m)		{ m = r_orthographic; }
	
	// Pass-through accessor methods for key engine components
	CMPINLINE ID3D11Device *		GetDevice(void)			{ return m_D3D->GetDevice(); }
	CMPINLINE ID3D11DeviceContext *	GetDeviceContext(void)	{ return m_D3D->GetDeviceContext(); }

	// Validation method to determine whether the engine has all critical frame-generatation components available
	CMPINLINE bool			Operational()				{ return ( m_D3D && m_D3D->GetDevice() ); }

	// Pass-through methods to begin and end a scene; passes control directly through to the equivalent D3D methods
	CMPINLINE void			BeginFrame()				{ m_D3D->BeginScene(); }
	CMPINLINE void			EndFrame()					{ m_D3D->EndScene(); }


	/* *** Main rendering function *** */
	void					Render(void);

	// Method to render the system region
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_SystemRegion, 
		void, RenderSystemRegion, void, )

	// Method to render the immediate region surrounding the player
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ImmediateRegion, 
		void, RenderImmediateRegion, void, )

	// Renders all objects in the specified system, based on simulation state and visibility testing
	void					RenderAllSystemObjects(SpaceSystem *system);

	// Simple ship-rendering method
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_SimpleShips,
		void, RenderSimpleShip, SimpleShip *s, s)

	// Renders a complex ship, including all section and interior contents if applicable
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ComplexShips, 
		void, RenderComplexShip, SINGLE_ARG(ComplexShip *ship, bool renderinterior), SINGLE_ARG(ship, renderinterior))

	// Methods to render parts of a complex ship
	void					RenderComplexShipSection(ComplexShip *ship, ComplexShipSection *sec);
	void					RenderComplexShipTile(ComplexShipTile *tile, iSpaceObjectEnvironment *environment);

	// RenderObjectEnvironments(iSpaceObjectEnvironment *environment)
	// Method to render the interior of an object environment, including any tiles, objects or terrain within it
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ObjectEnvironments, 
		void, RenderObjectEnvironment, iSpaceObjectEnvironment *environment, environment)

	// Actor-rendering methods; actors are queued for rendering in one batch, after other objects are processed, to avoid
	// multiple engine state changes per frame
	CMPINLINE void			QueueActorRendering(Actor *actor)	{ if (actor) m_queuedactors.push_back(actor); }

	// Processes the actor render queue and renders all actors at once
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_Actors, 
		void, ProcessQueuedActorRendering, void, )

	// Rendering methods for skinned models
	void					RenderSkinnedModelInstance(SkinnedModelInstance &model);

	// User interface, text and all other 2D rendering functions
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_UI, 
		void, RenderUserInterface, void, )

	// Rendering of all effects handled by the effect manager
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_Effects, 
		void, RenderEffects, void, )

	// Render all particle emitters via the particle engine
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_Particles, 
		void, RenderParticleEmitters, void, )

	// Renders a standard model.  Processed via the instanced render queue for efficiency
	CMPINLINE void			RenderModel(Model *model, D3DXMATRIX *world)
	{
		// Render using the standard light shader.  Add to the queue for batched rendering.
		SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightShader, model, world);
	}

	// Renders a standard model.  Applies highlighting to the model
	CMPINLINE void			RenderModel(Model *model, D3DXMATRIX *world, const D3DXVECTOR3 & highlight)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		m_instanceparams.x = highlight.x; m_instanceparams.y = highlight.y; m_instanceparams.z = highlight.z;
		SubmitForRendering(CoreEngine::RenderQueueShader::RM_LightHighlightShader, model, world, m_instanceparams);
	}

	// Renders a standard model.  Applies alpha fade to the model
	CMPINLINE void			RenderModel(Model *model, D3DXMATRIX *world, const D3DXVECTOR3 & position, float alpha)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		m_instanceparams.x = alpha;
		SubmitForZSortedRendering(CoreEngine::RenderQueueShader::RM_LightFadeShader, model, world, m_instanceparams, position);
	}

	// Renders a standard model.  Applies highlighting and alpha fade to the model
	CMPINLINE void			RenderModel(Model *model, D3DXMATRIX *world, const D3DXVECTOR3 & position, const D3DXVECTOR3 & highlight, float alpha)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		m_instanceparams.x = highlight.x; m_instanceparams.y = highlight.y; m_instanceparams.z = highlight.z; m_instanceparams.w = alpha;
		SubmitForZSortedRendering(CoreEngine::RenderQueueShader::RM_LightHighlightFadeShader, model, world, m_instanceparams, position);
	}

	// Performs rendering of debug/special data
	void RenderDebugData(void);
	void DebugRenderSpaceCollisionBoxes(void);
	void DebugRenderEnvironmentCollisionBoxes(void);
	void DebugRenderSpatialPartitioningTree(void);

	// Gets or sets the environment that is the subject of debug terrain rendering
	CMPINLINE Game::ID_TYPE GetDebugTerrainRenderEnvironment(void) const { return m_debug_renderenvboxes; }
	CMPINLINE void SetDebugTerrainRenderEnvironment(Game::ID_TYPE environment_id) { m_debug_renderenvboxes = environment_id; }

	// Structure keeping track of render info per frame
	struct EngineRenderInfoData
	{
		int DrawCalls;
		int ShipRenderCount;
		int ComplexShipRenderCount;
		int ComplexShipSectionRenderCount;
		int ComplexShipTileRenderCount;
		int ActorRenderCount;
		int TerrainRenderCount;
	};

	// Function to return the per-frame render info
	CMPINLINE EngineRenderInfoData GetRenderInfo(void) { return m_renderinfo; }

	// Virtual inherited method to accept a command from the console
	bool ProcessConsoleCommand(GameConsoleCommand & command);

	// Retrieve key game engine parameters
	CMPINLINE HWND GetHWND(void) const			{ return m_hwnd; }
	CMPINLINE bool VsyncEnabled(void) const		{ return m_vsync; }

	// Returns the alpha blending state.  Passthrough method to the D3D component
	CMPINLINE D3DMain::AlphaBlendState GetAlphaBlendState(void) const		{ return m_D3D->GetAlphaBlendState(); }

	// Return a flag indicating whether a particular render stage is active this cycle
	CMPINLINE bool RenderStageActive(RenderStage stage) { return m_renderstages[(int)stage]; }

	// Activate or deactivate a particular stage of the rendering cycle.  Changing the 'All' stage will overwrite all stage values
	void SetRenderStageState(RenderStage stage, bool active);

	// Enumeration of render flags
	enum RenderFlag
	{
		None = 0,
		RenderTree,
		DisableHullRendering,
		RenderOBBs,
		RenderTerrainBoxes,
		_RFLAG_COUNT
	};

	// Methods to get and set render flags (to potentially be replaced by a parameterised method in future)
	CMPINLINE bool	GetRenderFlag(RenderFlag flag)							{ return m_renderflags[flag]; }
	CMPINLINE void	SetRenderFlag(RenderFlag flag, bool value)				{ m_renderflags[flag] = value; }


private:
	
	// Private methods to initialise each component in turn
	Result					InitialiseDirect3D(HWND hwnd);
	Result					InitialiseDXLocaliser(DXLocaliser::DXLevel DirectXLevel);
	Result					InitialiseRenderQueue(void);
	Result					InitialiseRenderFlags(void);
	Result					InitialiseCamera(void);
	Result					InitialiseLightShader(void);
	Result					InitialiseLightFadeShader(void);
	Result					InitialiseLightHighlightShader(void);
	Result					InitialiseLightHighlightFadeShader(void);
	Result					InitialiseParticleShader(void);
	Result					InitialiseTextureShader(void);
	Result					InitialiseLights(void);
	Result					InitialiseFrustrum(void);
	Result					InitialiseFontShader(void);
	Result					InitialiseTextRendering(void);
	Result					InitialiseFonts(void);
	Result					InitialiseTexcubeShader(void);
	Result					InitialiseFireShader(void);
	Result					InitialiseEffectManager(void);
	Result					InitialiseSkinnedNormalMapShader(void);
	Result					InitialiseParticleEngine(void);
	Result					Initialise2DRenderManager(void);
	Result					InitialiseOverlayRenderer(void);
	Result					InitialiseEnvironmentRendering(void);

	// Private methods to release each component in turn
	void					ShutdownDirect3D(void);
	void					ShutdownDXLocaliser(void);
	void					ShutdownRenderQueue(void);
	void					ShutdownTextureData(void);
	void					ShutdownCamera(void);
	void					ShutdownLightShader(void);
	void					ShutdownLightFadeShader(void);
	void					ShutdownLightHighlightShader(void);
	void					ShutdownLightHighlightFadeShader(void);
	void					ShutdownParticleShader(void);
	void					ShutdownTextureShader(void);
	void					ShutdownLights(void);
	void					ShutdownFrustrum(void);
	void					ShutdownFontShader(void);
	void					ShutdownTextRendering(void);
	void					ShutdownFonts(void);
	void					ShutdownTexcubeShader(void);
	void					ShutdownFireShader(void);
	void					ShutdownEffectManager(void);
	void					ShutdownSkinnedNormalMapShader(void);
	void					ShutdownParticleEngine(void);
	void					Shutdown2DRenderManager(void);
	void					ShutdownOverlayRenderer(void);
	void					ShutdownEnvironmentRendering(void);

	// Update window size details based on these parameters, recalculating for windowed mode as required
	void					UpdateWindowSizeParameters(int screenWidth, int screenHeight, bool fullscreen);

	// Pointer to each component that makes up this game engine
	D3DMain					*m_D3D;
	DXLocaliser				*m_dxlocaliser;
	CameraClass				*m_camera;
	LightShader				*m_lightshader;
	LightFadeShader			*m_lightfadeshader;
	LightHighlightShader	*m_lighthighlightshader;
	LightHighlightFadeShader*m_lighthighlightfadeshader;
	Light					*m_light;
	ParticleShader			*m_particleshader;
	TextureShader			*m_textureshader;
	ViewFrustrum			*m_frustrum;
	FontShader				*m_fontshader;
	TextManager				*m_textmanager;
	TexcubeShader			*m_texcubeshader;
	FireShader				*m_fireshader;
	EffectManager			*m_effectmanager;
	SkinnedNormalMapShader 	*m_skinnedshader;
	ParticleEngine			*m_particleengine;
	Render2DManager			*m_render2d;
	OverlayRenderer			*m_overlayrenderer;

	// Game engine parameters
	HWND					m_hwnd;							// Handle to the target window
	bool					m_vsync;						// Flag determining whether vsync is enabled

	// Render-cycle-specific parameters; denoted by r_*, these are valid for the current render cycle only 
	// and are used for reasons of render efficiency
	ID3D11DeviceContext *	r_devicecontext;		// The device context in use for this render cycle
	D3DXMATRIX				r_view;					// View matrix for the current render cycle
	D3DXMATRIX				r_projection;			// Projection matrix for the current render cycle
	D3DXMATRIX				r_orthographic;			// Orthographic matrix for the current render cycle
	D3DXMATRIX				r_invview;				// We will also store the inverse view matrix given its usefulness

	// Structure of a single instance in the instancing model
	struct					RM_InstanceStructure
	{
		D3DXMATRIX			World;								// World matrix to transform into the world
		D3DXVECTOR4			Params;								// Float-4 of parameters that can be passed for each instance

		// Constructor where only the world transform is required; other params will be unitialised (for efficiency) and should not be used
		RM_InstanceStructure(const D3DXMATRIX *world) : World(*world) {}

		// Constructor including additional per-instance parameters
		RM_InstanceStructure(const D3DXMATRIX *world, const D3DXVECTOR4 & params) : World(*world), Params(params) {}
	};


	// Geometry batching parameters, used by the render manager to sequence & minimise render calls
	typedef						RM_InstanceStructure							RM_Instance;
	typedef						vector<RM_Instance>								RM_InstanceData;
	typedef						unordered_map<Model*, RM_InstanceData>			RM_ModelInstanceMap;
	typedef						vector<RM_ModelInstanceMap>						RM_ShaderModelInstanceMap;



	// Structure to hold z-sorted instance data, used where objects must be sorted before processing the render queue
	struct							RM_ZSortedInstance
	{
		int							Key;
		const Model *				ModelPtr;
		RM_Instance					Item;

		bool operator<(const RM_ZSortedInstance & val) const	{ return (Key < val.Key); }

		RM_ZSortedInstance(int key, const Model *model, const D3DXMATRIX *world, const D3DXVECTOR4 & params) : 
			Key(key), ModelPtr(model), Item(RM_Instance(world, params)) {}
	};


	// Details on a shader used in the render queue
	struct							RM_InstancedShaderDetails
	{
		iShader *					Shader;							// The shader itself
		bool						RequiresZSorting;				// Flag determining whether instances must go through an intermediate z-sorting step
		D3DMain::AlphaBlendState	AlphaBlendRequired;				// Flag indicating if/how alpha blending should be enabled for this shader

		vector<RM_ZSortedInstance>	SortedInstances;				// Vector used for the intermediate sorting step, where required, so that items 
																	// are sent for rendering in a particular Z order

		// Default constructor, no reference to shader, sets all parameters to defaults
		RM_InstancedShaderDetails(void) : 
			Shader(NULL), RequiresZSorting(false), AlphaBlendRequired(D3DMain::AlphaBlendState::AlphaBlendDisabled)
		{}

		// Constructor allowing all parameters to be specified
		RM_InstancedShaderDetails(iShader *shader, bool requiresZsorting, D3DMain::AlphaBlendState alphablend) : 
			Shader(shader), RequiresZSorting(requiresZsorting), AlphaBlendRequired(alphablend)
		{}
	};

	typedef						vector<RM_InstancedShaderDetails>				RM_ShaderCollection;
	


	ID3D11Buffer *				m_instancebuffer;
	RM_ShaderModelInstanceMap	m_renderqueue;
	RM_ShaderCollection			m_renderqueueshaders;
	ID3D11Buffer *				m_instancedbuffers[2];
	unsigned int				m_instancedstride[2], m_instancedoffset[2];

	// Process the full render queue for all shaders in scope
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ProcessRenderQueue, 
		void, ProcessRenderQueue, void, );

	// Clear the render queue ready for the next frame
	void						ClearRenderingQueue(void);

	// Performs an intermediate z-sorting of instances before rendering via the render queue.  Used only for shaders/techniques (e.g. alpha
	// blending) that require instances to be z-sorted
	void						PerformZSortedRenderQueueProcessing(int shaderindex);

	// Method to submit for rendering where only the transform matrix is required; no additional params.  Will submit directly to
	// the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void				SubmitForRendering(RenderQueueShader shader, Model *model, const D3DXMATRIX *transform)
	{
		// No sorting required, so push directly onto the vector of instances, to be applied for the specified model & shader
		((m_renderqueue[shader])[model]).push_back(RM_Instance(transform));
	}

	// Method to submit for rendering that includes additional per-instance parameters beyond the world transform.  Will submit 
	// directly to the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void				SubmitForRendering(RenderQueueShader shader, Model *model, const D3DXMATRIX *transform, const D3DXVECTOR4 & params)
	{
		// Push this matrix onto the vector of transform matrices, to be applied for the specified model & shader
		((m_renderqueue[shader])[model]).push_back(RM_Instance(transform, params));
	}

	// Method to submit for z-sorted rendering.  Should be used for any techniques (e.g. alpha blending) that require reverse-z-sorted 
	// objects.  Performance overhead; should be used only where specifically required
	CMPINLINE void				SubmitForZSortedRendering(	RenderQueueShader shader, Model *model, const D3DXMATRIX *transform, 
															const D3DXVECTOR4 & params, const D3DXVECTOR3 &position)
	{
		// Compute the z-value as the distance squared from this object to the camera
		int z = (int)D3DXVec3LengthSq(&(position - m_camera->GetPosition()));

		// Add to the z-sorted vector with this z-value as the sorting key
		m_renderqueueshaders[(int)shader].SortedInstances.push_back(RM_ZSortedInstance(z, model, transform, params));
	}

	// Recursively analsyses and renders a sector of the environment.  Performs binary splitting to efficiently test visibility.
	// Only called internally so no parameter checks are performed, for efficiency.
	void					RenderObjectEnvironmentSector(iSpaceObjectEnvironment *environment, const INTVECTOR3 & start, const INTVECTOR3 & size);

	// Renders the contents of an element, including all linked tiles, objects & terrain.  Updates the temporary
	// render lists to ensure an item that spans multiple elements is not rendered more than once
	void					RenderObjectEnvironmentSectorContents(iSpaceObjectEnvironment *environment, const INTVECTOR3 & element);

	// Render variants for specific scenarios, e.g. specifically for 2D rendering
	D3DXMATRIX				m_baseviewmatrix;		// Base view matrix for all 2D rendering

	// Functions for processing the per-frame render info
	EngineRenderInfoData	m_renderinfo;
	void ResetRenderInfo(void);

	// Pre-populated parameter sets for greater efficiency at render time, since only specific components need to be updated
	D3DXVECTOR4				m_instanceparams;

	// Vector used to queue up actors for rendering.  This allows us to render them all at once, avoiding multiple state changes
	std::vector<Actor*>		m_queuedactors;

	// Vector of flags that determine which render stages will be performed
	std::vector<bool>		m_renderstages;

	// Vector of flags that determine special rendering states/effects
	std::vector<bool>		m_renderflags;

	// Cached & precalculated fields used for rendering an environment
	D3DXVECTOR3					m_cache_zeropoint;								// World position of the (0,0,0) element, i.e. corner of the environment
	D3DXVECTOR3					m_cache_el_inc[3];								// World position delta to move +1 element in each local dimension
	D3DXVECTOR3					m_cache_el_inc_base[3];							// Base world position delta to move +1 element in each local dimension (transformed each frame)
	std::vector<Game::ID_TYPE>	m_tmp_renderedtiles;							// Temporary vector of tile IDs that have been rendered this cycle
	std::vector<Game::ID_TYPE>	m_tmp_renderedobjects;							// Temporary vector of object IDs that have been rendered this cycle
	std::vector<Game::ID_TYPE>	m_tmp_renderedterrain;							// Temporary vector of terrain IDs that have been rendered this cycle
	
	// Further temporary fields, used only for intermediate calculations to avoid allocation memory each time
	D3DXVECTOR3					m_tmp_vector3;
	D3DXMATRIX					m_tmp_matrix;

	// Persistent storage for objects being debug-rendered
	Game::ID_TYPE 			m_debug_renderenvboxes;
};



#endif