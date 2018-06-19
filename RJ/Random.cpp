#include "Random.h"

// Static initialisation of the underlying random generator
std::default_random_engine Random::m_generator(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
