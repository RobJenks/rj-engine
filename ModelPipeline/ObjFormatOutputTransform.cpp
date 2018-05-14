#include "ObjFormatOutputTransform.h"
#include "ObjFormatIntegration.h"


ObjFormatOutputTransform::ObjFormatOutputTransform(const std::string & material_texture)
	:
	m_material(material_texture)
{
}


ByteString ObjFormatOutputTransform::ExecuteTransform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m)
	{
		TRANSFORM_ERROR << "No valid model data is present; cannot generate output\n";
		return ByteString();
	}

	// Generate OBJ format output
	std::string obj_data = ObjFormatIntegration::GenerateObjData(*m, m_material);
	if (obj_data.empty())
	{
		TRANSFORM_ERROR << "Conversion of model data to OBJ format failed\n";
		return ByteString();
	}
	else
	{
		TRANSFORM_INFO << "Model data converted to OBJ format\n";
	}

	// Return the output directly in a byte string
	ByteString b;
	b.WriteString(obj_data);
	return b;
}

void ObjFormatOutputTransform::ExecuteTransform(std::unique_ptr<ModelData> model, fs::path output_file) const
{
	ModelData *m = model.get();
	if (!m) return;

	// Attempt to remove the target file if it already exists
	if (fs::exists(output_file)) fs::remove(output_file);

	// Generate OBJ format output
	bool generated = ObjFormatIntegration::GenerateObjFile(*m, output_file, m_material);
	if (!generated)
	{
		TRANSFORM_ERROR << "Conversion of model data to OBJ format failed\n";
		return;
	}
	else
	{
		TRANSFORM_INFO << "Model data converted to OBJ format\n";
	}
}










