#include "DX11_Core.h"

#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "Order.h"

// Default constructor
Order::Order(void)
{
	// Initialise default values
	this->ID = 0;														// Unique ID will be set on assignment to an entity (ID is unique per entity)
	this->Active = true;
	this->Parent = 0;
	this->Dependency = 0;
	this->EvaluationFrequency = Game::C_DEFAULT_ORDER_EVAL_FREQUENCY;
	this->TimeSinceLastEvaluation = 99999.0f;							// Orders begin ready to execute

	// Initialise default parameter values
	/*Parameters.Vector_1 = Parameters.Vector_2 = NULL_VECTOR;
	Parameters.Float3_1 = Parameters.Float3_2 = NULL_FLOAT3;
	Parameters.Int3_1 = Parameters.Int3_2 = NULL_INTVECTOR3;
	Parameters.Flag_1 = Parameters.Flag_2 = Parameters.Flag_3 = false;
	Parameters.Target_1 = Parameters.Target_2 = NULL;*/
}


Order::~Order(void)
{
}

// Translates an order type to its string representation
std::string Order::TranslateOrderTypeToString(Order::OrderType order_type)
{
	switch (order_type)
	{
		case Order::OrderType::ActorMoveToPosition:				return "ActorMoveToPosition";
		case Order::OrderType::ActorMoveToTarget:				return "ActorMoveToTarget";
		case Order::OrderType::ActorTravelToPosition:			return "ActorTravelToPosition";
		case Order::OrderType::ActorTravelToTarget:				return "ActorTravelToTarget";
		case Order::OrderType::AttackBasic:						return "AttackBasic";
		case Order::OrderType::MoveAwayFromTarget:				return "MoveAwayFromTarget";
		case Order::OrderType::MoveToPosition:					return "MoveToPosition";
		case Order::OrderType::MoveToTarget:					return "MoveToTarget";
		default:												return "(Unknown)";
	}
}









