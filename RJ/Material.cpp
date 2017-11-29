#include "Data\\Shaders\\render_constants.h"
#include "FastMath.h"
#include "Material.h"

// Initialise static variables
Material::MaterialID Material::GlobalMaterialIDCount = 0U;


// Default constructor
Material::Material(void)
{
	AssignUniqueID();
}

// Default copy constructor
Material::Material(const Material &other)
	:
	Data(other.Data)
{
	AssignUniqueID();
}

// Default destructor
Material::~Material(void)
{

}


