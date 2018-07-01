#include "BlendState.h"


// No blending
const BlendState::BlendMode BlendState::BlendModes::NoBlend = BlendState::BlendMode
(
	false,											// Blend disabled
	false											// Logic op disabled
);

// Alpha blend state
const BlendState::BlendMode BlendState::BlendModes::AlphaBlend = BlendState::BlendMode
(
	true,											// Blend enabled
	false,											// Logic op disabled
	BlendState::BlendFactor::SrcAlpha,				// Src factor
	BlendState::BlendFactor::OneMinusSrcAlpha		// Dest factor
);


// Additive blend state
const BlendState::BlendMode BlendState::BlendModes::AdditiveBlend = BlendState::BlendMode
(
	true,											// Blend enabled
	false,											// Logic op disabled
	BlendState::BlendFactor::One,					// Src factor
	BlendState::BlendFactor::One					// Dest factor
);

