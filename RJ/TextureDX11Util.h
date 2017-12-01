#pragma once

#include <string>
#include "DX11_Core.h"
#include "Texture.h"

class TextureDX11Util
{
public:

	static DXGI_FORMAT TranslateFormat(const Texture::TextureFormat & format);

	static DXGI_FORMAT GetTextureFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetSRVFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetRTVFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format);

	static uint8_t GetBPP(DXGI_FORMAT format);

	static Texture::TextureFormat TranslateFormat(DXGI_FORMAT format, uint8_t numSamples);

	// Try to choose the best multi-sampling quality level that is supported for the given format.
	static DXGI_SAMPLE_DESC GetSupportedSampleCount(DXGI_FORMAT format, uint8_t numSamples);


	// Reports a more detailed texture error with format information extracted from the texture resource
#	define ReportTextureFormatError( textureformat, message ) LogTextureFormatError( (textureformat), __FILE__, __LINE__, __FUNCTION__, (message) )
	static void LogTextureFormatError(const Texture::TextureFormat& format, const std::string & file, int line, const std::string & function, const std::string & message);


private:




};