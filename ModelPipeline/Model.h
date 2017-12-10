#pragma once

#include <memory>
#include <assimp\scene.h>

class Model
{
public:

	static std::unique_ptr<Model>		FromAssimpScene(const aiScene *scene);

private:


};