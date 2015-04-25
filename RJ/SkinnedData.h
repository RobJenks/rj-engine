#pragma once

#ifndef __SkinnedDataH__
#define __SkinnedDataH__

#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <xnamath.h>
#include "CompilerSettings.h"

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe();
	~Keyframe();

    float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

    void Interpolate(float t, XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes; 	
};

///<summary>
/// Examples of AnimationClips are "Walk", "Run", "Attack", "Defend".
/// An AnimationClip requires a BoneAnimation for every bone to form
/// the animation clip.    
///</summary>
class AnimationClip
{
public:
	CMPINLINE float GetClipStartTime() const		{ return StartTime; }
	CMPINLINE float GetClipEndTime() const			{ return EndTime; }

    void Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms) const;

    std::vector<BoneAnimation> BoneAnimations; 	
	
	std::string Name;
	float StartTime, EndTime;

	AnimationClip(void) { Name = ""; StartTime = 0.0f; EndTime = 0.0f; }
};

class SkinnedData
{
public:

	UINT BoneCount() const;

	float GetClipStartTime(const std::string& clipName) const;
	float GetClipEndTime(const std::string& clipName) const;

	void Set(
		std::vector<int>& boneHierarchy, 
		std::vector<XMFLOAT4X4>& boneOffsets,
		std::map<std::string, AnimationClip>& animations);

	// Get final transforms based on the specified animation clip
    void GetFinalTransforms(const AnimationClip *clip, float timePos, 
		 std::vector<XMFLOAT4X4>& finalTransforms) const;

	// Get final transforms based on the name of the animation clip.  Preferable to use direct method
    //void GetFinalTransforms(const std::string& clipName, float timePos, 
	//	 std::vector<XMFLOAT4X4>& finalTransforms) const;

	// Retrieves a reference to an animation based on its string code
	CMPINLINE const AnimationClip * GetAnimation(const std::string & code)
	{
		// Attempt to retrieve the animation with this clip name, or return null if we do not have one
		std::map<std::string, AnimationClip>::const_iterator clip = mAnimations.find(code);
		if (clip != mAnimations.end()) return (&(clip->second)); else return NULL;
	}

private:
    // Gives parentIndex of ith bone.
	std::vector<int> mBoneHierarchy;

	std::vector<XMFLOAT4X4> mBoneOffsets;
   
	std::map<std::string, AnimationClip> mAnimations;
};
 

#endif 