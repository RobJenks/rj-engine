#include "Material.h"

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
