#include "RenderDeviceDX11.h"
#include "GameVarsExtern.h"
#include "Logging.h"
#include "Utility.h"

// We will negotiate the highest possible supported feature level when attempting to initialise the render device
const D3D_FEATURE_LEVEL RenderDeviceDX11::SUPPORTED_FEATURE_LEVELS[] = 
{ 
	D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 
};


RenderDeviceDX11::RenderDeviceDX11(void)
	:
	m_device(NULL), 
	m_devicecontext(NULL),
	m_drivertype(D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN), 
	m_debuglayer(NULL), 
	m_devicename(NullString), 
	m_devicememory(0U)
{
	SetRenderDeviceName("RenderDeviceDX11 (Direct3D 11.2)");
}

Result RenderDeviceDX11::Initialise(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_depth)
{
	Game::Log << "Initialising render device \"" << GetRenderDeviceName() << "\"\n";

	// Device flags for initialisation; enable debug layer if required & only in debug builds
	UINT deviceflags = 0U;
#	ifdef _DEBUG
	if (Game::C_RENDER_DEBUG_LAYER)
	{
		Game::Log << LOG_INFO << "Enabling D3D11 debug layer\n";
		deviceflags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#	endif

	// Base device & context which we then try to upgrade to later DX versions
	ID3D11Device * base_device;
	ID3D11DeviceContext * base_context;
	D3D_FEATURE_LEVEL featurelevel;

	// Retain support for WARP although performance will likely be prohibitively low
	m_drivertype = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE;
	if (Game::ForceWARPRenderDevice)
	{
		Game::Log << LOG_INFO << "Reverting to WARP render drivers\n";
		m_drivertype = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_WARP;
	}
	

	// Attempt to initialise the base device and context
	UINT feature_count = sizeof(SUPPORTED_FEATURE_LEVELS) / sizeof(D3D_FEATURE_LEVEL);
	HRESULT hr = D3D11CreateDevice(nullptr, m_drivertype, nullptr, deviceflags,
		&(SUPPORTED_FEATURE_LEVELS[0]), feature_count,
		D3D11_SDK_VERSION, &base_device, &featurelevel, &base_context);

	// Some older devices do not recognise later feature levels at all.  If this is the case, attempt 
	// to back up to the minimum required feature set and retry the device creation
	if (hr == E_INVALIDARG)
	{
		Game::Log << LOG_WARN << "Render device reports unrecognised feature levels up to 11.2\n";
		
		UINT min_feature_req = 1U;
		Game::Log << LOG_INFO << "Attempting reinitialisation of render device with reduced feature set (-" << min_feature_req <<
			" level" << (min_feature_req == 1U ? "" : "s") << ")\n";

		hr = D3D11CreateDevice(nullptr, m_drivertype, nullptr, deviceflags,
			&(SUPPORTED_FEATURE_LEVELS[min_feature_req]), (feature_count - min_feature_req),
			D3D11_SDK_VERSION, &base_device, &featurelevel, &base_context);
	}

	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Fatal error: failed to initialise D3D11 render device (hr:" << hr << ")\n";
		return ErrorCodes::CannotCreateDirect3DDevice;
	}

	Game::Log << LOG_INFO << "D3D11 render device and context initialised successfully\n";

	// Attempt to query for later DX version (D3D 11.x) support
	Game::Log << LOG_INFO << "Attempting to query for device version \"" << Rendering::GetRenderDeviceTypeName() << "\" support\n";
	hr = base_device->QueryInterface<Rendering::RenderDeviceType>(&m_device);
	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Fatal error: failed to initialise \"" << Rendering::GetRenderDeviceTypeName() << "\" render device version\n";
		return ErrorCodes::CannotCreateDirect3DFinalVersion;
	}

	// We have all required DX device support, so retrieve the active context and report success
	Game::Log << LOG_INFO << "Successfully initialised final render device version \"" << Rendering::GetRenderDeviceTypeName() << "\"\n";
	m_device->GetImmediateContext2(&m_devicecontext);
	Game::Log << LOG_INFO << "Successfully initialised final render context version \"" << Rendering::GetRenderDeviceContextTypeName() << "\"\n";
	
	// Initialise D3D debug layer support if required & available (also based upon device flags above)
	if (SUCCEEDED(m_device->QueryInterface<ID3D11Debug>(&m_debuglayer)))
	{
		Game::Log << LOG_INFO << "Initialising D3D11 debug layer components\n";

		ID3D11InfoQueue *debuginfoqueue = NULL;
		if (SUCCEEDED(m_debuglayer->QueryInterface<ID3D11InfoQueue>(&debuginfoqueue)))
		{
#			ifdef _DEBUG
			debuginfoqueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			debuginfoqueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			debuginfoqueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
#			endif

			D3D11_MESSAGE_ID suppress_messages[] = 
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS
			};
			UINT suppress_count = sizeof(suppress_messages) / sizeof(D3D11_MESSAGE_ID);

			D3D11_INFO_QUEUE_FILTER filter;
			memset((void *)&filter, 0, sizeof(D3D11_INFO_QUEUE_FILTER));
			filter.DenyList.NumIDs = suppress_count;
			filter.DenyList.pIDList = suppress_messages;
			debuginfoqueue->AddStorageFilterEntries(&filter);

			ReleaseIfExists(debuginfoqueue);
		}
		else
		{
			Game::Log << LOG_WARN << "Could not initialise D3D debug info queue; debug layer may not be fully-enabled\n";
		}
	}
	else
	{
		Game::Log << LOG_INFO << "No D3D debug support, debug layer will not be initialised\n";
	}

	// We can now determine the primary adapter output & capabilities, then initialise it for rendering
	Result res = InitialisePrimaryGraphicsAdapter(screen_size, vsync);
	if (res != ErrorCodes::NoError)
	{
		Game::Log << LOG_WARN << "Error encountered [" << res << "] during initialisation of primary graphics adapter\n";
		return res;
	}
		
	Game::Log << LOG_INFO << "Initialisation of primary render device completed successfully\n";
	return ErrorCodes::NoError;
}

Result RenderDeviceDX11::InitialisePrimaryGraphicsAdapter(INTVECTOR2 screen_size, bool vsync)
{
	// Create a DirectX graphics interface factory.
	IDXGIFactory* factory = NULL;
	HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Could not create DX11 interface factory (hr:" << result << ")\n";
		return ErrorCodes::CouldNotCreateDXInterfaceFactory;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	IDXGIAdapter* adapter = NULL;
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Could not create primary graphics adapter interface (hr:" << result << ")\n";
		return ErrorCodes::CouldNotCreatePrimaryAdapterInterface;
	}

	DXGI_ADAPTER_DESC adapterDesc;
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Could not retrieve primary graphics adapter descriptor (hr:" << result << ")\n";
		return ErrorCodes::CouldNotDetermineAdapterDescription;
	}

	// Convert the name of the video card to a character array and store it.
	if (adapterDesc.Description == NULL)
	{
		Game::Log << LOG_WARN << "Primary adapter has no descriptor name; render device initialisation may fail\n";
	}
	else
	{
		m_devicename = ConvertWStringToString(adapterDesc.Description);
		if (m_devicename.empty())
		{
			Game::Log << LOG_WARN << "Primary adapter has empty descriptor name; render device initialisation may fail\n";
		}
		else
		{
			Game::Log << LOG_INFO << "Identified primary video adapter: \"" << m_devicename << "\"\n";
		}
	}

	// Store the dedicated video card memory in megabytes.
	m_devicememory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	Game::Log << LOG_INFO << "Located " << m_devicememory << "MB dedicated memory on primary video adapter\n";
	

	// Enumerate the primary adapter output (monitor).
	IDXGIOutput* adapterOutput = NULL;
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Could not enumerate primary adapter outputs (hr:" << result << ")\n";
		return ErrorCodes::CouldNotEnumeratePrimaryAdapterOutputs;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	UINT numModes = 0U;
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Could not determine adapter display mode count (hr:" << result << ")\n";
		return ErrorCodes::CouldNotDetermineAdapterDisplayModeCount;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	DXGI_MODE_DESC *displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		Game::Log << LOG_ERROR << "Could not allocate adapter display mode storage (mc:" << numModes << ")\n";
		return ErrorCodes::CouldNotAllocateAdapterDisplayModeStorage;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Could not enumerate adapter display modes (hr:" << result << ")\n";
		return ErrorCodes::CouldNotEnumerateAdapterDisplayModes;
	}

	// Make sure we were able to locate at least one display mode
	if (numModes == 0U)
	{
		Game::Log << LOG_ERROR << "Error: no display modes available for primary adapter\n";
		return ErrorCodes::FoundNoDisplayModesAvailableForPrimaryAdapter;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	std::string modestr = ""; int modeindex = -1;
	unsigned int numerator = 0U, denominator = 0U;
	for (UINT i = 0; i < numModes; ++i)
	{
		// Output all the available display modes
		modestr = concat(modestr)(i == 0 ? "" : ", ")(RenderDevice::DXDisplayModeToString(displayModeList[i])).str();

		// Use the display mode if it matches our desired resolution
		if (displayModeList[i].Width == (unsigned int)screen_size.x)
		{
			if (displayModeList[i].Height == (unsigned int)screen_size.y)
			{
				// Additional check: refresh rate must match if the user has set it specifically
				if (Game::ScreenRefresh > 0)
				{
					if (displayModeList[i].RefreshRate.Denominator == 0 ||
						((displayModeList[i].RefreshRate.Numerator /
							displayModeList[i].RefreshRate.Denominator) != Game::ScreenRefresh)) continue;
				}

				// This display mode will work for us, so store it
				modeindex = (int)i;
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Output the available display modes to the log
	Game::Log << LOG_INFO << numModes << " display modes found for primary adapter (" << modestr << ")\n";

	// Make sure we found at least one matching display mode
	if (modeindex == -1 || (numerator == 0 && denominator == 0))
	{
		if (!vsync)
		{
			Game::Log << LOG_WARN << "Warning: No matching display mode located for [" << screen_size.x << "x"
				<< screen_size.y << "]; attempting to proceed since vsync is disabled\n";
		}
		else
		{
			Game::Log << LOG_ERROR << "Error: No matching display mode located for [" << screen_size.x << "x"
				<< screen_size.y << "]; cannot proceed since vsync is required\n";
			return ErrorCodes::FoundNoSupportedDisplayModeForResolution;
		}
	}
	else
	{
		Game::Log << LOG_INFO << "Enabling display mode of "
			<< RenderDevice::DXDisplayModeToString(displayModeList[modeindex]) << " on primary adapter\n";
	}

	// Release all relevant COM objects
	SafeDeleteArray(displayModeList);
	ReleaseIfExists(adapterOutput);
	ReleaseIfExists(adapter);
	ReleaseIfExists(factory);
}



RenderDeviceDX11::~RenderDeviceDX11(void)
{

}









