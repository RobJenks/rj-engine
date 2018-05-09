#pragma once

#include <string>
#include <filesystem>
#include "../Definitions/ModelData.h"
namespace fs = std::experimental::filesystem;


class ObjFormatIntegration
{
public:

	static std::string					GenerateObjData(const ModelData & model, const std::string & material = "");
	
	static bool							GenerateObjFile(const ModelData & model, fs::path target_file, const std::string & material = "");


private:

	static fs::path						GetAssociatedMaterialFile(fs::path source_file);
	static void							GenerateMaterialFile(fs::path source_file, const std::string & material_texture);


};