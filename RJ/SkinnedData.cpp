#include <string>
#include <map>
#include "SkinnedData.h"
#include "MeshGeometry.h"

Keyframe::Keyframe()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Keyframe::~Keyframe()
{
}
 
float BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	return Keyframes.back().TimePos;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{
	if( t <= Keyframes.front().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if( t >= Keyframes.back().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		std::vector<Keyframe>::size_type kf_upper_limit = (Keyframes.size() - 1); 
		for (std::vector<Keyframe>::size_type i = 0; i < kf_upper_limit; ++i)
		{
			if( t >= Keyframes[i].TimePos && t <= Keyframes[i+1].TimePos )
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i+1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i+1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i+1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i+1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}

/*float AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = FLT_MAX;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}*/

void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms)const
{
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const
{
	std::map<std::string, AnimationClip>::const_iterator clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const
{
	std::map<std::string, AnimationClip>::const_iterator clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

std::vector<int>::size_type SkinnedData::BoneCount()const
{
	return mBoneHierarchy.size();
}

void SkinnedData::Set(std::vector<int>& boneHierarchy, 
		              std::vector<XMFLOAT4X4>& boneOffsets,
		              std::map<std::string, AnimationClip>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets   = boneOffsets;
	mAnimations    = animations;
}

/*void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const
{
	// Attempt to retrieve the animation with this clip name, then call the direct method if we have one
	std::map<std::string, AnimationClip>::const_iterator clip = mAnimations.find(clipName);
	if (clip != mAnimations.end()) GetFinalTransforms(&(clip->second), timePos, finalTransforms);
}*/

void SkinnedData::GetFinalTransforms(const AnimationClip *clip, float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const
{
	// Make sure parameters are valid
	if (!clip) return;
	
	// Allocate space for the transforms based on the number of bones in this mesh
	std::vector<XMFLOAT4X4>::size_type numBones = mBoneOffsets.size();
	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	// Interpolate all the bones of this clip at the given time instance
	clip->Interpolate(timePos, toParentTransforms);

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	// The root bone has index 0.  The root bone has no parent, so its toRootTransform
	// is just its local bone transform.
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for (std::vector<XMFLOAT4X4>::size_type i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	// Premultiply by the bone offset transform to get the final transform.
	for (std::vector<XMFLOAT4X4>::size_type i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose( XMMatrixMultiply(offset, toRoot) ));
	}
}