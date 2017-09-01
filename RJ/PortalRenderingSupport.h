#pragma once

// Enumeration identifying the level of support for portal-based rendering
enum PortalRenderingSupport
{
	DetermineAutomatically = 0,			// Use of portal-based rendering is at the discretion of the engine
	ForceEnabled,						// Portal-based rendering will be force-enabled
	ForceDisabled						// Portal-based rendering will be force-disabled
};

