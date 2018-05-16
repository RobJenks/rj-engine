#pragma once

#include <memory>
#include <vector>
#include <DirectXMath.h>
class ModelData;
using namespace DirectX;


struct ModelSizeProperties
{
public:

	XMFLOAT3							MinBounds;	
	XMFLOAT3							MaxBounds;	
	XMFLOAT3							ModelSize;	
	XMFLOAT3							CentrePoint;


	ModelSizeProperties(void);
	ModelSizeProperties(const XMFLOAT3 & minbounds, const XMFLOAT3 & maxbounds);
	ModelSizeProperties(const XMFLOAT3 & minbounds, const XMFLOAT3 & maxbounds, const XMFLOAT3 & modelsize, const XMFLOAT3 & centrepoint);

	void								RecalculateDerivedData(void);

	bool								HasData(void) const;


	static ModelSizeProperties			Calculate(const ModelData & model);
	static ModelSizeProperties			Calculate(const std::vector<ModelSizeProperties> & properties);
	static ModelSizeProperties			Calculate(const std::vector<std::unique_ptr<ModelData>> & models, bool recalculate_individual_models);


};
