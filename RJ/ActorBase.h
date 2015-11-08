#pragma once

#ifndef __ActorBaseH__
#define __ActorBaseH__

#include <string>
#include "ActorAttributes.h"
class SkinnedModel;
class AnimationClip;
class Actor;

// Base actor class
// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ActorBase : public ALIGN16<ActorBase>
{
public:

	// Default constructor/destructor
	ActorBase(void);
	~ActorBase(void);

	// Method to create a new instance of this actor
	Actor *									CreateInstance(void);

	// Base attributes for this actor type
	ActorBaseAttributes						Attributes;

	// Methods to get and set the actor code/string name, and other key actor fields
	CMPINLINE std::string					GetCode(void)							{ return m_code; }
	CMPINLINE void							SetCode(const std::string & code)		{ m_code = code; }
	CMPINLINE std::string					GetName(void)							{ return m_name; }
	CMPINLINE void							SetName(const std::string & name)		{ m_name = name; }
	CMPINLINE XMVECTOR						GetSize(void) const						{ return m_size; }

	// Methods to get or set the model assigned to this actor type
	CMPINLINE const SkinnedModel *			GetModel(void)							{ return m_model; }
	Result									SetModel(SkinnedModel *model);
	Result									SetModel(const std::string & model);

	// Methods to get or set other key actor fields
	CMPINLINE float							GetMass(void) const						{ return m_mass; }
	CMPINLINE void							SetMass(float m)						{ m_mass = m; }

	// Set or retrieve key animations for this actor type
	CMPINLINE const AnimationClip *			GetDefaultAnimation(void)						{ return m_defaultanimation; }
	CMPINLINE void							SetDefaultAnimation(const AnimationClip *anim)	{ m_defaultanimation = anim; }


	// Head-bob parameters are stored per actor type, but only applicable when the player is controlling that actor
	// TODO: Change to be relative to actor run speed (and then it should be calculated from an instance upon applying player
	// control.  Or link to animation.  But run speed likely the best option.
	CMPINLINE float				GetHeadBobAmount(void)			{ return m_headbobamount; }
	CMPINLINE float				GetHeadBobSpeed(void)			{ return m_headbobspeed; }
	CMPINLINE void				SetHeadBobAmount(float amount)	{ m_headbobamount = amount; }
	CMPINLINE void				SetHeadBobSpeed(float speed)	{ m_headbobspeed = speed; }

	// Offset of the player view from the actor position, for use when controlling this actor.  Read-only, determined
	// by the model data assigned to this actor
	CMPINLINE XMVECTOR			GetViewOffset(void) const				{ return m_viewoffset; }

protected:

	// Basic details for the actor type
	std::string									m_code;
	std::string									m_name;

	// Reference to the model data for this type of actor
	SkinnedModel *								m_model;

	// Size of the skinned model, allowing for any scaling applied at the model level
	AXMVECTOR									m_size;

	// Other key object properties relevant for actors
	float										m_mass;

	// References to key animation sequences, stored for efficiency of rendering
	const AnimationClip *						m_defaultanimation;


	// Player-specific parameters, relevant if the player takes control of an actor
	AXMVECTOR									m_viewoffset;					
	float										m_headbobspeed, m_headbobamount;

};


#endif