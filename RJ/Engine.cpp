#include "Engine.h"


Engine::Engine(void) : Equipment()
{
	// Set default values
	this->Acceleration = this->BaseMaxThrust = this->MaxThrust = this->BaseMinThrust = this->MinThrust = 0.0f;
	this->EmitterClass = "";
}

Engine::~Engine(void)
{
}
