#pragma once

#include "../Definitions/ModelData.h"
#include "InputTransformerAssimp.h"
#include "AssimpIntegration.h"




class InputTransformerLegacyRjm : public InputTransformerAssimp
{

public:

	InputTransformerLegacyRjm(const std::string & texture = "", bool generate_obj = false, unsigned int operations = AssimpIntegration::DefaultOperations());


	inline std::string						GetName(void) const { return "InputTransformLegacyRjm"; }

	virtual std::unique_ptr<ModelData>		Transform(fs::path file) const;
	virtual std::unique_ptr<ModelData>		Transform(const std::string & data) const;


private:

	std::string								ConvertRjmToObj(const std::string & data, fs::path source_file) const;
	fs::path								GetAssociatedMaterialFile(fs::path source_file) const;
	void									GenerateMaterialFile(fs::path source_file) const;

	// Texture filename (in same local directory as model) which will be generated into a material, if set
	std::string								m_texture;

	// Transformer will also generate the equivalent OBJ file alongside final output if this flag is set
	bool									m_generate_obj;

};