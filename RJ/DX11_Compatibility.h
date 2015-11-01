#include <Windows.h>
#include <DirectXMath.h>

bool PlatformSupportsSSEInstructionSets(void)
{
	return DirectX::XMVerifyCPUSupport();
}