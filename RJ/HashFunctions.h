#pragma once

#ifndef __HashFunctionsH__
#define __HashFunctionsH__

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include "CompilerSettings.h"
#include "Utility.h"

// Define hash type - for now, we will use unsigned 32-bit hash values
typedef uint32_t HashVal;

// Define the seed value to be used for generating standard (non-cryptographic) hashes
extern const HashVal HashStandardSeed;

// Macro used to define a standard hash value
#define DefineHash(Hash, Str) const PreHashedStringType Hash = PreHashedStringType(Str);

// Hash function to generate a hash from a given string input.  32-bit.  More advanced hash functions, and functions able to 
// handle much larger input volumes, at http://code.google.com/p/smhasher/wiki/MurmurHash3
extern HashVal ExecuteHash(const char *key, HashVal len, HashVal seed);

// Convenience methods to hash the given values
CMPINLINE HashVal HashString(const std::string & str) { return ExecuteHash(str.c_str(), (HashVal)str.size(), HashStandardSeed); }
CMPINLINE HashVal HashString(const char *str) { return ExecuteHash(str, (HashVal)strlen(str), HashStandardSeed); }
CMPINLINE HashVal HashString(const char *str, HashVal len) { return ExecuteHash(str, len, HashStandardSeed); }

// Structure used to store pre-hashed text strings
struct PreHashedStringType
{
public:
	const std::string				Text;
	const HashVal					Hash;

	// Constructor, to store the text string and hash it
	PreHashedStringType(const std::string & text) : Text(text), Hash((Text == NullString ? 0U : HashString(text))) { }

	// Define custom equality operators; automatically compare against the hash value if comparator is a hash, or another pre-hashed string
	CMPINLINE friend bool operator== (const PreHashedStringType &a1, const PreHashedStringType &a2) { return (a1.Hash == a2.Hash); }
	CMPINLINE friend bool operator!= (const PreHashedStringType &a1, const PreHashedStringType &a2) { return (a1.Hash != a2.Hash); }
	CMPINLINE friend bool operator== (const PreHashedStringType &a1, const HashVal &a2) { return (a1.Hash == a2); }
	CMPINLINE friend bool operator!= (const PreHashedStringType &a1, const HashVal &a2) { return (a1.Hash == a2); }
	CMPINLINE friend bool operator== (const HashVal &a1, const PreHashedStringType &a2) { return (a1 == a2.Hash); }
	CMPINLINE friend bool operator!= (const HashVal &a1, const PreHashedStringType &a2) { return (a1 != a2.Hash); }

	// Automatically compare string values if the comparator is a cstr or std::string
	CMPINLINE friend bool operator== (const PreHashedStringType &a1, const char *a2) { return (a1.Text == a2); }
	CMPINLINE friend bool operator== (const PreHashedStringType &a1, const std::string &a2) { return (a1.Text == a2); }
	CMPINLINE friend bool operator!= (const PreHashedStringType &a1, const char *a2) { return (a1.Text != a2); }
	CMPINLINE friend bool operator!= (const PreHashedStringType &a1, const std::string &a2) { return (a1.Text != a2); }
	CMPINLINE friend bool operator== (const char *a1, const PreHashedStringType &a2) { return (a1 == a2.Text); }
	CMPINLINE friend bool operator== (const std::string &a1, const PreHashedStringType &a2) { return (a1 == a2.Text); }
	CMPINLINE friend bool operator!= (const char *a1, const PreHashedStringType &a2) { return (a1 != a2.Text); }
	CMPINLINE friend bool operator!= (const std::string &a1, const PreHashedStringType &a2) { return (a1 != a2.Text); }
};

// Set of pre-hashed text strings in common use throughout the application
namespace HashedStrings
{
	DefineHash(H_SimpleShip, "simpleship");
	DefineHash(H_ComplexShip, "complexship");
	DefineHash(H_ComplexShipSection, "complexshipsection");
	DefineHash(H_ComplexShipSectionInstance, "complexshipsectioninstance");
	DefineHash(H_ComplexShipElement, "complexshipelement");
	DefineHash(H_ComplexShipTile, "complexshiptile");
	DefineHash(H_ComplexShipTileDefinition, "complexshiptiledefinition");
	DefineHash(H_ComplexShipTileClass, "complexshiptileclass");
	DefineHash(H_StaticTerrain, "staticterrain");

	DefineHash(H_Code, "code"); 
	DefineHash(H_Name, "name");
	DefineHash(H_Model, "model");
	DefineHash(H_Type, "type");
	DefineHash(H_CameraPosition, "cameraposition");
	DefineHash(H_CameraRotation, "camerarotation");
	DefineHash(H_CameraElasticity, "cameraelasticity");
	DefineHash(H_Mass, "mass");
	DefineHash(H_VelocityLimit, "velocitylimit");
	DefineHash(H_AngularVelocityLimit, "angularvelocitylimit");
	DefineHash(H_BrakeFactor, "brakefactor");
	DefineHash(H_TurnRate, "turnrate");
	DefineHash(H_TurnAngle, "turnangle");
	DefineHash(H_EngineAngularAcceleration, "engineangularacceleration");
	DefineHash(H_BankExtent, "bankextent");
	DefineHash(H_BankRate, "bankrate");
	DefineHash(H_PreviewImage, "previewimage");
	DefineHash(H_Hardpoint, "hardpoint");
	DefineHash(H_Position, "position");
	DefineHash(H_Orientation, "orientation");
	DefineHash(H_Size, "size");
	DefineHash(H_DefaultLoadout, "defaultloadout");
	DefineHash(H_Ship, "ship");
	DefineHash(H_Class, "class");
	DefineHash(H_Map, "map");
	DefineHash(H_CompoundMap, "compoundmap");
	DefineHash(H_Option, "option");
	DefineHash(H_Filename, "filename");
	DefineHash(H_Texture, "texture");
	DefineHash(H_NoModelCentering, "nomodelcentering");
	DefineHash(H_EffectiveSize, "effectivesize");
	DefineHash(H_ActualEffectiveSize, "actualeffectivesize");
	DefineHash(H_Location, "location");
	DefineHash(H_ElementLocation, "elementlocation");
	DefineHash(H_ElementSize, "elementsize");
	DefineHash(H_Include, "include");
	DefineHash(H_File, "file");
	DefineHash(H_PrimaryTile, "primarytile");
	DefineHash(H_StandardTile, "standardtile");
	DefineHash(H_MinimumSize, "minimumsize");
	DefineHash(H_MaximumSize, "maximumsize");
	DefineHash(H_InfrastructureRequirement, "infrastructurerequirement");
	DefineHash(H_ObjectRequirement, "objectrequirement");
	DefineHash(H_Level, "level");
	DefineHash(H_SDOffset, "sdoffset");
	DefineHash(H_Rotation, "rotation");
	DefineHash(H_Image, "image");
	DefineHash(H_Buildable, "buildable");
	DefineHash(H_AttachPoint, "attachpoint");
	DefineHash(H_Properties, "properties");
	DefineHash(H_DefaultProperty, "defaultproperty");
	DefineHash(H_DefaultProperties, "defaultproperties");
	DefineHash(H_ClassSpecificDefinition, "classspecificdefinition");
	DefineHash(H_CompoundTileModelData, "compoundtilemodeldata");
	DefineHash(H_ProductionCost, "productioncost");
	DefineHash(H_TimeRequirement, "timerequirement");
	DefineHash(H_ResourceRequirement, "resourcerequirement");
	DefineHash(H_ShipTileRequirement, "shiptilerequirement");
	DefineHash(H_ConstructedBy, "constructedby");
	DefineHash(H_EffectModel, "effectmodel");
	DefineHash(H_FireTexture, "firetexture");
	DefineHash(H_NoiseTexture, "noisetexture");
	DefineHash(H_AlphaTexture, "alphatexture");
	DefineHash(H_NoiseScrollSpeed, "noisescrollspeed");
	DefineHash(H_NoiseScaling, "noisescaling");
	DefineHash(H_NoiseDistortion1, "noisedistortion1");
	DefineHash(H_NoiseDistortion2, "noisedistortion2");
	DefineHash(H_NoiseDistortion3, "noisedistortion3");
	DefineHash(H_NoiseDistortionScale, "noisedistortionscale");
	DefineHash(H_NoiseDistortionBias, "noisedistortionbias");
	DefineHash(H_Health, "health"); 
	DefineHash(H_MaxHealth, "maxhealth");
	DefineHash(H_IsInvulnerable, "isinvulnerable");
	DefineHash(H_MaxThrust, "maxthrust");
	DefineHash(H_MinThrust, "minthrust");
	DefineHash(H_Acceleration, "acceleration");
	DefineHash(H_EmitterClass, "emitterclass");
	DefineHash(H_ParticleLimit, "particlelimit");
	DefineHash(H_ParticleTexture, "particletexture");
	DefineHash(H_EmissionFrequency, "emissionfrequency");
	DefineHash(H_InitialLifetime, "initiallifetime");
	DefineHash(H_InitialPosition_min, "initialposition.min");
	DefineHash(H_InitialPosition_max, "initialposition.max");
	DefineHash(H_InitialColour_min, "initialcolour.min");
	DefineHash(H_InitialColour_max, "initialcolour.max");
	DefineHash(H_InitialSize, "initialsize");
	DefineHash(H_InitialVelocity_min, "initialvelocity.min");
	DefineHash(H_InitialVelocity_max, "initialvelocity.max");
	DefineHash(H_Attribute, "attribute");
	DefineHash(H_AttributeRange, "attributerange");
	DefineHash(H_AttributeEffect, "attributeeffect");
	DefineHash(H_ViewOffset, "viewoffset");
	DefineHash(H_DefaultAnimation, "defaultanimation");
	DefineHash(H_ScaleFactor, "scalefactor");
	DefineHash(H_Value, "value");
	DefineHash(H_AsteroidResource, "asteroidresource");
	DefineHash(H_PlanetResource, "planetresource");
	DefineHash(H_SpaceBackdrop, "spacebackdrop");
	DefineHash(H_Description, "description");
	DefineHash(H_Constant, "constant");
	DefineHash(H_Image2D, "image2d");
	DefineHash(H_Image2DGroup, "image2dgroup");
	DefineHash(H_Instance, "instance");
	DefineHash(H_Button, "button");
	DefineHash(H_TextBlock, "textblock");
	DefineHash(H_TextBox, "textbox");
	DefineHash(H_ComboBox, "combobox");
	DefineHash(H_ComponentGroup, "componentgroup");
	DefineHash(H_Component, "component");
	DefineHash(H_SimulationState, "simulationstate");
	DefineHash(H_Active, "active");
	DefineHash(H_Visible, "visible");
	DefineHash(H_StandardObject, "standardobject");
	DefineHash(H_VisibilityTestingMode, "visibilitytestingmode");
	DefineHash(H_NavNodePositionCount, "navnodepositioncount");
	DefineHash(H_NavNodeConnectionCount, "navnodeconnectioncount");
	DefineHash(H_NavNodePosition, "navnodeposition");
	DefineHash(H_NavNodeConnection, "navnodeconnection");
	DefineHash(H_InterchangeableXYSize, "interchangeablexysize");
	DefineHash(H_Gravity, "gravity");
	DefineHash(H_OxygenLevel, "oxygenlevel");
	DefineHash(H_OxygenRange, "oxygenrange");
	DefineHash(H_InitialGravity, "initialgravity");
	DefineHash(H_InitialOxygenLevel, "initialoxygenlevel");
	DefineHash(H_InitialOxygenRange, "initialoxygenrange");
	DefineHash(H_GravityRange, "gravityrange");
	DefineHash(H_GravityFalloffDelay, "gravityfalloffdelay");
	DefineHash(H_GravityExponent, "gravityexponent");
	DefineHash(H_CollisionMode, "collisionmode");
	DefineHash(H_CollisionOBB, "collisionobb");
	DefineHash(H_Px, "px");
	DefineHash(H_Py, "py");
	DefineHash(H_Pz, "pz");
	DefineHash(H_Ox, "ox");
	DefineHash(H_Oy, "oy");
	DefineHash(H_Oz, "oz");
	DefineHash(H_Ow, "ow");
	DefineHash(H_Sx, "sx");
	DefineHash(H_Sy, "sy");
	DefineHash(H_Sz, "sz");
	DefineHash(H_Ex, "ex");
	DefineHash(H_Ey, "ey");
	DefineHash(H_Ez, "ez");
	DefineHash(H_NumChildren, "numchildren");
	DefineHash(H_Skip, "skip");
	DefineHash(H_Attachment, "attachment");
	DefineHash(H_ArticulatedModel, "articulatedmodel");
	DefineHash(H_ConstraintTag, "constrainttag");
	DefineHash(H_ComponentTag, "componenttag");
	DefineHash(H_Yaw, "yaw");
	DefineHash(H_Pitch, "pitch");
	DefineHash(H_Range, "range");
	DefineHash(H_ProjectileLauncher, "projectilelauncher");
	DefineHash(H_Projectile, "projectile");
	DefineHash(H_Spread, "spread");
	DefineHash(H_Launch, "launch");
	DefineHash(H_LaunchInterval, "launchinterval");
	DefineHash(H_LaunchAngularVelocity, "launchangularvelocity");
	DefineHash(H_DegradeLinearVelocity, "degradelinearvelocity");
	DefineHash(H_DegradeAngularVelocity, "degradeangularvelocity");
	DefineHash(H_AddOrientationDrift, "addorientationdrift");
	DefineHash(H_ProjectileType, "projectiletype");
	DefineHash(H_DefaultLifetime, "defaultlifetime");
	DefineHash(H_LifetimeEndAction, "lifetimeendaction");
	DefineHash(H_CreateLaunchers, "createlaunchers");
	DefineHash(H_RelativePosition, "relativeposition");
	DefineHash(H_RelativeOrientation, "relativeorientation");
	DefineHash(H_TurretFireDelay, "turretfiredelay");
	DefineHash(H_Speed, "speed");
	DefineHash(H_ProjectileBeamLength, "projectilebeamlength");
	DefineHash(H_ProjectileBeamRadius, "projectilebeamradius");
	DefineHash(H_Lifetime, "lifetime");
	DefineHash(H_Colour, "colour");
	DefineHash(H_Connections, "connections");
	DefineHash(H_CanConnect, "canconnect");
	DefineHash(H_Connection, "connection");
	DefineHash(H_DynamicTileSet, "dynamictileset");
	DefineHash(H_Damage, "damage");
	DefineHash(H_DamageSet, "damageset");
	DefineHash(H_DamageResistance, "damageresistance");
	DefineHash(H_DamageResistanceSet, "damageresistanceset");
	DefineHash(H_ElementStateDefinition, "elementstatedefinition");
	DefineHash(H_State, "state");
	DefineHash(H_DefaultState, "defaultstate");
}


#endif
