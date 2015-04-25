#include "DX11_Core.h"

#include "Utility.h"
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
	Parameters.Float3_1 = Parameters.Float3_2 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	Parameters.Int3_1 = Parameters.Int3_2 = INTVECTOR3(0, 0, 0);
	Parameters.Flag_1 = Parameters.Flag_2 = Parameters.Flag_3 = false;
	Parameters.Target_1 = Parameters.Target_2 = NULL;
}


Order::~Order(void)
{
}
