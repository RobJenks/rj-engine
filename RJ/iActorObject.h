/*#pragma once

#ifndef __iActorObjectH__
#define __iActorObjectH__

class ActorModel;
#include "iSpaceObject.h"
#include "SkinnedModel.h"

class iActorObject : public iSpaceObject
{
public:

	// Methods to get the absolute position & orientation of this actor, rather than its relative position within its current environment
	virtual D3DXVECTOR3				GetAbsolutePosition(void) const			= 0;
	virtual D3DXQUATERNION			GetAbsoluteOrientation(void) const		= 0;

	// Get or set the model for this actor
	virtual SkinnedModelInstance 	GetModel(void)							= 0;
	virtual SkinnedModelInstance *	GetModelReference(void)					= 0;
	virtual void					SetModel(SkinnedModelInstance model)	= 0;

	// Assign or return the current environment for this actor
	virtual iSpaceObject *			GetParentEnvironment(void) const		= 0;
	virtual void					MoveIntoEnvironment(iSpaceObject *env)	= 0;

	//Constructor & destructor
	iActorObject(void);
	~iActorObject(void);
};



#endif*/