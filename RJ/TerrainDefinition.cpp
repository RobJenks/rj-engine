#include "TerrainDefinition.h"
#include "ShadowSettings.h"

// Recalculate instance rendering flags after a change to a relevant field
void TerrainDefinition::DetermineInstanceRenderingFlags(void)
{
	// Shadow casting state
	InstanceFlags.SetFlagState(InstanceFlags::INSTANCE_FLAG_SHADOW_CASTER,
		ShadowSettings::ShouldCastShadows(m_model, max(max(m_defaultextent.x, m_defaultextent.y), m_defaultextent.z) * 1.414f));
}

void TerrainDefinition::SetModel(Model *m)
{ 
	m_model = m; 
	
	DetermineInstanceRenderingFlags();
}

void TerrainDefinition::SetDefaultExtent(const XMFLOAT3 & e) 
{ 
	m_defaultextent = e; 

	DetermineInstanceRenderingFlags();
}