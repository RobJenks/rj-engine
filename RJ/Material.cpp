#include "Data\\Shaders\\render_constants.h"
#include "FastMath.h"
#include "Material.h"

// Maximum number of materials that can be rendered in one pass
const unsigned int Material::MATERIAL_LIMIT = C_MATERIAL_LIMIT;


// Default constructor
Material::Material(void)
{
	Data.ID = 0;
	Data.Ambient = Data.Diffuse = ONE_FLOAT4;
	Data.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	Data.Reflect = XMFLOAT4(0.25f, 0.25f, 0.25f, 0.25f);
}


// Default destructor
Material::~Material(void)
{

}


