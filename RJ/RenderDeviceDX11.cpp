#include "RenderDeviceDX11.h"
#include "GameVarsExtern.h"
#include "Logging.h"
#include "Utility.h"
#include "ShaderDX11.h"
#include "InputLayoutDesc.h"
#include "SamplerStateDX11.h"


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
	m_devicememory(0U), 

	m_deferred_vs(NULL), 
	m_deferred_geometry_ps(NULL), 

	m_sampler_linearclamp(NULL), 
	m_sampler_linearrepeat(NULL)
{
	SetRenderDeviceName("RenderDeviceDX11 (Direct3D 11.2)");
}

Result RenderDeviceDX11::Initialise(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_depth)
{
	Game::Log << "Initialising rendering engine \"" << GetRenderDeviceName() << "\"\n";

	// Initialise the render device and context
	Result result = InitialiseRenderDevice(hwnd, screen_size, full_screen, vsync, screen_near, screen_depth);
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "Rendering engine startup failed [" << result << "] during initialisation of primary render device\n";
		return result;
	}

	// We can now determine the primary adapter output & capabilities, then initialise it for rendering
	result = InitialisePrimaryGraphicsAdapter(screen_size, vsync);
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "Rendering engine startup failed [" << result << "] during initialisation of primary graphics adapter\n";
		return result;
	}
		
	// Initialise input layout descriptors
	result = InitialiseInputLayoutDefinitions();
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "Rendering engine startup failed [" << result << "] during initialisation of input layout descriptors\n";
		return result;
	}

	// Load all shaders from external resources
	result = InitialiseShaderResources();
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "Rendering engine startup failed [" << result << "] during initialisation of shader resources\n";
		return result;
	}

	


	Game::Log << LOG_INFO << "Initialisation of rendering engine completed successfully\n";
	return ErrorCodes::NoError;
}

Result RenderDeviceDX11::InitialiseRenderDevice(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_depth)
{
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

Result RenderDeviceDX11::InitialiseInputLayoutDefinitions(void)
{
	Game::Log << LOG_INFO << "Loading standard shader input layouts\n";

	// Input layouts to be loaded
	std::vector<std::tuple<std::string, InputLayoutDesc &>> layout_definitions
	{
		{ "Vertex_Inst_Standard_Layout", m_standard_input_layout }
	};

	// Attempt to load each defintion in turn
	for (auto & layout : layout_definitions)
	{
		if (InputLayoutDesc::GetStandardLayout(std::get<0>(layout), std::get<1>(layout)) == false)
		{
			Game::Log << LOG_ERROR << "Cannot load standard input layout \"" << std::get<0>(layout) << "\"\n";
			return ErrorCodes::CouldNotCreateVertexShaderInputLayout;
		}
	}
	
	Game::Log << LOG_INFO << "All standard input layout definitions initialised\n";
	return ErrorCodes::NoError;
}

Result RenderDeviceDX11::InitialiseShaderResources(void)
{


	// Shader definitions
	std::vector<std::tuple<ShaderDX11**, Shader::Type, std::string, std::string, std::string, InputLayoutDesc*>> shader_resources
	{
		{ &m_deferred_vs, Shader::Type::VertexShader, "VS_Deferred", "Shaders\\deferred_vs_standard.vs.hlsl", "latest", &m_standard_input_layout }, 
		{ &m_deferred_geometry_ps, Shader::Type::PixelShader, "PS_Deferred_Geometry", "Shaders\\deferred_ps_geometry.ps.hlsl", "latest", NULL }
	};

	// Attempt to load each shader resource in turn
	Game::Log << LOG_INFO << "Loading all shader resources (" << shader_resources.size() << ")\n";
	for (auto & shader : shader_resources)
	{
		Result result = InitialiseExternalShaderResource(std::get<0>(shader), std::get<1>(shader), std::get<3>(shader), std::get<2>(shader), std::get<4>(shader), std::get<5>(shader));
		if (result != ErrorCodes::NoError)
		{
			Game::Log << LOG_ERROR << "Fatal error: shader initialisation failed, cannot recover from errors (res:" << result << ")\n";
			return result;
		}
	}
	
	Game::Log << LOG_INFO << "All shader resources initialised\n";	
}

Result RenderDeviceDX11::InitialiseExternalShaderResource(	ShaderDX11 ** ppOutShader, Shader::Type shadertype, const std::string & fileName, const std::string & entryPoint,
															const std::string & profile, const InputLayoutDesc *input_layout = NULL)
{
	Game::Log << LOG_INFO << "Initialising shader \"" << entryPoint << "\" from \"" << fileName << "\"\n";

	// No duplicates allowed
	if (m_shaders.find(entryPoint) != m_shaders.end())
	{
		Game::Log << LOG_ERROR << "Multiple shader resources detected with entry point \"" << entryPoint << "\"\n";
		return ErrorCodes::CannotLoadDuplicateShaderResource;
	}

	// Verify shader pointer provided for initialisation
	if (!ppOutShader) return ErrorCodes::InvalidShaderReferenceProvidedForInitialisation;
	if (*ppOutShader)
	{
		Game::Log << LOG_WARN << "Shader resource already exists, existing resource will be deallocated and overwritten\n";
		SafeDelete(*ppOutShader);
	}

	// Attempt to initialise from file
	(*ppOutShader) = new ShaderDX11();
	bool success = (*ppOutShader)->LoadShaderFromFile(shadertype, ConvertStringToWString(fileName), entryPoint, profile, input_layout);

	// Deallocate the shader object if initialisation failed
	if (!success)
	{
		Game::Log << LOG_ERROR << "Initialisation of shader \"" << entryPoint << "\" failed, cannot proceed\n";
		SafeDelete(*ppOutShader);
		return ErrorCodes::CannotLoadShaderFromFile;
	}

	// Add to the central shader collection and return success.  All shaders are owned by this collection and will
	// be disposed by it during shutdown.  The RenderDeviceDX11::m_* pointers are for efficiency since we know they 
	// will always be non-null
	m_shaders[entryPoint] = std::move(std::unique_ptr<ShaderDX11>(*ppOutShader));
	Game::Log << LOG_INFO << "Shader \"" << entryPoint << "\" loaded successfully\n";
	return ErrorCodes::NoError;
}

// Initialise all sampler states that will be bound during shader rendering
Result RenderDeviceDX11::InitialiseSamplerStateDefinitions(void)
{
	Result result = ErrorCodes::NoError;
	Game::Log << LOG_INFO << "Initialising sampler state definitions\n";

	m_sampler_linearclamp = new SamplerStateDX11();
	m_sampler_linearclamp->SetFilter(SamplerState::MinFilter::MinLinear, SamplerState::MagFilter::MagLinear, SamplerState::MipFilter::MipLinear);
	m_sampler_linearclamp->SetWrapMode(SamplerState::WrapMode::Clamp, SamplerState::WrapMode::Clamp, SamplerState::WrapMode::Clamp);
	HandleErrors(StoreNewSamplerState("LinearClampSampler", m_sampler_linearclamp), result);

	m_sampler_linearrepeat = new SamplerStateDX11();
	m_sampler_linearrepeat->SetFilter(SamplerState::MinFilter::MinLinear, SamplerState::MagFilter::MagLinear, SamplerState::MipFilter::MipLinear);
	m_sampler_linearrepeat->SetWrapMode(SamplerState::WrapMode::Repeat, SamplerState::WrapMode::Repeat, SamplerState::WrapMode::Repeat);
	HandleErrors(StoreNewSamplerState("LinearRepeatSampler", m_sampler_linearrepeat), result);

	Game::Log << LOG_INFO << "Sample state definitions initialised\n";
}

Result RenderDeviceDX11::StoreNewSamplerState(const std::string & name, SamplerStateDX11 *sampler)
{
	if (name.empty()) { Game::Log << LOG_ERROR << "Cannot initialise sampler state definition with null identifier\n"; return ErrorCodes::CouldNotCreateSamplerState; }
	if (sampler == NULL) { Game::Log << LOG_ERROR << "Failed to initialise sampler state definition \"" << name << "\"\n"; return ErrorCodes::CouldNotCreateSamplerState; }

	if (m_samplers.find(name) != m_samplers.end())
	{
		Game::Log << LOG_WARN << "Sample state definition for \"" << name << "\" already exists, cannot create duplicate\n";
		return ErrorCodes::NoError;
	}

	m_samplers[name] = std::move(std::unique_ptr<SamplerStateDX11>(sampler));
	Game::Log << LOG_INFO << "Initialised sampler state \"" << name << "\" definition\n";
	return ErrorCodes::NoError;
}

// Initialise all resources (e.g. GBuffer) required for the deferred rendering process
Result RenderDeviceDX11::InitialiseDeferredRenderingResources(void)
{
	
}

void RenderDeviceDX11::BeginDeferredRenderingFrame(void)
{

}





void RenderDeviceDX11::EndDeferredRenderinFrame(void)
{

}


RenderDeviceDX11::~RenderDeviceDX11(void)
{
	Game::Log << LOG_INFO << "Terminating render device context \"" << Rendering::GetRenderDeviceContextTypeName() << "\"\n";
	ReleaseIfExists(m_devicecontext);

	Game::Log << LOG_INFO << "Terminating primary render device \"" << Rendering::GetRenderDeviceTypeName() << "\"\n";
	ReleaseIfExists(m_device);
}









