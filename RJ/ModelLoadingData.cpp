#include "ModelLoadingData.h"

ModelLoadingData::ModelLoadingData(void)
{
}

ModelLoadingData::ModelLoadingData(const std::string & filename, const std::string & material)
	:
	m_filename(filename), 
	m_material(material)
{
}
