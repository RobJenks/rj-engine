#include "DX11_Core.h"

#include <unordered_map>
#include "ErrorCodes.h"

#include "FastMath.h"
#include "Image2D.h"
#include "Render2DGroup.h"
#include "TextureShader.h"
#include "Render2DManager.h"
class GameInputDevice;


Render2DManager::Render2DManager(void)
{

}

Render2DManager::~Render2DManager(void)
{
}

Result RJ_XM_CALLCONV Render2DManager::Initialise(Rendering::RenderDeviceType * device, Rendering::RenderDeviceContextType * deviceContext, HWND hwnd,
								   int screenWidth, int screenHeight, const FXMMATRIX baseviewmatrix)
{
	// Store references to the key supplied parameters
	m_device = device;
	m_devicecontext = deviceContext;
	m_hwnd = hwnd;
	m_screenwidth = screenWidth;
	m_screenheight = screenHeight;

	// Store the base matrices used for 2D rendering
	m_baseworldmatrix = ID_MATRIX;
	m_baseviewmatrix = baseviewmatrix;

	// Return success
	return ErrorCodes::NoError;
}

Render2DGroup *Render2DManager::CreateRenderGroup(std::string code)
{
	// Make sure a valid code has been provided
	if (code == NullString) return NULL;

	// Make sure this group doesn't already exist; if so, return a reference to it instead
	if (m_rendergroups.count(code) > 0) return m_rendergroups[code]; 

	// Instantiate a new render group, and set its render manager pointer back to us
	Render2DGroup *group = new Render2DGroup();
	group->SetRenderManager(this);

	// Add this group to the render manager collection
	m_rendergroups[code] = group;

	// All render groups start off inactive
	group->SetRenderActive(false);

	// Return a reference to the render group
	return group; 
}

void Render2DManager::ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse)
{
	Render2DGroup *group;

	// Look at each render group in turn
	RenderGroupCollection::const_iterator it_end = m_rendergroups.end();
	for (RenderGroupCollection::const_iterator it = m_rendergroups.begin(); it != it_end; ++it)
	{
		// Make sure this render group is active
		group = it->second;
		if (!group || !(group->GetRenderActive())) continue;

		// If it is, pass details through to the render group and have it trigger any relevant events
		group->ProcessUserEvents(keyboard, mouse);
	}
}

void Render2DManager::Render(void)
{
	Render2DGroup *group;

	// Look at each render group in turn
	RenderGroupCollection::const_iterator it_end = m_rendergroups.end();
	for (RenderGroupCollection::const_iterator it = m_rendergroups.begin(); it != it_end; ++it)
	{
		// Make sure this render group is set to be rendered
		group = it->second;
		if (!group || !(group->GetRenderActive())) continue;

		// Call the render function for this group
		group->Render();
	}
}

void Render2DManager::Shutdown(void)
{
	Render2DGroup *group;

	// Look at each render group in turn
	RenderGroupCollection::const_iterator it_end = m_rendergroups.end();
	for (RenderGroupCollection::const_iterator it = m_rendergroups.begin(); it != it_end; ++it)
	{
		// Make sure this render group exists
		group = it->second;
		if (!group) continue;

		// Have the render group shut itself down
		group->Shutdown();

		// Now dispose of the render group 
		delete group;
		group = NULL;
	}
}

void Render2DManager::ActivateGroup(std::string code)
{
	if (code != NullString && m_rendergroups.count(code) > 0 && m_rendergroups[code] != NULL)
	{
		// Enable rendering of this render group
		m_rendergroups[code]->SetRenderActive(true);
	}
	
}

void Render2DManager::DeactivateGroup(std::string code)
{
	if (code != NullString)
		if (m_rendergroups.count(code) > 0)
			if (m_rendergroups[code])
				m_rendergroups[code]->SetRenderActive(false);
}

void Render2DManager::DeactivateAllGroups(void)
{
	RenderGroupCollection::const_iterator it_end = m_rendergroups.end();
	for (RenderGroupCollection::const_iterator it = m_rendergroups.begin(); it != it_end; ++it)
	{
		if (it->second)
			it->second->SetRenderActive(false);
	}
}
