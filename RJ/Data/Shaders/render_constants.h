#if !defined(__render_constantsH__)
#define __render_constantsH__

// Maximum number of directional lights that can contribute to a scene
static const unsigned int				C_DIR_LIGHT_LIMIT = 4U;

// Maximum number of lights that can contribute to a scene
static const unsigned int				C_LIGHT_LIMIT = 32U;

// Maximum light count of all types
static const unsigned int				C_TOTAL_LIGHT_LIMIT = (C_DIR_LIGHT_LIMIT + C_LIGHT_LIMIT);

// Maximum number of materials that can be rendered in a single pass
static const unsigned int				C_MATERIAL_LIMIT = 32U;




#endif