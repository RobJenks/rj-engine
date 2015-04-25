#pragma once

#ifndef __LoadM3dH__
#define __LoadM3dH__

#include <string>
#include <vector>
#include "MeshGeometry.h"
#include "SkinnedData.h"
#include "SkinnedModel.h"

struct M3dMaterial
{
	SkinnedModel::SM_Material Mat;
	bool AlphaClip;
	std::string EffectTypeName;
	std::string DiffuseMapName;		// Was std::wstring
	std::string NormalMapName;		// Was std::wstring
};

class M3DLoader
{
public:
	/*bool LoadM3d(const std::string& filename, 
		std::vector<Vertex::PosNormalTexTan>& vertices,
		std::vector<USHORT>& indices,
		std::vector<MeshGeometry::Subset>& subsets,
		std::vector<M3dMaterial>& mats);*/
	bool LoadM3d(const std::string& filename, 
		std::vector<SkinnedModel::Vertex_PosNormalTexTanSkinned>& vertices,
		std::vector<USHORT>& indices,
		std::vector<MeshGeometry::Subset>& subsets,
		std::vector<M3dMaterial>& mats,
		SkinnedData& skinInfo, 
		XMFLOAT3 (&bounds)[2]);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<MeshGeometry::Subset>& subsets);
	/* void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices); */
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinnedModel::Vertex_PosNormalTexTanSkinned>& vertices, XMFLOAT3 (&bounds)[2]);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
	void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};



#endif 