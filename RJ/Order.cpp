#include "DX11_Core.h"

#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "Order.h"

// Default constructor
Order::Order(void) :
	m_ordertype(Order::OrderType::Unknown),						// Order type will be initialised by subclass constructor
	ID(0),														// Unique ID will be set on assignment to an entity (ID is unique per entity)
	Active(true),
	Parent(0),
	Dependency(0),
	Source(Order::OrderSource::Entity),
	EvaluationFrequency(Game::C_DEFAULT_ORDER_EVAL_FREQUENCY),
	TimeSinceLastEvaluation(99999U)								// Orders begin ready to execute
{
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









