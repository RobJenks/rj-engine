#include "BlendState.h"


// Alpha blend state
const BlendState::BlendMode BlendState::BlendModes::AlphaBlend = BlendState::BlendMode
(
	true,											// Blend enabled
	false,											// Logic op enabled
	BlendState::BlendFactor::SrcAlpha,				// Src factor
	BlendState::BlendFactor::OneMinusSrcAlpha		// Dest factor
);


// Additive blend state
const BlendState::BlendMode BlendState::BlendModes::AdditiveBlend = BlendState::BlendMode
(
	true,											// Blend enabled
	false,											// Logic op enabled
	BlendState::BlendFactor::One,					// Src factor
	BlendState::BlendFactor::One					// Dest factor
);

