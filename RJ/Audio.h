#pragma once

#include <vector>
#include <unordered_map>

class AudioItem;
class AudioInstance;

namespace Audio
{

	/* Key audio type definitions */

	// Collection type and ID used to uniquely identify audio items
	typedef std::vector<AudioItem>						AudioCollection;
	typedef std::vector<AudioItem>::size_type			AudioID;
	typedef std::unordered_map<std::string, AudioID>	AudioIDMap;

	// Collection type and ID used to uniquely identify audio instances
	// NOTE: AudioInstanceID is NOT a persistent identifier, and may change between frames 
	// based on actions by the AudioManager
	typedef std::vector<AudioInstance>					AudioInstanceCollection;
	typedef std::vector<AudioInstance>::size_type		AudioInstanceID;

	// Type used to uniquely number instances as they are generated.  Unique across all audio items
	typedef size_t										AudioInstanceIdentifier;






};



