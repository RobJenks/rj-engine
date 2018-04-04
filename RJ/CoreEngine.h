#pragma once

#ifndef __CoreEngineH__
#define __CoreEngineH__

#include <array>
#include <unordered_map>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "GlobalFlags.h"
#include "DX11_Core.h"
#include "Rendering.h"
#include "RenderDeviceDX11.h"
#include "RenderAssetsDX11.h"

#include "Profiler.h"
#include "CameraClass.h"
#include "iAcceptsConsoleCommands.h"
#include "RenderQueue.h"
#include "RenderQueueShaders.h"
#include "RenderQueueOptimiser.h"
#include "ShaderManager.h"
#include "Model.h"
#include "ModelBuffer.h"
#include "Frustum.h"
#include "ViewPortal.h"
#include "BasicColourDefinition.h"
#include "ShaderRenderPredicate.h"
#include "ModelRenderPredicate.h"
#include "PipelineStateDX11.h"
class iShader;
class ModelBuffer;
class LightShader;
class LightFadeShader;
class LightHighlightShader;
class LightHighlightFadeShader;
class LightFlatHighlightFadeShader;
class ParticleShader;
class TextureShader;
class TexcubeShader;
class FireShader;
class SkinnedNormalMapShader;
class Frustum;
class Light;
class LightingManagerObject;
class FontShader;
class AudioManager;
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

// Debug rendering compiler flags
#define ENABLE_PORTAL_RENDERING_DEBUG_MODE

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

	// Enumeration of render flags
	enum RenderFlag
	{
		None = 0,
		RenderTree,
		RenderEnvTree,
		DisableHullRendering,
		RenderOBBs,
		RenderOBBLeafNodesOnly,
		RenderTerrainBoxes,
		RenderNavNetwork,
		RenderObjectIdentifiers,
		_RFLAG_COUNT
	};

	// Methods to get and set render flags (to potentially be replaced by a parameterised method in future)
	CMPINLINE bool	GetRenderFlag(RenderFlag flag)							{ return m_renderflags[flag]; }
	CMPINLINE void	SetRenderFlag(RenderFlag flag, bool value)				{ m_renderflags[flag] = value; }

	// Constructor/destructor/copy constructor/assignment operator
	CoreEngine(void);
	~CoreEngine(void);

	// Initialise all components of the game engine
	Result					InitialiseGameEngine(HWND hwnd);

	// Perform any post-data-load activities, e.g.retrieving models that have now been loaded
	Result					PerformPostDataLoadInitialisation(void);

	// Release all components of the game engine as part of a controlled shutdown
	void					ShutdownGameEngine();
	
	// Accessor/modifier methods for key game engine components
	CMPINLINE RenderDeviceDX11 * GetRenderDevice()		{ return m_renderdevice; }
	CMPINLINE CameraClass	*GetCamera()				{ return m_camera; }
	CMPINLINE Frustum		*GetViewFrustrum()			{ return m_frustrum; }
	CMPINLINE TextManager	*GetTextManager()			{ return m_textmanager; }
	CMPINLINE EffectManager *GetEffectManager()			{ return m_effectmanager; }
	CMPINLINE ParticleEngine *GetParticleEngine()		{ return m_particleengine; }
	CMPINLINE Render2DManager *Get2DRenderManager()		{ return m_render2d; }
	CMPINLINE OverlayRenderer *GetOverlayRenderer()		{ return m_overlayrenderer; }
	CMPINLINE AudioManager	  *GetAudioManager()		{ return m_audiomanager; }

	// Methods to retrieve the key render matrices from the engine
	CMPINLINE const XMMATRIX & GetRenderViewMatrix(void) const							{ return r_view; }
	CMPINLINE const XMMATRIX & GetRenderInverseViewMatrix(void) const					{ return r_invview; }
	CMPINLINE const XMMATRIX & GetRenderProjectionMatrix(void) const					{ return r_projection; }
	CMPINLINE const XMMATRIX & GetRenderInverseProjectionMatrix(void) const				{ return r_invproj; }
	CMPINLINE const XMMATRIX & GetRenderOrthographicMatrix(void) const					{ return r_orthographic; }
	CMPINLINE const XMMATRIX & GetRenderInverseOrthographicMatrix(void) const			{ return r_invorthographic; }
	CMPINLINE const XMMATRIX & GetRenderViewProjectionMatrix(void) const				{ return r_viewproj; }
	CMPINLINE const XMMATRIX & GetRenderInverseViewProjectionMatrix(void) const			{ return r_invviewproj; }
	CMPINLINE const XMMATRIX & GetRenderViewProjectionScreenMatrix(void) const			{ return r_viewprojscreen; }
	CMPINLINE const XMMATRIX & GetRenderInverseViewProjectionScreenMatrix(void) const	{ return r_invviewprojscreen; }
	CMPINLINE const XMFLOAT4X4 & GetRenderViewMatrixF(void) const						{ return r_view_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderInverseViewMatrixF(void) const				{ return r_invview_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderProjectionMatrixF(void) const					{ return r_projection_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderInverseProjectionMatrixF(void) const			{ return r_invproj_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderOrthographicMatrixF(void) const				{ return r_orthographic_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderInverseOrthographicMatrixF(void) const		{ return r_invorthographic_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderViewProjectionMatrixF(void) const				{ return r_viewproj_f; }
	CMPINLINE const XMFLOAT4X4 & GetRenderInverseViewProjectionMatrixF(void) const		{ return r_invviewproj_f; }

	// Pass-through accessor methods for key engine components
	CMPINLINE Rendering::RenderDeviceType * 			GetDevice(void)			{ return m_renderdevice->GetDevice(); }
	CMPINLINE Rendering::RenderDeviceContextType *		GetDeviceContext(void)	{ return m_renderdevice->GetDeviceContext(); }

	// Pass-through accessor for engine assets, since they are used so frequently
	CMPINLINE RenderAssetsDX11 &						GetAssets(void) { return m_renderdevice->Assets; }

	// Validation method to determine whether the engine has all critical frame-generatation components available
	CMPINLINE bool			Operational()				{ return (m_renderdevice && m_renderdevice->GetDevice() ); }

	// Methods to perform initialisation/tear-down for a frame; not currently used
	CMPINLINE void			BeginFrame()				{ }
	CMPINLINE void			EndFrame()					{ }


	/* *** Main rendering function *** */
	void					Render(void);

	// Process all items in the queue via instanced rendering.  All instances for models passing the supplied render predicates
	// will be rendered through the given rendering pipeline
	template <class TShaderRenderPredicate = ShaderRenderPredicate::RenderAll, class TModelRenderPredicate = ModelRenderPredicate::RenderAll>
	void					ProcessRenderQueue(PipelineStateDX11 *pipeline);

	// Perform instanced rendering for a model and a set of instance data; generally called by the render queue but can be 
	// invoked by other processes (e.g. for deferred light volume rendering).  A material can be supplied that will override
	// the material specified in the model buffer; a null material will fall back to the default model buffer material
	void					RenderInstanced(const PipelineStateDX11 & pipeline, const ModelBuffer & model, const MaterialDX11 * material, const RM_Instance & instance_data, UINT instance_count);

	// Clear the render queue.  No longer performed during render queue processing since we need to be able to process all render
	// queue items multiple times through e.g. different shader pipelines
	void					ClearRenderQueue(void);


	// Method to render the system region
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_SystemRegion, 
		void, RenderSystemRegion, void, )

	// Method to render the immediate region surrounding the player
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ImmediateRegion, 
		void, RenderImmediateRegion, void, )

	// Renders all objects in the specified system, based on simulation state and visibility testing
	void					RenderAllSystemObjects(SpaceSystem & system);

    // Generic iObject rendering method; used by subclasses wherever possible.  Returns a flag indicating whether
	// anything was rendered
	bool                    RenderObject(iObject *object);

	// Simple ship-rendering method
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_SimpleShips,
		void, RenderSimpleShip, SimpleShip *s, s)

	// Renders a complex ship, including all section and interior contents if applicable
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ComplexShips, 
		void, RenderComplexShip, SINGLE_ARG(ComplexShip *ship, bool renderinterior), SINGLE_ARG(ship, renderinterior))

	// Methods to render parts of a complex ship
	bool					RenderComplexShipSection(ComplexShip *ship, ComplexShipSection *sec);
	void					RenderComplexShipTile(ComplexShipTile *tile, iSpaceObjectEnvironment *environment);

	// Renders a collection of turrets that have already been updated by their turret controller
	void					RenderTurrets(TurretController & controller);

	// Renders all elements of a projectile set which are currently visible
	void					RenderProjectileSet(BasicProjectileSet & projectiles);

	// RenderObjectEnvironments(iSpaceObjectEnvironment *environment)
	// Method to render the interior of an object environment, including any tiles, objects or terrain within it
	RJ_ADDPROFILE(Profiler::ProfiledFunctions::Prf_Render_ObjectEnvironments,
		void, RenderEnvironment, SINGLE_ARG(iSpaceObjectEnvironment *environment, const Frustum **pOutGlobalFrustum), 
								 SINGLE_ARG(environment, pOutGlobalFrustum))

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
	CMPINLINE void RJ_XM_CALLCONV			RenderModel(ModelBuffer *model, const FXMMATRIX world, const CXMVECTOR position)
	{
		// Render using the standard light shader.  Add to the queue for batched rendering.
		SubmitForRendering(RenderQueueShader::RM_LightShader, model, NULL, std::move(
			RM_Instance(world, RM_Instance::CalculateSortKey(position))));
	}

	// Renders a standard model.  Applies highlighting to the model
	CMPINLINE void			RenderModel(ModelBuffer *model, const XMFLOAT4 & highlight, const CXMMATRIX world, const CXMVECTOR position)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		SubmitForRendering(RenderQueueShader::RM_LightHighlightShader, model, NULL, std::move(
			RM_Instance(world, RM_Instance::CalculateSortKey(position), highlight)));
	}

	// Renders a standard model.  Applies alpha fade to the model
	CMPINLINE void			RenderModel(ModelBuffer *model, const FXMVECTOR position, float alpha, const CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		//m_instanceparams.x = alpha;
		//SubmitForZSortedRendering(RenderQueueShader::RM_LightFadeShader, model, world, m_instanceparams, position);
		
		// TODO: Need to re-enable transparent object rendering
	}

	// Renders a standard model.  Applies highlighting and alpha fade to the model
	CMPINLINE void			RenderModel(ModelBuffer *model, const FXMVECTOR position, const XMFLOAT3 & highlight, float alpha, CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		//m_instanceparams = XMFLOAT4(highlight.x, highlight.y, highlight.z, alpha);
		//SubmitForZSortedRendering(RenderQueueShader::RM_LightHighlightFadeShader, model, world, m_instanceparams, position);

		// TODO: Need to re-enable transparent object rendering
	}

	// Renders a standard model.  Applies highlighting and alpha fade to the model (specified in xyz and w components respectively)
	CMPINLINE void			RenderModel(ModelBuffer *model, const FXMVECTOR position, const XMFLOAT4 & colour_alpha, CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		//m_instanceparams = colour_alpha;
		//SubmitForZSortedRendering(RenderQueueShader::RM_LightHighlightFadeShader, model, world, m_instanceparams, position);

		// TODO: Need to re-enable transparent object rendering
	}

	// Renders a standard model using flat lighting.  Applies highlighting and alpha fade to the model (specified in xyz and w components respectively)
	CMPINLINE void			RenderModelFlat(ModelBuffer *model, const FXMVECTOR position, const XMFLOAT3 & highlight, float alpha, CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		//m_instanceparams = XMFLOAT4(highlight.x, highlight.y, highlight.z, alpha);
		//SubmitForZSortedRendering(RenderQueueShader::RM_LightFlatHighlightFadeShader, model, world, m_instanceparams, position);

		// TODO: Need to re-enable transparent object rendering
	}
	
	// Renders a standard model using flat lighting.  Applies highlighting and alpha fade to the model (specified in xyz and w components respectively)
	CMPINLINE void			RenderModelFlat(ModelBuffer *model, const FXMVECTOR position, const XMFLOAT4 & colour_alpha, CXMMATRIX world)
	{
		// Use the highlight shader to apply a global highlight to the model.  Add to the queue for batched rendering
		//m_instanceparams = colour_alpha;
		//SubmitForZSortedRendering(RenderQueueShader::RM_LightFlatHighlightFadeShader, model, world, m_instanceparams, position);

		// TODO: Need to re-enable transparent object rendering
	}

	// Submit a material directly for orthographic rendering (of its diffuse texture) to the screen
	void RJ_XM_CALLCONV						RenderMaterialToScreen(MaterialDX11 & material, const XMFLOAT2 & position, const XMFLOAT2 size, float rotation = 0.0f);

	// Primitive topology is managed by the render queue in an attempt to minimise state changes
	CMPINLINE D3D_PRIMITIVE_TOPOLOGY		GetCurrentPrimitiveTopology(void) const { return m_current_topology; }
	void									ChangePrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitive_topology);
	void									ChangePrimitiveTopologyIfRequired(D3D_PRIMITIVE_TOPOLOGY primitive_topology);


	// Performs rendering of debug/special data
	void RenderDebugData(void);
	void DebugRenderSpaceCollisionBoxes(void);
	void DebugRenderEnvironmentCollisionBoxes(void);
	void DebugRenderSpatialPartitioningTree(void);
	void DebugRenderEnvironmentTree(void);
	void DebugRenderEnvironmentNavNetwork(void);
	void DebugRenderObjectIdentifiers(void);
	void DebugRenderPortalTraversal(void);

	// Gets or sets the environment that is the subject of debug rendering
	CMPINLINE Game::ID_TYPE GetDebugTerrainRenderEnvironment(void) const { return m_debug_renderenvboxes; }
	CMPINLINE void SetDebugTerrainRenderEnvironment(Game::ID_TYPE environment_id) { m_debug_renderenvboxes = environment_id; }
	CMPINLINE Game::ID_TYPE GetDebugTreeRenderEnvironment(void) const { return m_debug_renderenvtree; }
	CMPINLINE void SetDebugTreeRenderEnvironment(Game::ID_TYPE environment_id) { m_debug_renderenvtree = environment_id; }
	CMPINLINE Game::ID_TYPE GetDebugNavNetworkRenderEnvironment(void) const { return m_debug_renderenvnetwork; }
	CMPINLINE void SetDebugNavNetworkRenderEnvironment(Game::ID_TYPE environment_id) { m_debug_renderenvnetwork = environment_id; }
	CMPINLINE Game::ID_TYPE GetDebugObjectIdentifierRenderTargetObject(void) const { return m_debug_renderobjid_object; }
	CMPINLINE void SetDebugObjectIdentifierRenderTargetObject(Game::ID_TYPE id) { m_debug_renderobjid_object = id; }
	CMPINLINE float GetDebugObjectIdentifierRenderingDistance(void) const { return m_debug_renderobjid_distance; }
	CMPINLINE void SetDebugObjectIdentifierRenderingDistance(float dist) { m_debug_renderobjid_distance = clamp(dist, 1.0f, 100000.0f); }
	CMPINLINE Game::ID_TYPE GetDebugPortalRenderingTarget(void) const { return m_debug_renderportaltraversal; }
	CMPINLINE void SetDebugPortalRenderingTarget(Game::ID_TYPE environment_id) { m_debug_renderportaltraversal = environment_id; }

	
	// Structure keeping track of render info per frame
	struct EngineRenderInfoData
	{
		size_t DrawCalls;
		size_t ShipRenderCount;
		size_t ComplexShipRenderCount;
		size_t ComplexShipSectionRenderCount;
		size_t ComplexShipTileRenderCount;
		size_t ActorRenderCount;
		size_t TerrainRenderCount;

		size_t InstanceCount;
		size_t InstanceCountZSorted;
		size_t InstanceCountSkinnedModel;
	};

	// Function to return the per-frame render info
	CMPINLINE EngineRenderInfoData GetRenderInfo(void) { return m_renderinfo; }

	// Set the visibility state of the system cursor
	void SetSystemCursorVisibility(bool cursor_visible);

	// Event triggered when the application window is resized
	void WindowResized(void);

	// Virtual inherited method to accept a command from the console
	bool ProcessConsoleCommand(GameConsoleCommand & command);

	// Retrieve key game engine parameters
	CMPINLINE HWND GetHWND(void) const			{ return m_hwnd; }
	CMPINLINE bool VsyncEnabled(void) const		{ return m_vsync; }

	// Central shader manager for the engine
	ShaderManager			ShaderManager;

	// Manager for all lighting data
	LightingManagerObject *	LightingManager;

	// Return a flag indicating whether a particular render stage is active this cycle
	CMPINLINE bool RenderStageActive(RenderStage stage) { return m_renderstages[(int)stage]; }

	// Activate or deactivate a particular stage of the rendering cycle.  Changing the 'All' stage will overwrite all stage values
	void SetRenderStageState(RenderStage stage, bool active);

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
	void				DetermineObjectScreenBounds(const OrientedBoundingBox::CoreOBBData & obb, XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds);
	CMPINLINE void		DetermineObjectScreenBounds(const iObject & obj, XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds)
	{
		DetermineObjectScreenBounds(obj.CollisionOBB.ConstData(), outMinBounds, outMaxBounds);
	}

	// Calculates the bounds of this object in screen space, after applying a world-space offset to the object position
	void				DetermineObjectScreenBoundsWithWorldSpaceOffset(const OrientedBoundingBox::CoreOBBData & obb, const FXMVECTOR world_offset,
																		XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds);

	// Returns a position in screen space corresponding to the specified object.  Accepts an offset parameter
	// in screen coordinates based on the object size; [0, +0.5] would be centred in x, and return a position
	// at the top edge of the object in screen space
	XMVECTOR				GetScreenLocationForObject(const OrientedBoundingBox::CoreOBBData & obb, const XMFLOAT2 & offset);
	CMPINLINE XMVECTOR		GetScreenLocationForObject(const iObject & obj, const XMFLOAT2 & offset)
	{
		return GetScreenLocationForObject(obj.CollisionOBB.ConstData(), offset);
	}

	// Returns a position in screen space corresponding to the specified object, after applying the specified
	// world offset to object position.  Accepts an offset parameter in screen coordinates based on the 
	// object size; [0, +0.5] would be centred in x, and return a position at the top edge of the object in screen space
	XMVECTOR				GetScreenLocationForObjectWithWorldOffset(const OrientedBoundingBox::CoreOBBData & obb,
																	  const FXMVECTOR world_offset, const XMFLOAT2 & offset);
	
	// Outputs the contents of the render queue to debug-out
	void					DebugOutputRenderQueueContents(void);

	// Override the initial portal rendering frustum; relevant in debug builds only.  Override frustum will be deallocated by the
	// engine at the end of the frame
	void					DebugOverrideInitialPortalRenderingViewer(const iObject *viewer); 

	// Static constant collection of basic colours, for ease of use in e.g. simple debug overlays
	static const std::array<BasicColourDefinition, 8> BASIC_COLOURS;


private:
	
	// Private methods to initialise each component in turn
	Result					InitialiseRenderDevice(HWND hwnd);
	Result					InitialiseDirectXMath(void);
	Result					InitialiseRenderQueue(void);
	Result					InitialiseRenderFlags(void);
	Result					InitialiseCamera(void);
	Result					InitialiseShaderSupport(void);
	Result					InitialiseFrustrum(void);
	Result					InitialiseAudioManager(void);
	Result					InitialiseLightingManager(void);
	Result					InitialiseTextRendering(void);
	Result					InitialiseEffectManager(void);
	Result					InitialiseParticleEngine(void);
	Result					Initialise2DRenderManager(void);
	Result					InitialiseOverlayRenderer(void);
	Result					InitialiseEnvironmentRendering(void);

	// Private methods to release each component in turn
	void					ShutdownRenderDevice(void);
	void					ShutdownDXMath(void);
	void					ShutdownRenderQueue(void);
	void					ShutdownTextureData(void);
	void					ShutdownCamera(void);
	void					ShutdownShaderSupport(void);
	void					ShutdownFrustrum(void);
	void					ShutdownAudioManager(void);
	void					ShutdownLightingManager(void);
	void					ShutdownTextRendering(void);
	void					ShutdownEffectManager(void);
	void					ShutdownParticleEngine(void);
	void					Shutdown2DRenderManager(void);
	void					ShutdownOverlayRenderer(void);
	void					ShutdownEnvironmentRendering(void);

	// Post-data load initialisation for the core engine component itself
	Result					PerformInternalEnginePostDataLoadInitialisation(void);

	// Update window size details based on these parameters, recalculating for windowed mode as required
	//void					UpdateWindowSizeParameters(int screenWidth, int screenHeight, bool fullscreen);

	// Pointer to each component that makes up this game engine
	RenderDeviceDX11 *				m_renderdevice;
	CameraClass						*m_camera;
	LightShader						*m_lightshader;
	LightFadeShader					*m_lightfadeshader;
	LightHighlightShader			*m_lighthighlightshader;
	LightHighlightFadeShader		*m_lighthighlightfadeshader;
	LightFlatHighlightFadeShader 	*m_lightflathighlightfadeshader;
	ParticleShader					*m_particleshader;
	TextureShader					*m_textureshader;
	Frustum							*m_frustrum;
	FontShader						*m_fontshader;
	AudioManager					*m_audiomanager;
	TextManager						*m_textmanager;
	TexcubeShader					*m_texcubeshader;
	FireShader						*m_fireshader;
	EffectManager					*m_effectmanager;
	SkinnedNormalMapShader 			*m_skinnedshader;
	VolLineShader					*m_vollineshader;
	ParticleEngine					*m_particleengine;
	Render2DManager					*m_render2d;
	OverlayRenderer					*m_overlayrenderer;

	// Game engine parameters
	HWND					m_hwnd;							// Handle to the target window
	bool					m_vsync;						// Flag determining whether vsync is enabled

	// Render-cycle-specific parameters; denoted by r_*, these are valid for the current render cycle only 
	// and are used for reasons of render efficiency
	Rendering::RenderDeviceContextType *	
							r_devicecontext;		// The device context in use for this render cycle
	AXMMATRIX				r_view;					// View matrix for the current render cycle
	AXMMATRIX				r_projection;			// Projection matrix for the current render cycle
	AXMMATRIX				r_orthographic;			// Orthographic matrix for the current render cycle
	AXMMATRIX				r_invview;				// We will also store the inverse view matrix
	AXMMATRIX				r_invproj;				// We will also store the inverse projection matrix
	AXMMATRIX				r_invorthographic;		// We will also store the inverse orthographic matrix
	AXMMATRIX				r_viewproj;				// Store the combined (view * proj) matrix
	AXMMATRIX				r_invviewproj;			// Also store the inverse viewproj matrix, i.e. (view * proj)^-1
	AXMMATRIX				m_projscreen;			// Adjustment matrix from projection to screen coordinates (only recalculated when screen parameters change)
	AXMMATRIX				r_viewprojscreen;		// Combined (view * proj * screen) matrix
	AXMMATRIX				r_invviewprojscreen;	// Inverse of the view/proj/screen transform matrix
	XMFLOAT4X4				r_view_f;				// Local float representation of the current frame view matrix
	XMFLOAT4X4				r_projection_f;			// Local float representation of the current frame projection matrix
	XMFLOAT4X4				r_orthographic_f;		// Local float representation of the current frame orthographic matrix
	XMFLOAT4X4				r_invview_f;			// Local float representation of the current frame inverse view matrix
	XMFLOAT4X4				r_invproj_f;			// Local float representation of the current frame inverse projection matrix
	XMFLOAT4X4				r_invorthographic_f;	// Local float representation of the current frame inverse orthographic matrix
	XMFLOAT4X4				r_viewproj_f;			// Local float representation of the current frame (view * proj) matrix
	XMFLOAT4X4				r_invviewproj_f;		// Local float representation of the current frame inverse (view * proj) matrix


	VertexBufferDX11 *			m_instancebuffer;
	RenderQueue					m_renderqueue;
	RM_ShaderCollection			m_renderqueueshaders;
	const ID3D11Buffer *		m_instancedbuffers[2];
	unsigned int				m_instancedstride[2], m_instancedoffset[2];
	D3D_PRIMITIVE_TOPOLOGY		m_current_topology;

	// Optimiser performs periodic maintenance on the engine render queue
	RenderQueueOptimiser		m_rq_optimiser;

	// Cached reference to unit quad model, used for direct screen-space rendering of materials
	Model *						m_unit_quad_model;

	// Clear the render queue.  Not required per-frame; invoked on shutdown to clear down resources
	void								DeallocateRenderingQueue(void);

	// Performs an intermediate z-sorting of instances before rendering via the render queue.  Used only for shaders/techniques (e.g. alpha
	// blending) that require instances to be z-sorted
	void								PerformZSortedRenderQueueProcessing(RM_InstancedShaderDetails & shader);

	
	// Submit a model buffer to the render queue manager for rendering this frame.  A material can be supplied that
	// overrides any default material specified in the model buffer.  A material of NULL will use the default 
	// material in the model buffer
	void RJ_XM_CALLCONV					SubmitForRendering(RenderQueueShader shader, ModelBuffer *model, MaterialDX11 *material, RM_Instance && instance);
	CMPINLINE void RJ_XM_CALLCONV		SubmitForRendering(RenderQueueShader shader, Model *model, MaterialDX11 *material, RM_Instance && instance)
	{
		if (model) SubmitForRendering(shader, &(model->Data), material, std::move(instance));
	}

	// Method to submit for z-sorted rendering.  Should be used for any techniques (e.g. alpha blending) that require reverse-z-sorted 
	// objects.  Performance overhead; should be used only where specifically required
	void RJ_XM_CALLCONV					SubmitForZSortedRendering(RenderQueueShader shader, ModelBuffer *model, RM_Instance && instance, const CXMVECTOR position);
	CMPINLINE void RJ_XM_CALLCONV		SubmitForZSortedRendering(RenderQueueShader shader, Model *model, RM_Instance && instance, const CXMVECTOR position)
	{
		if (model) SubmitForZSortedRendering(shader, &(model->Data), std::move(instance), position);
	}


	/* Method to render the interior of an object environment including any tiles, for an environment
	   which supports portal rendering
	    - environment			The environment to be rendered
		- view_position			Position of the viewer in world space
		- initial_frustum		The initial view frustum, generally the global engine ViewFrustum
		- pOutGlobalFrustum		Output parameter.  Passes a newly-constructed frustum object back to the 
								caller if rendering of the environment resulted in a more restrictive 
								global visibility frustum
		- Returns				A result code indicating whether the environment could be rendered
								via environment portal rendering
	*/
	Result					RenderPortalEnvironment(iSpaceObjectEnvironment *environment, const FXMVECTOR viewer_position, Frustum *initial_frustum, const Frustum **pOutGlobalFrustum);

	/* Method to render the interior of an object environment including any tiles, for an environment
	   which does not support portal rendering or where the viewer state does not permit it
	      - environment:		The environment to be rendered
	      - pOutGlobalFrustum:	Output parameter.  Passes a newly-constructed frustum object back to the
								caller if rendering of the environment resulted in a more restrictive
								global visibility frustum
	*/
	void					RenderNonPortalEnvironment(iSpaceObjectEnvironment *environment, const Frustum **pOutGlobalFrustum);
	
	// Calculates the bounds of a portal in world space, by transforming into view space and determining the
	// portal extents and then transforming those points back into world space
	void					CalculateViewPortalBounds(const ViewPortal & portal, const FXMMATRIX portal_world_transform, const CameraClass & camera, AXMVECTOR(&pOutVertices)[4]) const;

	// Create a new view frustum by clipping the current frustum against the bounds of a view portal
	
	Frustum *				CreateClippedFrustum(const FXMVECTOR view_position, const Frustum & current_frustum, const ViewPortal & portal, const FXMMATRIX world_transform);
	
	// Debug-render an environment portal based on the given definition and world transform
	void					DebugRenderPortal(const ViewPortal & portal, const FXMMATRIX world_matrix, bool is_active);

	// Set the debug level for portal rendering, if it has been enabled for a specific environment
	void					SetDebugPortalRenderingConfiguration(bool debug_render, bool debug_log);

	// Render an object with a static model.  Protected; called only from RenderObject()
	void                    RenderObjectWithStaticModel(iObject *object);

	// Render an object with an articulated model
	void                    RenderObjectWithArticulatedModel(iObject *object);
	void					RenderTerrainWithArticulatedModel(Terrain *object);

	// Renders the entire contents of an environment tree node.  Internal method; no parameter checking
	void					RenderObjectEnvironmentNodeContents(iSpaceObjectEnvironment *environment, EnvironmentTree *node, const FXMVECTOR environment_relative_viewer_position);

	// Renders an object within a particular environment
	void					RenderEnvironmentObject(iEnvironmentObject *object);

	// Pre- and post-render debug processes; only active in debug builds
	void					RunPreRenderDebugProcesses(void);
	void					RunPostRenderDebugProcesses(void);

	// Retrieve render-cycle-specific data that will not change for the duration of the cycle.  Prefixed r_*
	void					RetrieveRenderCycleData(void);

	// Render variants for specific scenarios, e.g. specifically for 2D rendering
	AXMMATRIX				m_baseviewmatrix;		// Base view matrix for all 2D rendering

	// Functions for processing the per-frame render info
	EngineRenderInfoData	m_renderinfo;
	void                    ResetRenderInfo(void);

	// Pre-populated parameter sets for greater efficiency at render time, since only specific components need to be updated
	XMFLOAT4				m_instanceparams;

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
	std::vector<EnvironmentTree*>		m_tmp_envnodes;							// Temporary vector of environment tree nodes being processed for rendering
	std::vector<iEnvironmentObject*>	m_tmp_envobjects;						// Temporary vector of environment objects to be rendered
	std::vector<Terrain*>			m_tmp_terrain;							// Temporary vector of static terrain objects to be rendered
	std::vector<Frustum*>		m_tmp_frustums;									// Temporary vector of frustums maintained during portal rendering, to avoid repeated allocations

	std::vector<Game::ID_TYPE>	m_tmp_renderedtiles;							// Temporary vector of tile IDs that have been rendered this cycle
	std::vector<Game::ID_TYPE>	m_tmp_renderedobjects;							// Temporary vector of object IDs that have been rendered this cycle
	std::vector<Game::ID_TYPE>	m_tmp_renderedterrain;							// Temporary vector of terrain IDs that have been rendered this cycle
	
	// Persistent storage for objects being debug-rendered
	Game::ID_TYPE 				m_debug_renderenvboxes;
	Game::ID_TYPE				m_debug_renderenvtree;
	Game::ID_TYPE				m_debug_renderenvnetwork;
	Game::ID_TYPE				m_debug_renderportaltraversal;
	bool						m_debug_portal_debugrender;
	bool						m_debug_portal_debuglog;
	AXMVECTOR					m_debug_portal_render_viewer_position;			// Override that can be applied to act as the initial viewer position in portal rendering (rather than the camera position)
	Frustum *					m_debug_portal_render_initial_frustum;			// Override that can be applied to act as the initial frustum in portal rendering (rather than the standard view frustum)
	Game::ID_TYPE				m_debug_renderobjid_object;						// Specific object for which we should render IDs (generally an environment)
	float						m_debug_renderobjid_distance;					// Distance from camera within which we should render the ID of objects
	std::vector<SentenceType*> 
								m_debug_renderobjid_text;						// Vector of text objects for debug render object identifiers

	// Counter for allowable render device failures before application will consider it unrecoverable and terminate
	unsigned int				m_render_device_failure_count;
	static const unsigned int	ALLOWABLE_RENDER_DEVICE_FAILURE_COUNT = 10U;

	// Enumeration of possible debug terain render modes
	enum DebugTerrainRenderMode { Normal = 0, Solid };

	// Structure holding info required for debug ID rendering
	__declspec(align(16))
	struct DebugIDRenderDetails { 	
		OrientedBoundingBox::CoreOBBData obb; AXMVECTOR_P pos_offset; std::string text; XMFLOAT4 text_col; float text_size;
		DebugIDRenderDetails(const OrientedBoundingBox::CoreOBBData & _obb, const XMVECTOR & _pos_offset, 
			const std::string & _text, const XMFLOAT4 & _text_col, float _text_size) 
			: obb(_obb), pos_offset(_pos_offset), text(_text), text_col(_text_col), text_size(_text_size) { }
	};

	// Debug terrain rendering mode
	DebugTerrainRenderMode		m_debug_terrain_render_mode;

	// Debug rendering of environment portal traversal
#	if defined(_DEBUG) && defined(ENABLE_PORTAL_RENDERING_DEBUG_MODE)
#		define DEBUG_PORTAL_TRAVERSAL(environment, expr) \
			if (m_debug_renderportaltraversal == environment->GetID()) { expr; }
#		define DEBUG_PORTAL_TRAVERSAL_LOG(environment, text) \
			if (m_debug_portal_debuglog && m_debug_renderportaltraversal == environment->GetID()) { Game::Log << LOG_DEBUG << text; }
#		define DEBUG_PORTAL_RENDER(portal, world_matrix, is_active) \
			if (m_debug_portal_debugrender && m_debug_renderportaltraversal == environment->GetID()) { DebugRenderPortal(portal, world_matrix, is_active); }
#		define INITIAL_PORTAL_RENDERING_FRUSTUM \
			(m_debug_portal_render_initial_frustum ? m_debug_portal_render_initial_frustum : GetViewFrustrum())
#		define INITIAL_PORTAL_RENDERING_VIEWER_POSITION \
			(m_debug_portal_render_initial_frustum ? m_debug_portal_render_viewer_position : GetCamera()->GetPosition())
#	else
#		define DEBUG_PORTAL_TRAVERSAL(environment, expr) 
#		define DEBUG_PORTAL_TRAVERSAL_LOG(environment, text)
#		define DEBUG_PORTAL_RENDER(portal, world_matrix, is_active)
#		define INITIAL_PORTAL_RENDERING_FRUSTUM GetViewFrustrum()
#		define INITIAL_PORTAL_RENDERING_VIEWER_POSITION GetCamera()->GetPosition()
#	endif

};



#endif