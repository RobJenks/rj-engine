#include <fstream>
#include "LoadM3d.h"

/* 
bool M3DLoader::LoadM3d(const std::string& filename, 
						std::vector<Vertex::PosNormalTexTan>& vertices,
						std::vector<USHORT>& indices,
						std::vector<MeshGeometry::Subset>& subsets,
						std::vector<M3dMaterial>& mats)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices  = 0;
	UINT numTriangles = 0;
	UINT numBones     = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if( fin )
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;
 
		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
	    ReadVertices(fin, numVertices, vertices);
	    ReadTriangles(fin, numTriangles, indices);
 
		return true;
	 }
    return false;
}

*/

bool M3DLoader::LoadM3d(const std::string& filename, 
						std::vector<SkinnedModel::Vertex_PosNormalTexTanSkinned>& vertices,
						std::vector<USHORT>& indices,
						std::vector<MeshGeometry::Subset>& subsets,
						std::vector<M3dMaterial>& mats,
						SkinnedData& skinInfo, 
						XMFLOAT3 (&bounds)[2])
{
    std::ifstream fin(filename.c_str());

	UINT numMaterials = 0;
	UINT numVertices  = 0;
	UINT numTriangles = 0;
	UINT numBones     = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if( fin )
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;
 
		std::vector<XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
		std::map<std::string, AnimationClip> animations;

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
	    ReadSkinnedVertices(fin, numVertices, vertices, bounds);
	    ReadTriangles(fin, numTriangles, indices);
		ReadBoneOffsets(fin, numBones, boneOffsets);
	    ReadBoneHierarchy(fin, numBones, boneIndexToParentIndex);
	    ReadAnimationClips(fin, numBones, numAnimationClips, animations);
 
		skinInfo.Set(boneIndexToParentIndex, boneOffsets, animations);

	    return true;
	}
    return false;
}

void M3DLoader::ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats)
{
	 std::string ignore;
     mats.resize(numMaterials);

	 std::string diffuseMapName;
	 std::string normalMapName;

     fin >> ignore; // materials header text
	 for(UINT i = 0; i < numMaterials; ++i)
	 {
			fin >> ignore >> mats[i].Mat.Ambient.x  >> mats[i].Mat.Ambient.y  >> mats[i].Mat.Ambient.z;
			fin >> ignore >> mats[i].Mat.Diffuse.x  >> mats[i].Mat.Diffuse.y  >> mats[i].Mat.Diffuse.z;
			fin >> ignore >> mats[i].Mat.Specular.x >> mats[i].Mat.Specular.y >> mats[i].Mat.Specular.z;
			fin >> ignore >> mats[i].Mat.Specular.w;
			fin >> ignore >> mats[i].Mat.Reflect.x >> mats[i].Mat.Reflect.y >> mats[i].Mat.Reflect.z;
			fin >> ignore >> mats[i].AlphaClip;
			fin >> ignore >> mats[i].EffectTypeName;
			fin >> ignore >> diffuseMapName;
			fin >> ignore >> normalMapName;

			/* *** REMOVED: No longer reading a wstring (unicode).  Simply stick with string.  May need to be updated in future
				   to support OSs other than Windows? ***
			mats[i].DiffuseMapName.resize(diffuseMapName.size(), ' ');
			mats[i].NormalMapName.resize(normalMapName.size(), ' ');
			std::copy(diffuseMapName.begin(), diffuseMapName.end(), mats[i].DiffuseMapName.begin());
			std::copy(normalMapName.begin(), normalMapName.end(), mats[i].NormalMapName.begin());*/
			
			mats[i].DiffuseMapName = diffuseMapName;		// Replacement; simply store the string versions
			mats[i].NormalMapName = normalMapName;			// Replacement; simply store the string versions
		}
}

void M3DLoader::ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<MeshGeometry::Subset>& subsets)
{
    std::string ignore;
	subsets.resize(numSubsets);

	fin >> ignore; // subset header text
	for(UINT i = 0; i < numSubsets; ++i)
	{
        fin >> ignore >> subsets[i].Id;
		fin >> ignore >> subsets[i].VertexStart;
		fin >> ignore >> subsets[i].VertexCount;
		fin >> ignore >> subsets[i].FaceStart;
		fin >> ignore >> subsets[i].FaceCount;
    }
}
/*
void M3DLoader::ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices)
{
	std::string ignore;
    vertices.resize(numVertices);

    fin >> ignore; // vertices header text
    for(UINT i = 0; i < numVertices; ++i)
    {
	    fin >> ignore >> vertices[i].Pos.x      >> vertices[i].Pos.y      >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x >> vertices[i].TangentU.y >> vertices[i].TangentU.z >> vertices[i].TangentU.w;
	    fin >> ignore >> vertices[i].Normal.x   >> vertices[i].Normal.y   >> vertices[i].Normal.z;
	    fin >> ignore >> vertices[i].Tex.x      >> vertices[i].Tex.y;
    }
}
*/
void M3DLoader::ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinnedModel::Vertex_PosNormalTexTanSkinned>& vertices, 
									XMFLOAT3 (&bounds)[2])
{
	std::string ignore;
    vertices.resize(numVertices);
	bounds[0] = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	bounds[1] = XMFLOAT3(FLT_MIN, FLT_MIN, FLT_MIN);

    fin >> ignore; // vertices header text
	int boneIndices[4];
	float weights[4];
    for(UINT i = 0; i < numVertices; ++i)
    {
	    fin >> ignore >> vertices[i].Pos.x        >> vertices[i].Pos.y          >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x   >> vertices[i].TangentU.y     >> vertices[i].TangentU.z >> vertices[i].TangentU.w;
	    fin >> ignore >> vertices[i].Normal.x     >> vertices[i].Normal.y       >> vertices[i].Normal.z;
	    fin >> ignore >> vertices[i].Tex.x        >> vertices[i].Tex.y;
		fin >> ignore >> weights[0]     >> weights[1]     >> weights[2]     >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].Weights.x = weights[0];
		vertices[i].Weights.y = weights[1];
		vertices[i].Weights.z = weights[2];

		vertices[i].BoneIndices[0] = (BYTE)boneIndices[0]; 
		vertices[i].BoneIndices[1] = (BYTE)boneIndices[1]; 
		vertices[i].BoneIndices[2] = (BYTE)boneIndices[2]; 
		vertices[i].BoneIndices[3] = (BYTE)boneIndices[3]; 

		// Keep track of min/max bounds for bounding volumes, collision & scaling later
		if (vertices[i].Pos.x < bounds[0].x) bounds[0].x = vertices[i].Pos.x;
		if (vertices[i].Pos.x > bounds[1].x) bounds[1].x = vertices[i].Pos.x;
		if (vertices[i].Pos.y < bounds[0].y) bounds[0].y = vertices[i].Pos.y;
		if (vertices[i].Pos.y > bounds[1].y) bounds[1].y = vertices[i].Pos.y;
		if (vertices[i].Pos.z < bounds[0].z) bounds[0].z = vertices[i].Pos.z;
		if (vertices[i].Pos.z > bounds[1].z) bounds[1].z = vertices[i].Pos.z;
    }
}

void M3DLoader::ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices)
{
	std::string ignore;
    indices.resize(numTriangles*3);

    fin >> ignore; // triangles header text
    for(UINT i = 0; i < numTriangles; ++i)
    {
        fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
    }
}
 
void M3DLoader::ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets)
{
	std::string ignore;
    boneOffsets.resize(numBones);

    fin >> ignore; // BoneOffsets header text
    for(UINT i = 0; i < numBones; ++i)
    {
        fin >> ignore >> 
            boneOffsets[i](0,0) >> boneOffsets[i](0,1) >> boneOffsets[i](0,2) >> boneOffsets[i](0,3) >>
            boneOffsets[i](1,0) >> boneOffsets[i](1,1) >> boneOffsets[i](1,2) >> boneOffsets[i](1,3) >>
            boneOffsets[i](2,0) >> boneOffsets[i](2,1) >> boneOffsets[i](2,2) >> boneOffsets[i](2,3) >>
            boneOffsets[i](3,0) >> boneOffsets[i](3,1) >> boneOffsets[i](3,2) >> boneOffsets[i](3,3);
    }
}

void M3DLoader::ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex)
{
	std::string ignore;
    boneIndexToParentIndex.resize(numBones);

    fin >> ignore; // BoneHierarchy header text
	for(UINT i = 0; i < numBones; ++i)
	{
	    fin >> ignore >> boneIndexToParentIndex[i];
	}
}

void M3DLoader::ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, 
								   std::map<std::string, AnimationClip>& animations)
{
	std::string ignore;
    fin >> ignore; // AnimationClips header text
    for(UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
    {
        // Read the clip name and convert to lower case; all animation clips are stored lower case and comparison is case-insensitive
		std::string clipName;
        fin >> ignore >> clipName;
		StrLowerC(clipName);

        fin >> ignore; // {

		AnimationClip clip;
		clip.BoneAnimations.resize(numBones);

        for(UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
        {
            ReadBoneKeyframes(fin, numBones, clip.BoneAnimations[boneIndex]);
        }
        fin >> ignore; // }

		// Now precalculate the clip start and end times for runtime efficiency
		float t_start = FLT_MAX; float t_end = 0.0f;
		for(UINT i = 0; i < clip.BoneAnimations.size(); ++i)
		{
			t_start = min(t_start, clip.BoneAnimations[i].GetStartTime());
			t_end = max(t_end, clip.BoneAnimations[i].GetEndTime());
		}
		clip.StartTime = t_start; clip.EndTime = t_end;

		// Store the new animation, indexed by its clip name
		clip.Name = clipName;
        animations[clipName] = clip;
    }
}

void M3DLoader::ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation)
{
	std::string ignore;
    UINT numKeyframes = 0;
    fin >> ignore >> ignore >> numKeyframes;
    fin >> ignore; // {

    boneAnimation.Keyframes.resize(numKeyframes);
    for(UINT i = 0; i < numKeyframes; ++i)
    {
        float t    = 0.0f;
        XMFLOAT3 p(0.0f, 0.0f, 0.0f);
        XMFLOAT3 s(1.0f, 1.0f, 1.0f);
        XMFLOAT4 q(0.0f, 0.0f, 0.0f, 1.0f);
        fin >> ignore >> t;
        fin >> ignore >> p.x >> p.y >> p.z;
        fin >> ignore >> s.x >> s.y >> s.z;
        fin >> ignore >> q.x >> q.y >> q.z >> q.w;

	    boneAnimation.Keyframes[i].TimePos      = t;
        boneAnimation.Keyframes[i].Translation  = p;
	    boneAnimation.Keyframes[i].Scale        = s;
	    boneAnimation.Keyframes[i].RotationQuat = q;
    }

    fin >> ignore; // }
}