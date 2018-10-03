#pragma once

#include <string>
#include <array>
#include <unordered_map>
#include <memory>
#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "RenderProcess.h"
#include "Utility.h"

class RenderDevice
{
public:

	// Constructor
	RenderDevice(void);

	CMPINLINE std::string		GetRenderDeviceName(void) const { return m_renderdevice_name; }

	// Set the active render process
	template <class TRenderProcess> void ActivateRenderProcess(RenderProcess::RenderProcessClass process_type);

	// Perform rendering; will delegate to the currently-active render process
	void						Render(void);
	
	// Perform any late initialisation that requires access to loaded game data
	void						PerformPostDataLoadInitialisation(void);


	static std::string			DXDisplayModeToString(const DXGI_MODE_DESC & mode);

	~RenderDevice(void);

protected:

	void						ExecuteRenderProcess(RenderProcess::RenderProcessClass process_type);

	CMPINLINE void				SetRenderDeviceName(const std::string & name) { m_renderdevice_name = name; }

	template <class TRenderProcess>
	RenderProcess *				ActivateInternalRenderProcess(RenderProcess::RenderProcessClass process_type);

	CMPINLINE RenderProcess *	GetActiveRenderProcess(RenderProcess::RenderProcessClass process_type) 
	{ 
		assert(static_cast<size_t>(process_type) < static_cast<size_t>(RenderProcess::RenderProcessClass::_COUNT));
		return m_active_render_processes[static_cast<size_t>(process_type)];
	}

protected:

	// String identifier for the render device implementation currently in use
	std::string					m_renderdevice_name;

	// Currently-active render processes
	std::array<RenderProcess*, static_cast<size_t>(RenderProcess::RenderProcessClass::_COUNT)> m_active_render_processes;

	// Collection of all render processes that have been activated at some point; can be reactivated during 
	// normal rendering, and will all be deallocated automatically on shutdown
	typedef std::unordered_map<std::string, std::unique_ptr<RenderProcess>> RenderProcessCollection;
	RenderProcessCollection		m_render_processes;
};



// Set the active primary render process
// Note: the header file for TRenderProcess must be included in the code file where ActivateRenderProcess<TRenderProcess>(...) is called
template <class TRenderProcess>
void RenderDevice::ActivateRenderProcess(RenderProcess::RenderProcessClass process_type)
{
	assert(static_cast<size_t>(process_type) < static_cast<size_t>(RenderProcess::RenderProcessClass::_COUNT));

	// Perform internal activation of this render process
	m_active_render_processes[static_cast<size_t>(process_type)] = ActivateInternalRenderProcess<TRenderProcess>(process_type);
}


// Perform internal activation of a render process
template <class TRenderProcess>
RenderProcess * RenderDevice::ActivateInternalRenderProcess(RenderProcess::RenderProcessClass process_type)
{
	const std::string name = RenderProcess::Name<TRenderProcess>();
	Game::Log << LOG_INFO << "Attempting to enable render process \"" << name << "\"\n";

	// Check whether we already have this process available; if so, reactivate it
	auto it = m_render_processes.find(name);
	if (it != m_render_processes.end())
	{
		Game::Log << LOG_INFO << "Reactivating suspended render process \"" << name << "\"\n";
		return it->second.get();
	}

	// Process has not been activated before; create a new instance and activate it
	Game::Log << LOG_INFO << "Render process \"" << name << "\" has not yet been initialised; creating and activating now\n";
	m_render_processes[name] = std::make_unique<TRenderProcess>();

	// Run post-load initialisation if we are instantiating a new process after application startup 
	if (Game::GameDataLoaded) m_render_processes[name].get()->PerformPostDataLoadInitialisation();

	// Store the newly-active render process
	return m_render_processes[name].get();
}






