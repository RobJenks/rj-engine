#include "DefaultValues.h"
#include "iObject.h"
#include "Actor.h"
#include "CapitalShipPerimeterBeacon.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "LightSource.h"
#include "SpaceProjectile.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "SpaceEmitter.h"

#include "DebugInvocation.h"

std::string DebugInvocation::SubclassInvocations::Invoke_DebugString(const iObject *obj)			
{ 
	INVOKE_SUBCLASS_METHOD(obj, DebugString(), std::string); 
}


