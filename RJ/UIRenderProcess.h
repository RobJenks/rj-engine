#pragma once

#include "RenderProcess.h"


class UIRenderProcess : public RenderProcess
{
public:

	// Constructor
	UIRenderProcess(void);

	// Virtual render method; must be implemented by all derived render processess
	void							Render(void);

	// Perform any initialisation that cannot be completed on construction, e.g. because it requires
	// data that is read in from disk during the data load process
	void							PerformPostDataLoadInitialisation(void);


private:


};