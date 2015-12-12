#include "iSpaceObject.h"
#include "Ship.h"
#include "Order_AttackBasic.h"

// Constructor including main order parameters
Order_AttackBasic::Order_AttackBasic(Ship *attacker, iSpaceObject *target)
{
	// Parameter check; if target is invalid, generate an order that will be rejected immediately
	if (!attacker || !target) { Target = NULL; return; }

	// Store the target, the close distance sq (based on target size) and the distance sq to back off between
	// attack runs (based on close distance)
	Target = target;
	CloseDist = 
		(attacker->VelocityLimit.Value * Game::C_DEFAULT_ATTACK_CLOSE_TIME) + 
		(Target->GetCollisionSphereRadius() * Game::C_DEFAULT_ATTACK_CLOSE_RADIUS_MULTIPLIER);
	CloseDistSq = (CloseDist * CloseDist);
	CloseDistV = XMVectorReplicate(CloseDist);
	CloseDistSqV = XMVectorReplicate(CloseDistSq);

	// Determine an appropriate retreat distance based on this
	RetreatDist = (CloseDist + (attacker->VelocityLimit.Value * Game::C_DEFAULT_ATTACK_RETREAT_TIME));
	RetreatDistSq = (RetreatDist * RetreatDist);
	RetreatDistV = XMVectorReplicate(RetreatDist);
	RetreatDistSqV = XMVectorReplicate(RetreatDistSq);

}