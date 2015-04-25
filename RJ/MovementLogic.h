#pragma once

#ifndef __MovementLogicH__
#define __MovementLogicH__

class Ship;

namespace Game { namespace Logic { 

	// Initialises all simulation data for a new cycle
	void BeginSimulationCycle(void);

	// Simulates the movement of all space objects in the universe
	void SimulateAllObjects(void);
	
	namespace Move
	{
		// Simulates movement of a single ship based on the supplied mouse data
		void UpdateShipMovementViaMouseFlightData(Ship *ship, float x_mv, float y_mv);
		void UpdatePlayerViewViaMouseData(float x_mv, float y_mv);
	};


}};

#endif