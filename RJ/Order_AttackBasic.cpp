#include "iSpaceObject.h"
#include "Order_AttackBasic.h"

// Constructor including main order parameters
Order_AttackBasic::Order_AttackBasic(iSpaceObject *target)
{
	// Parameter check; if target is invalid, generate an order that will be rejected immediately
	if (!target) { Target = NULL; return; }

	// Store the target, the close distance sq (based on target size) and the distance sq to back off between
	// attack runs (based on close distance)
	Target = target;
	CloseDist = (Target->GetCollisionSphereRadius() * Game::C_DEFAULT_ATTACK_CLOSE_MULTIPLIER);
	CloseDistSq = (CloseDist * CloseDist);

	// Determine an appropriate retreat distance based on this
	RetreatDist = (CloseDist + Game::C_DEFAULT_ATTACK_RETREAT_DISTANCE);
	RetreatDistSq = (RetreatDist * RetreatDist);

}