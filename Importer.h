#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include "Vertex.h"
#include "light.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>



/** Settings for the volumetric lightmap. */
struct FVolumetricLightmapSettings
{
	/** Size of the top level grid covering the volumetric lightmap, in bricks. */
	DirectX::XMINT3 TopLevelGridSize;

	/** World space size of the volumetric lightmap. */
	DirectX::XMINT3 VolumeMin;
	DirectX::XMINT3 VolumeSize;

	/**
	 * Size of a brick of unique lighting data.  Must be a power of 2.
	 * Smaller values provide more granularity, but waste more memory due to padding.
	 */
	INT32 BrickSize;

	/** Maximum number of times to subdivide bricks around geometry. */
	INT32 MaxRefinementLevels;

	/**
	 * Fraction of a cell's size to expand it by when voxelizing.
	 * Larger values add more resolution around geometry, improving the lighting gradients but costing more memory.
	 */
	float VoxelizationCellExpansionForSurfaceGeometry;
	float VoxelizationCellExpansionForVolumeGeometry;
	float VoxelizationCellExpansionForLights;

	/** Bricks with RMSE below this value are culled. */
	float MinBrickError;

	/** Triangles with fewer lightmap texels than this don't cause refinement. */
	float SurfaceLightmapMinTexelsPerVoxelAxis;

	/** Whether to cull bricks entirely below landscape.  This can be an invalid optimization if the landscape has holes and caves that pass under landscape. */
	bool bCullBricksBelowLandscape;

	/** Subdivide bricks when a static point or spot light affects some part of the brick with brightness higher than this. */
	float LightBrightnessSubdivideThreshold;

	/** Maximum desired curvature in the lighting stored in Volumetric Lightmaps, used to reduce Spherical Harmonic ringing via a windowing filter. */
	float WindowingTargetLaplacian;
};

struct Texture_t {
	std::vector<UINT8> data;
	size_t FormatSize;

	void Resize(size_t size) {
		data.resize(size * FormatSize);
	}

};

struct BrickData {
	DirectX::XMINT3 IndirectionTexturePosition;
	INT32 TreeDepth;
	float AverageClosestGeometryDistance;
	Texture_t AmbientVector;
	Texture_t SHCoefficients[6];
	Texture_t LQLightColor;
	Texture_t LQLightDirection;
	Texture_t SkyBentNormal;
	Texture_t DirectionalLightShadowing;

	BrickData() = default;
};

struct VLMData {
	DirectX::XMINT3 textureDimension;
	Texture_t indirectionTexture;
	
	DirectX::XMINT3 brickDataDimension;
	BrickData brickData;

	void SetBrickDimension(const DirectX::XMINT3& dimension) {
		brickDataDimension.x = dimension.x;
		brickDataDimension.y = dimension.y;
		brickDataDimension.z = dimension.z;
	}

	VLMData() = default;
};

class Importer
{
public:

	struct FGuid {
		INT32 A,B,C,D;
		FGuid() = default;
		FGuid(INT32 A, INT32 B, INT32 C, INT32 D): A(A), B(B), C(C), D(D){}
	};

	/** Data used by the editor import process and not uploaded into textures. */
	struct FIrradianceVoxelImportProcessingData
	{
		bool bInsideGeometry;
		bool bBorderVoxel;
		float ClosestGeometryDistance;
	};

	std::vector<BrickData> importedData;
	FVolumetricLightmapSettings VLMSetting;

	Importer(const wchar_t* filename);
	virtual ~Importer();

	bool Read();

	void ReadArray(Texture_t& arr);
	bool Record(const wchar_t* filename);

	void TransformData();
	void initVLMSetting();

	void BuildIndirectionTexture(
		const std::vector<std::vector<const BrickData*>>& BricksByDepth,
		const INT32 maxLayoutDimension,
		const DirectX::XMINT3 Layout,
		const INT32 formatSize,
		VLMData& vlmData);
	
	void CopyBrickToTexture(
		INT32 formatSize,
		const DirectX::XMINT3& brickDataDimension,
		const DirectX::XMINT3& layoutPos,
		const UINT8* srcPtr,
		UINT8* destPtr
	);

private:
	std::ifstream* pIfStream;
};

