#pragma once

#ifndef __RenderQueueOptimiserH__
#define __RenderQueueOptimiserH__

#include "CompilerSettings.h"
#include "ScheduledObject.h"
#include "RenderQueue.h"

class RenderQueueOptimiser : public ScheduledObject
{
public:

	// Default constructor
	RenderQueueOptimiser(RM_RenderQueue & renderqueue);

	// Object does not perform any frequent updates
	void						Update(void)					{ }

	// Object will perform periodic maintenance on the render queue to e.g. prune model instance
	// collections that are no longer required
	void						UpdateInfrequent(void);

	// Indicates whether the render queue optimiser is ready to execute another cycle
	CMPINLINE bool				Ready(void) const				{ return m_ready; }

	// Runs the render queue optimiser
	void						Run(void);

	// Performs a pre-optimisation check of the render queue
	void						CheckRenderQueue(void);

	// Prunes any model data collections from the render queue if they have not been
	// used to render any instances since the last optimisation run
	void						OptimiseRenderQueue(void);

	// Sets the interval at which the render queue will be checked, prior to optimisation
	void						SetCheckInterval(unsigned int interval);

	// Sets the interval at which optimisation passes will be run over the render queue
	void						SetOptimisationInterval(unsigned int interval);

	// Default destructor
	~RenderQueueOptimiser(void);



protected:

	// Reference to the render queue being maintained
	RM_RenderQueue &						m_renderqueue;

	// Interval at which the render queue will be checked, prior to optimisation
	unsigned int							m_checkinterval;

	// Interval at which optimisation passes will be run over the render queue
	unsigned int							m_optimiseinterval;

	// Flag indicating that the optimiser is ready for another cycle
	bool									m_ready;

	// Clock time at which the render queue was last optimised
	unsigned int							m_lastoptimised;

	// The number of checks performed since the last optimisation.  Used to avoid any
	// optimisation runs when zero checks have been performed first
	int										m_checksperformed;

};




#endif