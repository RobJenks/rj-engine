#include <memory>
#include <iostream>
#include "Model.h"

std::unique_ptr<Model> Model::FromAssimpScene(const aiScene *scene)
{
	if (!scene)
	{
		std::cerr << "Error: Cannot build model; null ai data provided\n";
		return NULL;
	}

	
}