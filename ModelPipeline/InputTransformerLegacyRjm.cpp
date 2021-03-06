#include <iostream>
#include <fstream>
#include <sstream>
#include "InputTransformerLegacyRjm.h"
#include "PipelineUtil.h"


InputTransformerLegacyRjm::InputTransformerLegacyRjm(const std::string & texture, bool generate_obj, unsigned int operations)
	:
	InputTransformerAssimp(operations), 
	m_texture(texture), 
	m_generate_obj(generate_obj)
{
}


std::vector<std::unique_ptr<ModelData>> InputTransformerLegacyRjm::ExecuteTransform(fs::path file) const
{
	// Read the file data and transform it from (RJM -> OBJ)
	if (!fs::exists(file)) return {};

	TRANSFORM_INFO << "Importing RJM data from file\n";
	std::string data = PipelineUtil::ReadFileToString(file);

	std::string obj_data = ConvertRjmToObj(data, file);
	if (obj_data.empty())
	{
		return {};
	}

	// Generate a new temporary file alongside the RJM and apply transformations using that data
	fs::path obj_path = PipelineUtil::NewTemporaryFileWithExistingFile(file, "obj");
	PipelineUtil::WriteDataTofile(obj_path, obj_data);

	// Generate a material file if a texture resource was provided.  This is not strictly 'input' but is relevant when creating an OBJ file
	if (!m_texture.empty())
	{
		GenerateMaterialFile(file);
	}

	// Make a copy of the source file, in OBJ format, if relevant flag is set
	if (m_generate_obj)
	{
		fs::copy_file(obj_path, fs::path(fs::absolute(file.parent_path()).string() + "/" + file.stem().string() + ".obj"), fs::copy_options::overwrite_existing);
	}

	// Now delegate to the Assimp transformer which accepts an OBJ file as input
	auto model_data = InputTransformerAssimp::Transform(obj_path);

	// Delete the temporary file and return the model data
	PipelineUtil::DeleteTemporaryFile(obj_path);
	return std::move(std::vector<std::unique_ptr<ModelData>>({ std::move(model_data) }));
}

std::vector<std::unique_ptr<ModelData>> InputTransformerLegacyRjm::ExecuteTransform(const std::string & data) const
{
	// Save data to a temporary file, then process as normal and clean up the temporary file
	fs::path file = PipelineUtil::NewTemporaryFile("rjm");
	auto model_data = ExecuteTransform(file);
	PipelineUtil::DeleteTemporaryFile(file);

	return model_data;
}

std::string InputTransformerLegacyRjm::ConvertRjmToObj(const std::string & data, fs::path source_file) const
{
	static const std::string HEADER_START = "Vertex Count:";
	static const std::string HEADER_END = "Data:";

	TRANSFORM_INFO << "Attempting conversion of RJM data to OBJ format\n";

	// Basic verification that this is in fact an RJM file
	if (data.find(HEADER_START, 0U) == std::string::npos ||
		data.find(HEADER_END, 0U) == std::string::npos ||
		data.find("vn ", 0U) != std::string::npos)
	{
		TRANSFORM_ERROR << "File \"" << source_file.string() << "\" does not appear to be an RJM file, terminating\n";
		return "";
	}

	std::vector<XMFLOAT3> v;
	std::vector<XMFLOAT3> n;
	std::vector<XMFLOAT2> t;

	std::ostringstream out;

	// Add material header if texture resources were provided
	if (!m_texture.empty()) out << "mtllib " << GetAssociatedMaterialFile(source_file).filename().string() << "\n";

	// Process each line in turn
	std::istringstream stream(data);
	std::string line;
	while (std::getline(stream, line))
	{
		std::vector<std::string> components;
		std::istringstream linedata(line);
		std::string comp;

		while (std::getline(linedata, comp, ' '))
		{
			if (!comp.empty()) components.push_back(comp);
		}

		// Valid lines have 8 components
		if (components.size() != 8U) continue;

		v.push_back(XMFLOAT3((float)atof(components[0].c_str()), (float)atof(components[1].c_str()), (float)atof(components[2].c_str())));
		n.push_back(XMFLOAT3((float)atof(components[3].c_str()), (float)atof(components[4].c_str()), (float)atof(components[5].c_str())));
		t.push_back(XMFLOAT2((float)atof(components[6].c_str()), (float)atof(components[7].c_str())));
	}

	size_t num_vertices = v.size();
	TRANSFORM_INFO << "Imported data for " << num_vertices << " vertices\n";

	if (num_vertices % 3 != 0)
	{
		TRANSFORM_ERROR << "Model has " << num_vertices << " vertices which is not the expected multiple of three\n";
	}

	for (auto & item : v)		out << "v " << item.x << " " << item.y << " " << item.z << "\n";
	for (auto & item : n)		out << "vn " << item.x << " " << item.y << " " << item.z << "\n";
	for (auto & item : t)		out << "vt " << item.x << " " << item.y << "\n";

	// Material reference should be added before face data if texture resources were provided
	if (!m_texture.empty()) out << "usemtl material0\n";

	// Face indices are 1-based
	for (size_t f = 1; f <= num_vertices; f += 3)
	{
		out << "f " << f << "/" << f << "/" << f << " " << f + 1 << "/" << f + 1 << "/" << f + 1 << " " << f + 2 << "/" << f + 2 << "/" << f + 2 << "\n";
	}

	// Return the transformed data
	TRANSFORM_INFO << "Generated OBJ data from RJM content\n";
	return out.str();
}

fs::path InputTransformerLegacyRjm::GetAssociatedMaterialFile(fs::path source_file) const
{
	return fs::path(fs::absolute(source_file.parent_path()).string() + "/" + source_file.stem().string() + ".mtl");
}

void InputTransformerLegacyRjm::GenerateMaterialFile(fs::path source_file) const
{
	fs::path mtl_file = GetAssociatedMaterialFile(source_file);

	std::ofstream out(mtl_file.string());
	out << "# Generated by RJM->OBJ format converter\n";
	out << "# All model files are associated to at most one material\n\n";

	out << "newmtl material0\n";
	out << "Ka 1.000000 1.000000 1.000000\n";
	out << "Kd 1.000000 1.000000 1.000000\n";
	out << "Ks 0.000000 0.000000 0.000000\n";
	out << "Tr 1.000000\n";
	out << "illum 1\n";
	out << "Ns 0.000000\n";
	out << "map_Kd " << m_texture << "\n";

	out.close();
	TRANSFORM_INFO << "Material file \"" << mtl_file.filename().string() << "\" generated\n";
}