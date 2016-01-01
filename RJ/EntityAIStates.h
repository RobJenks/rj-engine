#pragma once

#ifndef __EntityAIStatesH__
#define __EntityAIStatesH__

class EntityAIStates
{
public:

	// Enumeration of entity AI states which determine the level of autonomy granted to the entity
	enum EntityAIState
	{
		Independent = 0,			// Entity has full control over its actions.  It will accept higher-level orders, but
									// may override them if desired.  Will pursue its own objectives when it has no other orders
		StrategicControl			// Entity will take orders from higher-level AI, and will not override them.  Entity
									// does not have any objectives of its own and will wait idle until given directions
	};

	// Enumeration of entity AI states which determine how it reacts to potential engagements in its area
	enum EntityEngagementState
	{
		EngageIfSensible = 0,		// Will move to engage nearby contacts if we stand a chance, otherwise flee
		AlwaysEngage,				// Will always engage nearby enemies, regardless of relative strength
		FleeFromEngagements,		// Will flee from all engagements, regardless of relative strength
		IgnoreAllEngagements		// Will ignore all enemy contacts and proceed with orders 
	};

};



#endif







