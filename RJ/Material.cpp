#include "Data\\Shaders\\render_constants.h"
#include "FastMath.h"
#include "Material.h"

// Maximum number of materials that can be rendered in one pass
const unsigned int Material::MATERIAL_LIMIT = C_MATERIAL_LIMIT;


// Default constructor
Material::Material(void)
{
	AssignUniqueID();
}


// Default destructor
Material::~Material(void)
{

}


