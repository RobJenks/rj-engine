#pragma once

#ifndef __RenderQueueH__
#define __RenderQueueH__

#include <vector>
#include <unordered_map>
#include "DX11_Core.h"
#include "D3DMain.h"
class iShader;
class Model;
class ModelBuffer;

/* This header contains all structure and type definitions for the render queue maintained within the core engine */

// Structure of a single instance in the instancing model.  16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
struct					RM_InstanceStructure
{
	XMMATRIX			World;								// World matrix to transform into the world
	XMVECTOR			Params;								// Float-4 of parameters that can be passed for each instance

	// Constructor where only the world transform is required; other params will be unitialised (for efficiency) and should not be used
	RM_InstanceStructure(const CXMMATRIX world) : World(world) {}

	// Constructor including additional per-instance parameters
	RM_InstanceStructure(const CXMMATRIX world, const CXMVECTOR params) : World(world), Params(params) {}

	// Empty constructor
	RM_InstanceStructure(void) { }
};													

// Single instance to be rendered; includes the instance-specific data such as transform & params
typedef						RM_InstanceStructure								RM_Instance;

// Instance collection & supporting data for a particular model
typedef struct				InstanceDataStruct {
	std::vector<RM_Instance>	InstanceData;
	int							TimeoutCounter;
	InstanceDataStruct(void) :	TimeoutCounter(0) { }
}																				RM_InstanceData;

// Collection holding a map of instances to be rendered using each model
typedef						std::unordered_map<	ModelBuffer*, RM_InstanceData >	RM_ModelInstanceData;

// Set of model/instance data applicable to each shader; the render queue
typedef						std::vector<RM_ModelInstanceData>					RM_RenderQueue;



// Structure to hold z-sorted instance data, used where objects must be sorted before processing the render queue
struct							RM_ZSortedInstance
{
	int							Key;
	ModelBuffer *				ModelPtr;
	RM_Instance					Item;

	bool operator<(const RM_ZSortedInstance & val) const	{ return (Key < val.Key); }

	RM_ZSortedInstance(int key, ModelBuffer *model, const CXMMATRIX world, const CXMVECTOR params) : 
		Key(key), ModelPtr(model), Item(RM_Instance(world, params)) { }

	RM_ZSortedInstance(int key, ModelBuffer *model, const RM_Instance & instance) :
		Key(key), ModelPtr(model), Item(instance) { }
};


// Details on a shader used in the render queue
struct							RM_InstancedShaderDetails
{
	iShader *					Shader;							// The shader itself
	bool						RequiresZSorting;				// Flag determining whether instances must go through an intermediate z-sorting step
	D3DMain::AlphaBlendState	AlphaBlendRequired;				// Flag indicating if/how alpha blending should be enabled for this shader
	D3D11_PRIMITIVE_TOPOLOGY	PrimitiveTopology;				// The primitive topology to be used for rendering in this shader

	std::vector<RM_ZSortedInstance>	SortedInstances;			// Vector used for the intermediate sorting step, where required, so that items 
																// are sent for rendering in a particular Z order

	// Default constructor, no reference to shader, sets all parameters to defaults
	RM_InstancedShaderDetails(void) : 
		Shader(NULL), RequiresZSorting(false), AlphaBlendRequired(D3DMain::AlphaBlendState::AlphaBlendDisabled), 
		PrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	{}

	// Constructor allowing all parameters to be specified
	RM_InstancedShaderDetails(iShader *shader, bool requiresZsorting, D3DMain::AlphaBlendState alphablend, D3D11_PRIMITIVE_TOPOLOGY primitive_topology) :
		Shader(shader), RequiresZSorting(requiresZsorting), AlphaBlendRequired(alphablend), PrimitiveTopology(primitive_topology)
	{}
};

typedef						std::vector<RM_InstancedShaderDetails>		RM_ShaderCollection;
	

// Enumeration of instancing-enabled shaders; ordered to minimise the number of rendering state changes required.  Also
// aim to have all render states back at defaults by the final shader; this avoids having to reset them after processing the queue
enum RenderQueueShader
{
	RM_LightShader = 0,					// Requires: none
	RM_LightHighlightShader,			// Requires: none

	/* Alpha blending cutoff; perform all alpha blend-enabled operations after this point */

	RM_LightFadeShader,					// Requires: alpha blending
	RM_LightHighlightFadeShader,		// Requires: alpha blending
	RM_VolLineShader,					// Requires: alpha blending

	RM_RENDERQUEUESHADERCOUNT			// Count of shaders that can be used within the render queue	
};


#endif