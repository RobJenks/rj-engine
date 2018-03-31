#include "UIRenderProcess.h"

// Constructor
UIRenderProcess::UIRenderProcess(void)
{
	m_name = RenderProcess::Name<UIRenderProcess>();
}

// Virtual render method; must be implemented by all derived render processess
void UIRenderProcess::Render(void)
{

}

// Perform any initialisation that cannot be completed on construction, e.g. because it requires
// data that is read in from disk during the data load process
void UIRenderProcess::PerformPostDataLoadInitialisation(void)
{

}
