#include "Material.h"


// Initialise static variables
Material::MaterialID Material::GlobalMaterialIDCount = 0U;


Material::Material(void)
	:
	m_id(NewID())
{
	
}

void Material::AssignNewUniqueID(void)
{
	m_id = NewID();
}

Material::~Material(void)
{

}
