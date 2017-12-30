#include "RenderDeviceDX11.h"
#include "GameVarsExtern.h"
#include "Logging.h"
#include "Utility.h"
#include "ALIGN16.h"
#include "GameDataExtern.h"
#include "Shaders.h"
#include "TextureDX11.h"
#include "ShaderDX11.h"
#include "MaterialDX11.h"
#include "InputLayoutDesc.h"
#include "SamplerStateDX11.h"
#include "RenderTargetDX11.h"
#include "PipelineStateDX11.h"
#include "ConstantBufferDX11.h"
#include "BlendState.h"

// We will negotiate the highest possible supported feature level when attempting to initialise the render device
const D3D_FEATURE_LEVEL RenderDeviceDX11::SUPPORTED_FEATURE_LEVELS[] = 
{ 
	D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 
};


RenderDeviceDX11::RenderDeviceDX11(void)
	:
	m_device(NULL),
	m_devicecontext(NULL),
	m_swapchain(NULL), 
	m_backbuffer(NULL), 
	m_rendertarget(NULL), 
	m_drivertype(D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN),
	m_debuglayer(NULL),
	m_devicename(NullString),
	m_devicememory(0U),

	m_fov(Game::FOV),
	m_halffovtan(tanf(Game::FOV * 0.5f)),
	m_displaysize(Game::ScreenWidth, Game::ScreenHeight),
	m_aspectratio((float)Game::ScreenWidth / (float)Game::ScreenHeight),
	m_screen_near(Game::NearClipPlane),
	m_screen_far(Game::FarClipPlane), 
	m_projection(ID_MATRIX), 
	m_orthographic(ID_MATRIX),

	m_standard_vs(NULL), 
	m_standard_ps(NULL),
	m_deferred_geometry_ps(NULL), 
	m_deferred_lighting_ps(NULL),

	m_sampler_linearclamp(NULL), 
	m_sampler_linearrepeat(NULL), 

	m_cb_frame_data(NULL), 
	m_cb_frame(NULL), 
	m_cb_material_data(NULL), 
	m_cb_material(NULL)
{
	SetRenderDeviceName("RenderDeviceDX11 (Direct3D 11.2)");
}

Result RenderDeviceDX11::Initialise(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_far)
{
	Result result;
	Game::Log << "Initialising rendering engine \"" << GetRenderDeviceName() << "\"\n";

	// Store key data and calculated derived parameters
	SetDisplaySize(screen_size);
	SetFOV(Game::FOV);
	SetDepthPlanes(screen_near, screen_far);
	SetSampleDesc(1U, 0U);

	// Initialise the render device and context
	PERFORM_INIT( InitialiseRenderDevice(hwnd, screen_size, full_screen, vsync), "primary render device" )

	// We can now determine the primary adapter output & capabilities, then initialise it for rendering
	PERFORM_INIT( InitialisePrimaryGraphicsAdapter(screen_size, vsync), "primary graphics adapter" )

	// Swap chain and back buffer
	PERFORM_INIT( InitialiseSwapChain(hwnd, screen_size, full_screen, vsync), "swap chain interfaces" )

	// Primary render target
	PERFORM_INIT( InitialisePrimaryRenderTarget(screen_size), "primary render target" )

	// Initialise input layout descriptors
	PERFORM_INIT( InitialiseInputLayoutDefinitions(), "input layout descriptors" )
	
	// Load all shaders from external resources
	PERFORM_INIT( InitialiseShaderResources(), "shader resources" )

	// Initialise all standard components that are common to many rendering methods
	PERFORM_INIT( InitialiseStandardBuffers(), "common buffer resources" )
	PERFORM_INIT( InitialiseSamplerStateDefinitions(), "sampler state definitions" )
	PERFORM_INIT( InitialiseStandardRenderPipelines(), "standard render pipelines" )



	Game::Log << LOG_INFO << "Initialisation of rendering engine completed successfully\n";
	return ErrorCodes::NoError;
}

Result RenderDeviceDX11::InitialiseRenderDevice(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync)
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
		return ErrorCodes::CannotCreateRenderDevice;
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

Result RenderDeviceDX11::InitialiseSwapChain(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync)
{
	Game::Log << LOG_INFO << "Acquiring swap chain interfaces (target level: " << Rendering::GetSwapChainInterfaceTypeName() << ")\n";

	IDXGIFactory2 *factory = NULL;
	HRESULT hr = CreateDXGIFactory(__uuidof(Rendering::SwapChainInterfaceType), (void**)&factory);
	{
		Game::Log << LOG_ERROR << "Failed to create DXGI factory (hr: " << hr << ")\n";
		return ErrorCodes::CouldNotCreateSwapChain;
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = screen_size.x;
	swapChainDesc.Height = screen_size.y;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SampleDesc = m_sampledesc;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Use Alt-Enter to switch between full screen and windowed mode.

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullScreenDesc = {};

	swapChainFullScreenDesc.RefreshRate = QueryRefreshRateForDisplaySize(screen_size.x, screen_size.y, vsync);
	swapChainFullScreenDesc.Windowed = (full_screen ? TRUE : FALSE);

	// Instantiate the base swap chain interface
	HRESULT hr;
	IDXGISwapChain1 *swapchain = NULL;

	if (FAILED(hr = factory->CreateSwapChainForHwnd(m_device, hwnd,
		&swapChainDesc, &swapChainFullScreenDesc, nullptr, &swapchain)))
	{
		Game::Log << LOG_ERROR << "Failed to create swap chain (hr: " << hr << ")\n";
		return ErrorCodes::CouldNotCreateSwapChain;
	}

	// Attempt to uplift to require swap chain feature level
	if (FAILED(hr = swapchain->QueryInterface<Rendering::SwapChainInterfaceType>(&m_swapchain)))
	{
		Game::Log << LOG_ERROR << "Failed to retrieve \"" << Rendering::GetSwapChainInterfaceTypeName() << "\" interface (hr: " << hr << ")\n";
		return ErrorCodes::CouldNotCreateSwapChain;
	}

	if (FAILED(hr = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_backbuffer)))
	{
		Game::Log << LOG_ERROR << "Failed to get back buffer reference from initialised swap chain (hr: " << hr << ")\n";
		return ErrorCodes::CouldNotCreateSwapChain;
	}

	Game::Log << LOG_INFO << "Swap chain interfaces initialised successfully\n";
	return ErrorCodes::NoError;
}

DXGI_RATIONAL RenderDeviceDX11::QueryRefreshRateForDisplaySize(UINT screenwidth, UINT screenheight, bool vsync)
{
	Game::Log << LOG_INFO << "Querying refresh rate for display size " << screenwidth << "x" << screenheight << (vsync ? " (vsync)" : "") << "\n";
	DXGI_RATIONAL refreshRate = { 0, 1 };

	if (vsync)
	{
		IDXGIFactory *factory = NULL;
		IDXGIAdapter *adapter = NULL;
		IDXGIOutput *adapterOutput = NULL;
		DXGI_MODE_DESC* displayModeList;

		// Create a DirectX graphics interface factory.
		HRESULT hr;
		if (FAILED(hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
		{
			Game::Log << LOG_ERROR << "Failed to create DXGIFactory (hr: " << hr << ")\n";
			return refreshRate;
		}

		if (FAILED(hr = factory->EnumAdapters(0, &adapter)))
		{
			Game::Log << LOG_ERROR << "Failed to enumerate adapters (hr: " << hr << ")\n";
			return refreshRate;
		}

		if (FAILED(hr = adapter->EnumOutputs(0, &adapterOutput)))
		{
			Game::Log << LOG_ERROR << "Failed to enumerate adapter outputs (hr: " << hr << ")\n";
			return refreshRate;
		}

		UINT numDisplayModes;
		if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, NULL)))
		{
			Game::Log << LOG_ERROR << "Failed to query display modes (hr: " << hr << ")\n";
			return refreshRate;
		}

		displayModeList = new DXGI_MODE_DESC[numDisplayModes];
		assert(displayModeList);

		if (FAILED(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList)))
		{
			Game::Log << LOG_ERROR << "Failed to query dispaly mode list (hr: " << hr << ")\n";
			return refreshRate;
		}

		// Now store the refresh rate of the monitor that matches the width and height of the requested screen.
		for (UINT i = 0; i < numDisplayModes; ++i)
		{
			if (displayModeList[i].Width == screenwidth && displayModeList[i].Height == screenheight)
			{
				refreshRate = displayModeList[i].RefreshRate;
				Game::Log << LOG_INFO << "Refresh rate of " << refreshRate.Numerator << "/" << refreshRate.Denominator <<
					" supported for display size " << screenwidth << "x" << screenheight << "\n";
			}
		}

		SafeDeleteArray(displayModeList);
		ReleaseIfExists(adapterOutput);
		ReleaseIfExists(adapter);
		ReleaseIfExists(factory);
	}

	Game::Log << LOG_INFO << "Selected refresh rate of " << refreshRate.Numerator << "/" << refreshRate.Denominator <<
							 " for display size " << screenwidth << "x" << screenheight << (vsync ? " (vsync)" : "") << "\n";
	return refreshRate;
}

Result RenderDeviceDX11::InitialisePrimaryRenderTarget(INTVECTOR2 screen_size)
{
	m_rendertarget = CreateRenderTarget("PrimaryRenderTarget");

	// Initialise depth/stencil buffer
	Texture::TextureFormat depthStencilTextureFormat(
		Texture::Components::DepthStencil,
		Texture::Type::UnsignedNormalized,
		m_sampledesc.Count,
		0, 0, 0, 0, 24, 8);
	TextureDX11 *depthStencilTexture = CreateTexture2D("PrimaryDepthStencil", screen_size.x, screen_size.y, 1, depthStencilTextureFormat);

	// Initialise colour buffer (Color0)
	Texture::TextureFormat colorTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::UnsignedNormalized,
		m_sampledesc.Count,
		8, 8, 8, 8, 0, 0);
	TextureDX11 *colorTexture = CreateTexture2D("PrimaryColour", screen_size.x, screen_size.y, 1, colorTextureFormat);

	// Bind colour and depth/stencil to the primary render target
	m_rendertarget->AttachTexture(RenderTarget::AttachmentPoint::Color0, colorTexture);
	m_rendertarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, depthStencilTexture);

	Game::Log << LOG_INFO << "Initialised primary render target successfully\n";
}

Result RenderDeviceDX11::InitialiseInputLayoutDefinitions(void)
{
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
		{ &m_standard_vs, Shader::Type::VertexShader, Shaders::StandardVertexShader, "Shaders\\vs_standard.vs.hlsl", "latest", &m_standard_input_layout }, 
		{ &m_standard_ps, Shader::Type::PixelShader, Shaders::StandardPixelShader, "Shaders\\ps_standard.ps.hlsl", "latest", NULL }, 

		{ &m_deferred_geometry_ps, Shader::Type::PixelShader, Shaders::DeferredGeometryPixelShader, "Shaders\\deferred_ps_geometry.ps.hlsl", "latest", NULL },
		{ &m_deferred_lighting_ps, Shader::Type::PixelShader, Shaders::DeferredLightingPixelShader, "Shaders\\deferred_ps_lighting.ps.hlsl", "latest", NULL },
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
	bool success = (*ppOutShader)->LoadShaderFromFile(shadertype, ConvertStringToWString(BuildStrFilename(D::DATA, fileName)), entryPoint, profile, input_layout);

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
	m_sampler_linearclamp = CreateSamplerState("LinearClampSampler");
	m_sampler_linearclamp->SetFilter(SamplerState::MinFilter::MinLinear, SamplerState::MagFilter::MagLinear, SamplerState::MipFilter::MipLinear);
	m_sampler_linearclamp->SetWrapMode(SamplerState::WrapMode::Clamp, SamplerState::WrapMode::Clamp, SamplerState::WrapMode::Clamp);

	m_sampler_linearrepeat = CreateSamplerState("LinearRepeatSampler");
	m_sampler_linearrepeat->SetFilter(SamplerState::MinFilter::MinLinear, SamplerState::MagFilter::MagLinear, SamplerState::MipFilter::MipLinear);
	m_sampler_linearrepeat->SetWrapMode(SamplerState::WrapMode::Repeat, SamplerState::WrapMode::Repeat, SamplerState::WrapMode::Repeat);

	Game::Log << LOG_INFO << "Sample state definitions initialised\n";
	return ErrorCodes::NoError;
}

// Initialise all standard pipeline definitions, which can be referenced by render process without needing
// to reimplement each time
Result RenderDeviceDX11::InitialiseStandardRenderPipelines(void)
{
	PipelineStateDX11 * transparency = CreatePipelineState("Transparency");
	transparency->SetShader(Shader::Type::VertexShader, GetShader(Shaders::StandardVertexShader));
	transparency->SetShader(Shader::Type::PixelShader, GetShader(Shaders::StandardPixelShader));
	transparency->GetBlendState().SetBlendMode(BlendState::BlendModes::AlphaBlend);
	transparency->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(true, DepthStencilState::DepthWrite::Disable));
	transparency->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	transparency->SetRenderTarget(GetPrimaryRenderTarget());


}

// Initialise all standard constant buffers that are used across multiple rendering components
Result RenderDeviceDX11::InitialiseStandardBuffers(void)
{
	m_cb_frame_data.RawPtr = { 0 };
	m_cb_frame = CreateConstantBuffer<FrameDataBuffer>("FrameDataBuffer");

	m_cb_material_data.RawPtr = { 0 };
	m_cb_material = CreateConstantBuffer<MaterialBuffer>("MaterialBuffer");
}


TextureDX11 * RenderDeviceDX11::CreateTexture(const std::string & name)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>()));
}

TextureDX11 * RenderDeviceDX11::CreateTexture1D(const std::string & name, uint16_t width, uint16_t slices, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(width, slices, format, cpuAccess, gpuWrite)));
}

TextureDX11 * RenderDeviceDX11::CreateTexture2D(const std::string & name, uint16_t width, uint16_t height, uint16_t slices, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(width, height, slices, format, cpuAccess, gpuWrite)));
}

TextureDX11 * RenderDeviceDX11::CreateTexture3D(const std::string & name, uint16_t width, uint16_t height, uint16_t depth, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(TextureDX11::Tex3d, width, height, depth, format, cpuAccess, gpuWrite)));
}

TextureDX11 * RenderDeviceDX11::CreateTextureCube(const std::string & name, uint16_t size, uint16_t numCubes, const Texture::TextureFormat& format, CPUGraphicsResourceAccess cpuAccess, bool gpuWrite)
{
	return RegisterNewTexture(name, std::move(std::make_unique<TextureDX11>(TextureDX11::Cube, size, numCubes, format, cpuAccess, gpuWrite)));
}

// Attempts to register the given texture.  Returns a pointer to the underlying resource if successful.  Will return NULL, 
// deallocate any underlying resource and report an error if the registration fails
TextureDX11 * RenderDeviceDX11::RegisterNewTexture(const std::string & name, std::unique_ptr<TextureDX11> texture)
{
	if (name.empty())
	{
		Game::Log << LOG_ERROR << "Cannot register texture resource with null identifier\n";
		return  NULL;
	}

	if (texture.get() == NULL)
	{
		Game::Log << LOG_ERROR << "Cannot register \"" << name << "\" as null texture resource\n";
		return NULL;
	}

	if (m_textures.find(name) != m_textures.end())
	{
		Game::Log << LOG_WARN << "Cannot register texxture \"" << name << "\"; resource already exists with this identifier\n";
		return NULL;
	}

	m_textures[name] = std::move(texture);
	Game::Log << LOG_INFO << "Registered new texture resource \"" << name << "\"\n";
	return m_textures[name].get();
}



void RenderDeviceDX11::SetDisplaySize(INTVECTOR2 display_size)
{
	assert(display_size.x > 0 && display_size.y > 0);

	m_displaysize = display_size;
	m_aspectratio = (display_size.x / display_size.y);

	RecalculateOrthographicMatrix();
}

void RenderDeviceDX11::SetFOV(float fov)
{
	assert(fov > 0.0f);

	m_fov = fov;
	m_halffovtan = tanf(fov * 0.5f);

	RecalculateProjectionMatrix();
}

void RenderDeviceDX11::SetDepthPlanes(float screen_near, float screen_far)
{
	assert(screen_far > screen_near);

	m_screen_near = screen_near;
	m_screen_far = screen_far;

	RecalculateProjectionMatrix();
	RecalculateOrthographicMatrix();
}

void RenderDeviceDX11::SetSampleDesc(UINT count, UINT quality)
{
	m_sampledesc.Count = count;
	m_sampledesc.Quality = quality;
}

void RenderDeviceDX11::RecalculateProjectionMatrix(void)
{
	// TODO: Flip near/far plane distances as part of inverted depth buffer for greater FP precision
	// https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.matrix.xmmatrixperspectivefovlh(v=vs.85).aspx
	m_projection = XMMatrixPerspectiveFovLH(m_fov, m_aspectratio, m_screen_near, m_screen_far);
}

void RenderDeviceDX11::RecalculateOrthographicMatrix(void)
{
	m_orthographic = XMMatrixOrthographicLH((float)m_displaysize.x, (float)m_displaysize.y, m_screen_near, m_screen_far);
}


void RenderDeviceDX11::BeginDeferredRenderingFrame(void)
{

}





void RenderDeviceDX11::EndDeferredRenderinFrame(void)
{

}


RenderDeviceDX11::~RenderDeviceDX11(void)
{
	if (m_debuglayer)
	{
		Game::Log << LOG_INFO << "Terminating render device debug layer\n";
		ReleaseIfExists(m_debuglayer);
	}

	Game::Log << LOG_INFO << "Terminating render device context \"" << Rendering::GetRenderDeviceContextTypeName() << "\"\n";
	ReleaseIfExists(m_devicecontext);

	Game::Log << LOG_INFO << "Terminating primary render device \"" << Rendering::GetRenderDeviceTypeName() << "\"\n";
	ReleaseIfExists(m_device);
}









