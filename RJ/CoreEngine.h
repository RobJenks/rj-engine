#pragma once

#ifndef __CoreEngineH__
#define __CoreEngineH__

#include <unordered_map>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "GlobalFlags.h"
#include "DX11_Core.h"
#include "D3DMain.h"

#include "Profiler.h"
#include "CameraClass.h"
#include "iAcceptsConsoleCommands.h"
#include "RenderQueue.h"
#include "RenderQueueOptimiser.h"
#include "LightingManagerObject.h"
#include "ShaderManager.h"
#include "Model.h"
#include "ModelBuffer.h"
class iShader;
class ModelBuffer;
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
class LightingManager;
class FontShader;
class TextManager;
class EffectManager;
class SimpleShip;
class ComplexShip;
class ComplexShipSection;
class ComplexShipSectionDetails;
class iSpaceObjectEnvironment;
class ComplexShipTile;
class ParticleEngine;
class Render2DManager;
class SkinnedModelInstance;
class ArticulatedModel;
class TurretController;
class OverlayRenderer;
class BasicProjectileSet;
class VolLineShader;
class EnvironmentTree;
struct GameConsoleCommand;
struct VolumetricLine;
struct SentenceType;

using namespace std;
using namespace std::tr1;

// Constant engine rendering values
const float SCREEN_DEPTH = 5000.0f;
const float SCREEN_NEAR = 0.1f;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class CoreEngine : public ALIGN16<CoreEngine>, public iAcceptsConsoleCommands
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(CoreEngine)

	// Enumeration of all rendering stages
	enum RenderStage
	{
		Render_ALL = 0,
		Render_SystemRegion,
		Render_ImmediateRegion,
		Render_SystemObjects,
		Render_BasicProjectiles,
		Render_Effects,
		Render_ParticleEmitters,
		Render_UserInterface,
		Render_DebugData,
		Render_STAGECOUNT
	};
		
	// Constructor/destructor/copy constructor/assignment operator
	CoreEngine(void);
	~CoreEngine(void);

	// Initialise all components of the game engine
	Result					InitialiseGameEngine(HWND hwnd);

	// Release all components of the game engine as part of a controlled shutdown
	void					ShutdownGameEngine();

	// Key game engine components
	LightingManagerObject	LightingManager;
	
	// Accessor/modifier methods for key game engine components
	CMPINLINE D3DMain		*GetDirect3D()				{ return m_D3D; }
	CMPINLINE CameraClass	*GetCamera()				{ return m_camera; }
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
	CMPINLINE	VolLineShader *				GetVolLineShader(void)					{ return m_vollineshader; }

	// Methods to retrieve the key render matrices from the engine
	CMPINLINE const XMMATRIX & GetRenderViewMatrix(void) const							{ return r_view; }
	CMPINLINE const XMMATRIX & GetRenderInverseViewMatrix(void) const					{ return r_invview; }
	CMPINLINE const XMMATRIX & GetRenderProjectionMatrix(void) const					{ return r_projection; }
	CMPINLINE const XMMATRIX & GetRenderOrthographicMatrix(void) const					{ return r_orthographic; }
	CMPINLINE const XMMATRIX & GetRenderViewProjectionMatrix(void) const				{ return r_viewproj; }
	CMPINLINE const XMMATRIX & GetRenderInverseViewProjectionMatrix(void) const			{ return r_invviewproj; }
	CMPINLINE const XMMATRIX & GetRenderViewProjectionScreenMatrix(void) const			{ return r_viewprojscreen; }
	CMPINLINE const XMMATRIX & GetRenderInverseViewProjectionScreenMatrix(void) const	{ return r_invviewprojscreen; }
	CMPINLINE const XMFLOAT4X4 & GetRenderViewMatrixF(void) const						{ return r_view_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderInverseViewMatrixF(void) const				{ return r_invview_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderProjectionMatrixF(void) const					{ return r_projection_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderOrthographicMatrixF(void) const				{ return r_orthographic_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderViewProjectionMatrixF(void) const				{ return r_viewproj_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderInverseViewProjectionMatrixF(void) const		{ return r_invviewproj_f; }

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

    // Generic iObject rendering method; used by subclasses wherever possible
	void                    RenderObject(iObject *object);

	// Simple ship-rendering method
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_SimpleShips,
		void, RenderSimpleShip, SimpleShip *s, s)

	// Renders a complex ship, including all section and interior contents if applicable
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ComplexShips, 
		void, RenderComplexShip, SINGLE_ARG(ComplexShip *ship, bool renderinterior), SINGLE_ARG(ship, renderinterior))

	// Methods to render parts of a complex ship
	void					RenderComplexShipSection(ComplexShip *ship, ComplexShipSection *sec);
	void					RenderComplexShipTile(ComplexShipTile *tile, iSpaceObjectEnvironment *environment);

	// Renders a collection of turrets that have already been updated by their turret controller
	void					RenderTurrets(TurretController & controller);

	// Renders all elements of a projectile set which are currently visible
	void					RenderProjectileSet(BasicProjectileSet & projectiles);

	// RenderObjectEnvironments(iSpaceObjectEnvironment *environment)
	// Method to render the interior of an object environment, including any tiles, objects or terrain within it
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ObjectEnvironments,
		void, RenderObjectEnvironment, iSpaceObjectEnvironment *environment, environment)

	// Actor-rendering methods; actors are queued for rendering in one batch, after other objects are processed, to avoid
	// multiple engine state changes per frame
	void					QueueActorRendering(Actor *actor);
	
	// Processes the actor render queue and renders all actors at once
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_Actors, 
		void, ProcessQueuedActorRendering, void, )

	// Rendering methods for skinned models
	void					RenderSkinnedModelInstance(SkinnedModelInstance &model);

	// Renders a volumetric line
	void					RenderVolumetricLine(const VolumetricLine & line);

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
	CMPINLINE void XM_CALLCONV			RenderModel(Model *model, const FXMMATRIX world)
	{
		// Render using the standard light shader.  Add to the queue for batched rendering.
		SubmitForRendering(RenderQueueShader::RM_LightShader, model, world);
	}

	// Renders a standard model.  Applies highlighting to the model
	CMPINLINE void			RenderModel(Model *model, const XMFLOAT4 & highlight, const CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, model, world, highlight);
	}

	// Renders a standard model.  Applies alpha fade to the model
	CMPINLINE void			RenderModel(Model *model, const FXMVECTOR position, float alpha, const CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		m_instanceparams.x = alpha;
		SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, model, world, m_instanceparams, position);
	}

	// Renders a standard model.  Applies highlighting and alpha fade to the model
	CMPINLINE void			RenderModel(Model *model, const FXMVECTOR position, const XMFLOAT3 highlight, float alpha, CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		m_instanceparams = XMFLOAT4(highlight.x, highlight.y, highlight.z, alpha);
		SubmitForZSortedRendering(RenderQueueShader::RM_LightHighlightFadeShader, model, world, m_instanceparams, position);
	}

	// Returns a reference to the model buffer currently being rendered
	CMPINLINE ModelBuffer *	GetCurrentModelBuffer(void) const			{ return m_current_modelbuffer; }

	// Performs rendering of debug/special data
	void RenderDebugData(void);
	void DebugRenderSpaceCollisionBoxes(void);
	void DebugRenderEnvironmentCollisionBoxes(void);
	void DebugRenderSpatialPartitioningTree(void);
	void DebugRenderEnvironmentTree(void);
	void DebugRenderEnvironmentNavNetwork(void);
	void DebugRenderObjectIdentifiers(void);

	// Gets or sets the environment that is the subject of debug rendering
	CMPINLINE Game::ID_TYPE GetDebugTerrainRenderEnvironment(void) const { return m_debug_renderenvboxes; }
	CMPINLINE void SetDebugTerrainRenderEnvironment(Game::ID_TYPE environment_id) { m_debug_renderenvboxes = environment_id; }
	CMPINLINE Game::ID_TYPE GetDebugTreeRenderEnvironment(void) const { return m_debug_renderenvtree; }
	CMPINLINE void SetDebugTreeRenderEnvironment(Game::ID_TYPE environment_id) { m_debug_renderenvtree = environment_id; }
	CMPINLINE Game::ID_TYPE GetDebugNavNetworkRenderEnvironment(void) const { return m_debug_renderenvnetwork; }
	CMPINLINE void SetDebugNavNetworkRenderEnvironment(Game::ID_TYPE environment_id) { m_debug_renderenvnetwork = environment_id; }
	CMPINLINE float GetDebugObjectIdentifierRenderingDistance(void) const { return m_debug_renderobjid_distance; }
	CMPINLINE void SetDebugObjectIdentifierRenderingDistance(float dist) { m_debug_renderobjid_distance = clamp(dist, 1.0f, 100000.0f); }

	
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

	// Event triggered when the application window is resized
	void WindowResized(void);

	// Virtual inherited method to accept a command from the console
	bool ProcessConsoleCommand(GameConsoleCommand & command);

	// Retrieve key game engine parameters
	CMPINLINE HWND GetHWND(void) const			{ return m_hwnd; }
	CMPINLINE bool VsyncEnabled(void) const		{ return m_vsync; }

	// Returns the alpha blending state.  Passthrough method to the D3D component
	CMPINLINE D3DMain::AlphaBlendState GetAlphaBlendState(void) const		{ return m_D3D->GetAlphaBlendState(); }

	// Central shader manager for the engine
	ShaderManager			ShaderManager;

	// Return a flag indicating whether a particular render stage is active this cycle
	CMPINLINE bool RenderStageActive(RenderStage stage) { return m_renderstages[(int)stage]; }

	// Activate or deactivate a particular stage of the rendering cycle.  Changing the 'All' stage will overwrite all stage values
	void SetRenderStageState(RenderStage stage, bool active);

	// Enumeration of render flags
	enum RenderFlag
	{
		None = 0,
		RenderTree,
		RenderEnvTree,
		DisableHullRendering,
		RenderOBBs,
		RenderTerrainBoxes,
		RenderNavNetwork,
		RenderObjectIdentifiers,
		_RFLAG_COUNT
	};

	// Methods to get and set render flags (to potentially be replaced by a parameterised method in future)
	CMPINLINE bool	GetRenderFlag(RenderFlag flag)							{ return m_renderflags[flag]; }
	CMPINLINE void	SetRenderFlag(RenderFlag flag, bool value)				{ m_renderflags[flag] = value; }

	// Transform the specified world location into projection space ([-1 +1], [-1 +1])
	CMPINLINE XMVECTOR		WorldToProjection(const FXMVECTOR world_pos) 
	{ 
		return XMVector3TransformCoord(world_pos, r_viewproj);
	}

	// Transform the specified world location into screen space ([0 ScreenWidth], [0 ScreenHeight])
	CMPINLINE XMVECTOR		WorldToScreen(const FXMVECTOR world_pos)
	{
		return XMVector3TransformCoord(world_pos, r_viewprojscreen);
	}

	// Transform the specified projection space coordinates ([-1 +1], [-1 +1]) into world space
	CMPINLINE XMVECTOR		ProjectionToWorld(const FXMVECTOR proj_pos)
	{
		return XMVector3TransformCoord(proj_pos, r_invviewproj);
	}
	
	// Transform the specified screen space coordinates ([0 ScreenWidth], [0 ScreenHeight]) into world space
	CMPINLINE XMVECTOR		ScreenToWorld(const FXMVECTOR screen_pos)
	{
		return XMVector3TransformCoord(screen_pos, r_invviewprojscreen);
	}

	// Calculates the bounds of this object in screen space
	void				DetermineObjectScreenBounds(const iObject & obj, XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds);

	// Returns a position in screen space corresponding to the specified object.  Accepts an offset parameter
	// in screen coordinates based on the object size; [0, +0.5] would be centred in x, and return a position
	// at the top edge of the object in screen space
	XMVECTOR				GetScreenLocationForObject(const iObject & obj, const XMFLOAT2 & offset);
	
	// Outputs the contents of the render queue to debug-out
	void					DebugOutputRenderQueueContents(void);

private:
	
	// Private methods to initialise each component in turn
	Result					InitialiseDirect3D(HWND hwnd);
	Result					InitialiseDirectXMath(void);
	Result					InitialiseRenderQueue(void);
	Result					InitialiseRenderFlags(void);
	Result					InitialiseCamera(void);
	Result					InitialiseLightingManager(void);
	Result					InitialiseShaderSupport(void);
	Result					InitialiseLightShader(void);
	Result					InitialiseLightFadeShader(void);
	Result					InitialiseLightHighlightShader(void);
	Result					InitialiseLightHighlightFadeShader(void);
	Result					InitialiseParticleShader(void);
	Result					InitialiseTextureShader(void);
	Result					InitialiseFrustrum(void);
	Result					InitialiseFontShader(void);
	Result					InitialiseTextRendering(void);
	Result					InitialiseFonts(void);
	Result					InitialiseTexcubeShader(void);
	Result					InitialiseFireShader(void);
	Result					InitialiseEffectManager(void);
	Result					InitialiseSkinnedNormalMapShader(void);
	Result					InitialiseVolLineShader(void);
	Result					InitialiseParticleEngine(void);
	Result					Initialise2DRenderManager(void);
	Result					InitialiseOverlayRenderer(void);
	Result					InitialiseEnvironmentRendering(void);

	// Private methods to release each component in turn
	void					ShutdownDirect3D(void);
	void					ShutdownDXMath(void);
	void					ShutdownRenderQueue(void);
	void					ShutdownTextureData(void);
	void					ShutdownCamera(void);
	void					ShutdownLightingManager(void);
	void					ShutdownShaderSupport(void);
	void					ShutdownLightShader(void);
	void					ShutdownLightFadeShader(void);
	void					ShutdownLightHighlightShader(void);
	void					ShutdownLightHighlightFadeShader(void);
	void					ShutdownParticleShader(void);
	void					ShutdownTextureShader(void);
	void					ShutdownFrustrum(void);
	void					ShutdownFontShader(void);
	void					ShutdownTextRendering(void);
	void					ShutdownFonts(void);
	void					ShutdownTexcubeShader(void);
	void					ShutdownFireShader(void);
	void					ShutdownEffectManager(void);
	void					ShutdownSkinnedNormalMapShader(void);
	void					ShutdownVolLineShader(void);
	void					ShutdownParticleEngine(void);
	void					Shutdown2DRenderManager(void);
	void					ShutdownOverlayRenderer(void);
	void					ShutdownEnvironmentRendering(void);

	// Update window size details based on these parameters, recalculating for windowed mode as required
	//void					UpdateWindowSizeParameters(int screenWidth, int screenHeight, bool fullscreen);

	// Pointer to each component that makes up this game engine
	D3DMain					*m_D3D;
	CameraClass				*m_camera;
	LightShader				*m_lightshader;
	LightFadeShader			*m_lightfadeshader;
	LightHighlightShader	*m_lighthighlightshader;
	LightHighlightFadeShader*m_lighthighlightfadeshader;
	ParticleShader			*m_particleshader;
	TextureShader			*m_textureshader;
	ViewFrustrum			*m_frustrum;
	FontShader				*m_fontshader;
	TextManager				*m_textmanager;
	TexcubeShader			*m_texcubeshader;
	FireShader				*m_fireshader;
	EffectManager			*m_effectmanager;
	SkinnedNormalMapShader 	*m_skinnedshader;
	VolLineShader			*m_vollineshader;
	ParticleEngine			*m_particleengine;
	Render2DManager			*m_render2d;
	OverlayRenderer			*m_overlayrenderer;

	// Game engine parameters
	HWND					m_hwnd;							// Handle to the target window
	bool					m_vsync;						// Flag determining whether vsync is enabled

	// Render-cycle-specific parameters; denoted by r_*, these are valid for the current render cycle only 
	// and are used for reasons of render efficiency
	ID3D11DeviceContext *	r_devicecontext;		// The device context in use for this render cycle
	AXMMATRIX				r_view;					// View matrix for the current render cycle
	AXMMATRIX				r_projection;			// Projection matrix for the current render cycle
	AXMMATRIX				r_orthographic;			// Orthographic matrix for the current render cycle
	AXMMATRIX				r_invview;				// We will also store the inverse view matrix given its usefulness
	AXMMATRIX				r_viewproj;				// Store the combined (view * proj) matrix
	AXMMATRIX				r_invviewproj;			// Also store the inverse viewproj matrix, i.e. (view * proj)^-1
	AXMMATRIX				m_projscreen;			// Adjustment matrix from projection to screen coordinates (only recalculated when screen parameters change)
	AXMMATRIX				r_viewprojscreen;		// Combined (view * proj * screen) matrix
	AXMMATRIX				r_invviewprojscreen;	// Inverse of the view/proj/screen transform matrix
	XMFLOAT4X4				r_view_f;				// Local float representation of the current frame view matrix
	XMFLOAT4X4				r_projection_f;			// Local float representation of the current frame projection matrix
	XMFLOAT4X4				r_orthographic_f;		// Local float representation of the current frame orthographic matrix
	XMFLOAT4X4				r_invview_f;			// Local float representation of the current frame inverse view matrix
	XMFLOAT4X4				r_viewproj_f;			// Local float representation of the current frame (view * proj) matrix
	XMFLOAT4X4				r_invviewproj_f;		// Local float representation of the current frame inverse (view * proj) matrix


	ID3D11Buffer *				m_instancebuffer;
	RM_RenderQueue				m_renderqueue;
	RM_ShaderCollection			m_renderqueueshaders;
	ID3D11Buffer *				m_instancedbuffers[2];
	unsigned int				m_instancedstride[2], m_instancedoffset[2];
	D3D_PRIMITIVE_TOPOLOGY		m_current_topology;

	// Optimiser performs periodic maintenance on the engine render queue
	RenderQueueOptimiser		m_rq_optimiser;

	// Process the full render queue for all shaders in scope
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ProcessRenderQueue, 
		void, ProcessRenderQueue, void, );

	// Clear the render queue ready for the next frame
	void						ClearRenderingQueue(void);

	// Performs an intermediate z-sorting of instances before rendering via the render queue.  Used only for shaders/techniques (e.g. alpha
	// blending) that require instances to be z-sorted
	void						PerformZSortedRenderQueueProcessing(RM_InstancedShaderDetails & shader);
public:

	// Method to submit for rendering where only the transform matrix is required; no additional params.  Will submit directly to
	// the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void XM_CALLCONV	SubmitForRendering(RenderQueueShader shader, Model *model, const FXMMATRIX transform)
	{
		// No sorting required, so push directly onto the vector of instances, to be applied for the specified model & shader
		((m_renderqueue[shader])[model->GetModelBuffer()]).InstanceData.push_back(RM_Instance(transform, LightingManager.GetActiveLightingConfiguration()));
	}

	// Method to submit for rendering where only the transform matrix is required; no additional params.  Will submit directly to
	// the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void XM_CALLCONV			SubmitForRendering(RenderQueueShader shader, ModelBuffer *model, const FXMMATRIX transform)
	{
		// No sorting required, so push directly onto the vector of instances, to be applied for the specified model & shader
		((m_renderqueue[shader])[model]).InstanceData.push_back(RM_Instance(transform, LightingManager.GetActiveLightingConfiguration()));
	}

	// Method to submit for rendering that includes additional per-instance parameters beyond the world transform.  Will submit 
	// directly to the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void XM_CALLCONV			SubmitForRendering(RenderQueueShader shader, Model *model, const FXMMATRIX transform, const XMFLOAT4 & params)
	{
		// Push this matrix onto the vector of transform matrices, to be applied for the specified model & shader
		((m_renderqueue[shader])[model->GetModelBuffer()]).InstanceData.push_back(RM_Instance(transform, LightingManager.GetActiveLightingConfiguration(), params));
	}

	// Method to submit for rendering that includes additional per-instance parameters beyond the world transform.  Will submit 
	// directly to the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void XM_CALLCONV			SubmitForRendering(RenderQueueShader shader, ModelBuffer *model, const FXMMATRIX transform, const XMFLOAT4 & params)
	{
		// Push this matrix onto the vector of transform matrices, to be applied for the specified model & shader
		((m_renderqueue[shader])[model]).InstanceData.push_back(RM_Instance(transform, LightingManager.GetActiveLightingConfiguration(), params));
	}

	// Method to submit for rendering where the instance is directly specified.  Will submit directly to
	// the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void				SubmitForRendering(RenderQueueShader shader, Model *model, const RM_Instance & instance)
	{
		// No sorting required, so push directly onto the vector of instances, to be applied for the specified model & shader
		((m_renderqueue[shader])[model->GetModelBuffer()]).InstanceData.push_back(instance);
	}

	// Method to submit for rendering where the instance is directly specified.  Will submit directly to
	// the render queue and bypass the z-sorting process.  Should be used wherever possible for efficiency
	CMPINLINE void				SubmitForRendering(RenderQueueShader shader, ModelBuffer *model, const RM_Instance & instance)
	{
		// No sorting required, so push directly onto the vector of instances, to be applied for the specified model & shader
		((m_renderqueue[shader])[model]).InstanceData.push_back(instance);
	}

	// Method to submit for z-sorted rendering.  Should be used for any techniques (e.g. alpha blending) that require reverse-z-sorted 
	// objects.  Performance overhead; should be used only where specifically required
	CMPINLINE void XM_CALLCONV		SubmitForZSortedRendering(RenderQueueShader shader, Model *model, const FXMMATRIX transform,
															const XMFLOAT4 & params, const CXMVECTOR position)
	{
		// Compute the z-value as the distance squared from this object to the camera
		int z = (int)XMVectorGetX(XMVector3LengthSq(position - m_camera->GetPosition()));

		// Add to the z-sorted vector with this z-value as the sorting key2
		m_renderqueueshaders[(int)shader].SortedInstances.push_back(
			RM_ZSortedInstance(z, model->GetModelBuffer(), transform, LightingManager.GetActiveLightingConfiguration(), params));
	}
	
	// Method to submit for z-sorted rendering.  Should be used for any techniques (e.g. alpha blending) that require reverse-z-sorted 
	// objects.  Performance overhead; should be used only where specifically required
	CMPINLINE void XM_CALLCONV		SubmitForZSortedRendering(RenderQueueShader shader, ModelBuffer *model, const FXMMATRIX transform,
															const XMFLOAT4 & params, const CXMVECTOR position)
	{
		// Compute the z-value as the distance squared from this object to the camera
		int z = (int)XMVectorGetX(XMVector3LengthSq(position - m_camera->GetPosition()));

		// Add to the z-sorted vector with this z-value as the sorting key
		m_renderqueueshaders[(int)shader].SortedInstances.push_back(
			RM_ZSortedInstance(z, model, transform, LightingManager.GetActiveLightingConfiguration(), params));
	}

	// Method to submit for z-sorted rendering.  Should be used for any techniques (e.g. alpha blending) that require reverse-z-sorted 
	// objects.  Performance overhead; should be used only where specifically required
	CMPINLINE void XM_CALLCONV		SubmitForZSortedRendering(RenderQueueShader shader, Model *model, const RM_Instance & instance, const CXMVECTOR position)
	{
		// Compute the z-value as the distance squared from this object to the camera
		int z = (int)XMVectorGetX(XMVector3LengthSq(position - m_camera->GetPosition()));

		// Add to the z-sorted vector with this z-value as the sorting key
		m_renderqueueshaders[(int)shader].SortedInstances.push_back(RM_ZSortedInstance(z, model->GetModelBuffer(), instance));
	}

	// Method to submit for z-sorted rendering.  Should be used for any techniques (e.g. alpha blending) that require reverse-z-sorted 
	// objects.  Performance overhead; should be used only where specifically required
	CMPINLINE void XM_CALLCONV		SubmitForZSortedRendering(RenderQueueShader shader, ModelBuffer *model, const RM_Instance & instance, const CXMVECTOR position)
	{
		// Compute the z-value as the distance squared from this object to the camera
		int z = (int)XMVectorGetX(XMVector3LengthSq(position - m_camera->GetPosition()));

		// Add to the z-sorted vector with this z-value as the sorting key
		m_renderqueueshaders[(int)shader].SortedInstances.push_back(RM_ZSortedInstance(z, model, instance));
	}

	// Render an object with a static model.  Protected; called only from RenderObject()
	void                    RenderObjectWithStaticModel(iObject *object);

	// Render an object with an articulated model.  Protected; called only from RenderObject()
	void                    RenderObjectWithArticulatedModel(iObject *object);

	// Renders the entire contents of an environment tree node.  Internal method; no parameter checking
	void					RenderObjectEnvironmentNodeContents(iSpaceObjectEnvironment *environment, EnvironmentTree *node);

	// Lighting configuration is stored within the core engine and set for each object being rendered

	// Render variants for specific scenarios, e.g. specifically for 2D rendering
	AXMMATRIX				m_baseviewmatrix;		// Base view matrix for all 2D rendering

	// Functions for processing the per-frame render info
	EngineRenderInfoData	m_renderinfo;
	void                    ResetRenderInfo(void);

	// Pre-populated parameter sets for greater efficiency at render time, since only specific components need to be updated
	XMFLOAT4				m_instanceparams;

	// Reference to the model buffer currently being rendered by the render queue
	ModelBuffer *			m_current_modelbuffer;

	// Vector used to queue up actors for rendering.  This allows us to render them all at once, avoiding multiple state changes
	std::vector<Actor*>		m_queuedactors;

	// Vector of flags that determine which render stages will be performed
	std::vector<bool>		m_renderstages;

	// Vector of flags that determine special rendering states/effects
	std::vector<bool>		m_renderflags;

	// Cached & precalculated fields used for rendering an environment
	AXMVECTOR					m_cache_zeropoint;								// World position of the (0,0,0) element, i.e. corner of the environment
	AXMVECTOR_P					m_cache_el_inc[3];								// World position delta to move +1 element in each local dimension
	AXMVECTOR_P					m_cache_el_inc_base[3];							// Base world position delta to move +1 element in each local dimension (transformed each frame)
	std::vector<EnvironmentTree*> m_tmp_envnodes;								// Temporary vector of environment tree nodes being processed for rendering
	std::vector<Game::ID_TYPE>	m_tmp_renderedtiles;							// Temporary vector of tile IDs that have been rendered this cycle
	std::vector<Game::ID_TYPE>	m_tmp_renderedobjects;							// Temporary vector of object IDs that have been rendered this cycle
	std::vector<Game::ID_TYPE>	m_tmp_renderedterrain;							// Temporary vector of terrain IDs that have been rendered this cycle
	
	// Persistent storage for objects being debug-rendered
	Game::ID_TYPE 				m_debug_renderenvboxes;
	Game::ID_TYPE				m_debug_renderenvtree;
	Game::ID_TYPE				m_debug_renderenvnetwork;
	float						m_debug_renderobjid_distance;
	std::vector<SentenceType*> 
								m_debug_renderobjid_text;

	// Enumeration of possible debug terain render modes
	enum DebugTerrainRenderMode { Normal = 0, Solid };

	// Debug terrain rendering mode
	DebugTerrainRenderMode		m_debug_terrain_render_mode;

};



#endif