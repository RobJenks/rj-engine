#pragma once

#ifndef __LightH__
#define __LightH__

#include "LightData.hlsl.h"
#include "ALIGN16.h"
#include "DX11_Core.h"
#include "GameConsoleCommand.h"

class Light : public ALIGN16<Light>
{
public:

	typedef unsigned int			LightID;				// Custom type for light ID
	static LightID					GlobalLightIDCount;		// Monotonically-increasing ID counter for light objects

	// Lighting data
	LightData						Data;

	// Unique ID for the light
	CMPINLINE LightID				GetID(void) const			{ return m_id; }

	// Active / inactive status of the light
	CMPINLINE bool					IsActive(void) const		{ return Data.Enabled; }
	CMPINLINE void					Activate(void) 				{ Data.Enabled = true; }
	CMPINLINE void					Deactivate(void)			{ Data.Enabled = false; }
	CMPINLINE void					SetIsActive(bool active)	{ Data.Enabled = active; }
	CMPINLINE void					Toggle(void)				{ SetIsActive(!IsActive()); }

	// Default constructor
	Light(void);

	// Constructor taking predefined light data
	Light(const LightData & data);

	// Default copy constructor
	Light(const Light & source);


	// Initialise a light to the specified type
	void							InitialiseDirectionalLight(const XMFLOAT4 & directionWS, const XMFLOAT4 & colour, float intensity);
	void							InitialisePointLight(const XMFLOAT4 & positionWS, const XMFLOAT4 & colour, float range, float intensity);
	void							InitialiseSpotLight(const XMFLOAT4 & positionWS, const XMFLOAT4 & directionWS, const XMFLOAT4 & colour,
														float range, float intensity, float spotlight_angle);

	// Translate a light type value to/from its string representation
	static std::string				TranslateLightTypeToString(LightType type);
	static Light::LightType			TranslateLightTypeFromString(std::string type);

	// Process a debug command from the console.  "Light" objects are not part of the object hierarchy, but 
	// members of that hierarchy will invokve this method when asked to perform lighting-related actions
	void							ProcessDebugCommand(GameConsoleCommand & command);

	// Accessor functions which operate on the underlying LightData structure
	CMPINLINE LightType GetType(void) const						{ return Data.Type; }
	CMPINLINE XMFLOAT4 GetPositionWS(void) const				{ return Data.PositionWS; }
	CMPINLINE XMFLOAT4 GetDirectionWS(void) const				{ return Data.DirectionWS; }
	CMPINLINE XMFLOAT4 GetPositionVS(void) const				{ return Data.PositionVS; }
	CMPINLINE XMFLOAT4 GetDirectionVS(void) const				{ return Data.DirectionVS; }
	CMPINLINE XMFLOAT4 GetColour(void) const					{ return Data.Colour; }
	CMPINLINE float GetRange(void) const						{ return Data.Range; }
	CMPINLINE float GetIntensity(void) const					{ return Data.Intensity; }
	CMPINLINE float GetSpotlightAngle(void) const				{ return Data.SpotlightAngle; }

	// Mutator functions which operate on the underlying LightData structure
	CMPINLINE void SetType(LightType value) { Data.Type = value; }
	CMPINLINE void SetPositionWS(const XMFLOAT4 & value) { Data.PositionWS = value; }
	CMPINLINE void SetDirectionWS(const XMFLOAT4 & value) { Data.DirectionWS = value; }
	CMPINLINE void SetPositionVS(const XMFLOAT4 & value) { Data.PositionVS = value; }
	CMPINLINE void SetDirectionVS(const XMFLOAT4 & value) { Data.DirectionVS = value; }
	CMPINLINE void SetColour(const XMFLOAT4 & value) { Data.Colour = value; }
	CMPINLINE void SetRange(float value) { Data.Range = value; }
	CMPINLINE void SetIntensity(float value) { Data.Intensity = value; }
	CMPINLINE void ChangeIntensity(float delta) { SetIntensity(GetIntensity() + delta); }
	CMPINLINE void SetSpotlightAngle(float value) { Data.SpotlightAngle = value; }
	
	// Default destructor
	~Light(void);

protected:

	// Unique ID of the light
	LightID							m_id;

	// Method to return a new unique ID
	CMPINLINE void					AssignNewUniqueID(void) { m_id = ++GlobalLightIDCount; }

};




#endif