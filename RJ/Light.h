#pragma once

#ifndef __LightH__
#define __LightH__

#include "ALIGN16.h"
#include "DX11_Core.h"
#include "GameConsoleCommand.h"
#include "Data\\Shaders\\light_definition.h"

class Light : public ALIGN16<Light>
{
public:

	// Enumeration of possible light types
	enum LightType					{ Directional = 0, PointLight, SpotLight };

	// Static counter used to assign unique light IDs
	static unsigned int				LAST_ID;

	// Lighting data
	LightData						Data;

	// Returns a flag indicating whether the light is currently active
	CMPINLINE bool					IsActive(void) const		{ return m_active; }

	// Sets the active/inactive status of this light
	CMPINLINE void					Activate(void) 				{ m_active = true; }
	CMPINLINE void					Deactivate(void)			{ m_active = false; }
	CMPINLINE void					SetIsActive(bool active)	{ m_active = active; }

	// Default constructor
	Light(void);

	// Constructor accepting the core light data struct as a parameter
	Light(const LightData & light_data);

	// Custom copy constructor
	Light(const Light & source);

	// Method to return a new unique ID
	static unsigned int				NewUniqueID(void);

	// Initialise a light to the specified type
	void							InitialiseDirectionalLight(const XMFLOAT3 & direction, const XMFLOAT3 & colour, float ambient, float diffuse, float specular);
	void							InitialisePointLight(	const XMFLOAT3 & position, const XMFLOAT3 & colour, float range, float ambient, 
															float diffuse, float specular, const AttenuationData & attenuation);
	void							InitialiseSpotLight(const XMFLOAT3 & position, const XMFLOAT3 & colour, float range,
														float ambient, float diffuse, float specular, const AttenuationData & attenuation,
														const XMFLOAT3 & direction, float inner_half_angle, float fade_half_angle);

	// Process a debug command from the console.  "Light" objects are not part of the object hierarchy, but 
	// members of that hierarchy will invokve this method when asked to perform lighting-related actions
	void							ProcessDebugCommand(GameConsoleCommand & command);

	// Accessor functions which operate on the underlying LightData structure
	CMPINLINE unsigned int GetID(void) const					{ return Data.ID; }
	CMPINLINE XMFLOAT3 GetColour(void) const					{ return Data.Colour; }
	CMPINLINE int GetType(void) const							{ return Data.Type; }
	CMPINLINE float GetAmbient(void) const						{ return Data.AmbientIntensity; }
	CMPINLINE float GetDiffuse(void) const						{ return Data.DiffuseIntensity; }
	CMPINLINE float GetSpecular(void) const						{ return Data.SpecularPower; }
	CMPINLINE XMFLOAT3 GetDirection(void) const					{ return Data.Direction; }
	CMPINLINE float GetSpotlightInnerHalfAngleCos(void) const	{ return Data.SpotlightInnerHalfAngleCos; }
	CMPINLINE float GetSpotlightOuterHalfAngleCos(void) const	{ return Data.SpotlightOuterHalfAngleCos; }
	CMPINLINE XMFLOAT3 GetPosition(void) const					{ return Data.Position; }
	CMPINLINE float GetRange(void) const						{ return Data.Range; }
	CMPINLINE AttenuationData GetAttenuation(void) const		{ return Data.Attenuation; }
	CMPINLINE float GetAttenuationConstant(void) const			{ return Data.Attenuation.Constant; }
	CMPINLINE float GetAttenuationLinear(void) const			{ return Data.Attenuation.Linear; }
	CMPINLINE float GetAttenuationExp(void) const				{ return Data.Attenuation.Exp; }

	// Mutator functions which operate on the underlying LightData structure
	CMPINLINE void SetID(unsigned int id)						{ Data.ID = id; }
	CMPINLINE void SetColour(const XMFLOAT3 & col)				{ Data.Colour = col; }
	CMPINLINE void SetType(int type)							{ Data.Type = type; }
	CMPINLINE void SetAmbient(float a)							{ Data.AmbientIntensity = a; }
	CMPINLINE void SetDiffuse(float d)							{ Data.DiffuseIntensity = d; }
	CMPINLINE void SetSpecular(float s)							{ Data.SpecularPower = s; }
	CMPINLINE void SetDirection(const XMFLOAT3 & dir)			{ Data.Direction = dir; }
	CMPINLINE void SetSpotlightInnerHalfAngleCos(float ca)		{ Data.SpotlightInnerHalfAngleCos = ca; }
	CMPINLINE void SetSpotlightOuterHalfAngleCos(float ca)		{ Data.SpotlightOuterHalfAngleCos = ca; }
	CMPINLINE void SetPosition(const XMFLOAT3 & pos)			{ Data.Position = pos; }
	CMPINLINE void SetRange(float r)							{ Data.Range = r; }
	CMPINLINE void SetAttenuation(const AttenuationData & a)	{ Data.Attenuation = a; }
	CMPINLINE void SetAttenuationConstant(float a)				{ Data.Attenuation.Constant = a; }
	CMPINLINE void SetAttenuationLinear(float a)				{ Data.Attenuation.Linear = a; }
	CMPINLINE void SetAttenuationExp(float a)					{ Data.Attenuation.Exp = a; }


	// Default destructor
	~Light(void);

protected:

	// Flag that indicates whether this light is active
	bool							m_active;

};




#endif