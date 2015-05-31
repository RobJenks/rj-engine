#include "GameVarsExtern.h"
#include "RenderQueue.h"
#include "CentralScheduler.h"

#include "RenderQueueOptimiser.h"

#include "Model.h"//DBG
// Default constructor
RenderQueueOptimiser::RenderQueueOptimiser(RM_RenderQueue & renderqueue)
	: 
	m_renderqueue(renderqueue)
{
	// Initialise fields to defaults
	m_ready = false;
	m_checksperformed = 0;

	// Set the test & optimisation interval
	SetCheckInterval(Game::C_DEFAULT_RENDERQUEUE_CHECK_INTERVAL);
	SetOptimisationInterval(Game::C_DEFAULT_RENDERQUEUE_OPTIMISE_INTERVAL);

	// Schedule the optimiser task
	Game::Scheduler.ScheduleInfrequentUpdates(this, m_checkinterval);
	m_lastoptimised = Game::ClockMs;
}

// Object will perform periodic maintenance on the render queue to e.g. prune model instance
// collections that are no longer required
void RenderQueueOptimiser::UpdateInfrequent(void)
{
	// Set the flag indicating the optimiser is ready to be run
	m_ready = true;
}

// Runs the render queue optimiser
void RenderQueueOptimiser::Run(void)
{
	// Check whether this is just a 'check' update, or if we want to perform optimisation this cycle
	if (Game::ClockMs - m_lastoptimised < m_optimiseinterval)
	{
		// We have not exceeded the optimisation interval; simply perform a check of the queue now
		CheckRenderQueue();
	}
	else
	{
		// Otherwise, wWe want to perform full optimisation.  Prune any model data collections 
		// from the render queue if they have not been used to render any instances since 
		// the last optimisation run
		OptimiseRenderQueue();
	}

	// Clear the 'ready' flag so that we wait for the next scheduled execution
	m_ready = false;
}

// Performs a pre-optimisation check of the render queue
void RenderQueueOptimiser::CheckRenderQueue(void)
{
	// Iterate through each shader, and each model queued for each shader
	RM_ModelInstanceData::iterator it, it_end;
	for (int i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		it_end = m_renderqueue[i].end();
		for (it = m_renderqueue[i].begin(); it != it_end; ++it)
		{
			// Test whether any instances are queued for this model at this point
			if (it->second.InstanceData.size() == 0)
			{
				// No instance queued; increase the timeout counter
				++it->second.TimeoutCounter;
			}
		}
	}

	// Increase the counter that tracks the number of checks performed before optimisation
	++m_checksperformed;
}

// Prunes any model data collections from the render queue if they have not been
// used to render any instances since the last optimisation run
void RenderQueueOptimiser::OptimiseRenderQueue(void)
{
	// Make sure we have had the chance to check a reasonable number of times,
	// to avoid any issues with delays (e.g. debug breakpoints, or on startup
	// initialisation) making us optimise the queue when it has not been checked first
	if (m_checksperformed == 0) { m_lastoptimised = Game::ClockMs; return; }

	// Iterate through each shader, and each model queued for each shader
	RM_ModelInstanceData::iterator it, it_end;
	for (int i = 0; i < RenderQueueShader::RM_RENDERQUEUESHADERCOUNT; ++i)
	{
		if (m_renderqueue[i].size() == 0) continue;

		it = m_renderqueue[i].begin(); it_end = m_renderqueue[i].end();
		while (it != it_end)
		{
			// We want to remove this instance collection if it has not been populated in 
			// any render cycle that we checked since the prior optimisation
			if (it->second.TimeoutCounter == m_checksperformed)
			{
				// Erase this entry from the queue, removing its key from the map.  Erase
				// method returns an iterator to the next element after the one which
				// was deleted, so we can continue iterating from here
				it = m_renderqueue[i].erase(it);
			}
			else
			{
				// Otherwise, reset the timeout counter and move to the next element
				it->second.TimeoutCounter = 0;
				++it;
			}
		}
	}

	// Reset the counter recording number of checks performed since the last optimisation
	m_checksperformed = 0;
	
	// Update the last-optimised time ready for the next cycle
	m_lastoptimised = Game::ClockMs;
}


// Sets the interval at which the render queue will be checked, prior to optimisation
void RenderQueueOptimiser::SetCheckInterval(unsigned int interval)
{
	m_checkinterval = max(interval, 10U);
}

// Sets the interval at which optimisation passes will be run over the render queue
void RenderQueueOptimiser::SetOptimisationInterval(unsigned int interval)
{
	m_optimiseinterval = max(interval, 100U);
}

// Default destructor
RenderQueueOptimiser::~RenderQueueOptimiser(void)
{
	// Remove our periodic task from the schedule
	Game::Scheduler.RemoveInfrequentUpdate(this);
}
