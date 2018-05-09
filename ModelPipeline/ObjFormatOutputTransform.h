#pragma once

#include "TransformPipelineOutput.h"


class ObjFormatOutputTransform : public TransformPipelineOutput
{
public:

	inline std::string	GetName(void) const { return "ObjFormatOutputTransform"; }

	ObjFormatOutputTransform(const std::string & material_texture = "");


	ByteString			Transform(std::unique_ptr<ModelData> model) const;
	void				Transform(std::unique_ptr<ModelData> model, fs::path output_file) const;


private:

	std::string			m_material;

};