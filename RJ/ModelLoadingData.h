#pragma once

#include <string>
#include <filesystem>
#include "CompilerSettings.h"
namespace fs = std::filesystem;


struct ModelLoadingData
{
public:

	ModelLoadingData(void);
	ModelLoadingData(const std::string & filename, const std::string & material);

	CMPINLINE std::string		GetFilename(void) const			{ return m_filename; }
	CMPINLINE std::string		GetMaterial(void) const			{ return m_material; }


private:

	std::string					m_filename;
	std::string					m_material;

};