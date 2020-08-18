#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <DirectXPackedVector.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "ThridParty/DXTrace.h"
#include "Vertex.h"
#include "light.h"

/** Settings for the volumetric lightmap. */
struct FVolumetricLightmapSettings
{
	/** Size of the top level grid covering the volumetric lightmap, in bricks. */
	DirectX::XMINT3 TopLevelGridSize;

	/** World space size of the volumetric lightmap. */
	DirectX::XMFLOAT3 VolumeMin;
	DirectX::XMFLOAT3 VolumeSize;

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
	DXGI_FORMAT Format;
	void Resize(size_t size) {
		data.resize(size * FormatSize);
	}

};

struct BrickDataImported {
	DirectX::XMINT3 IndirectionTexturePosition;
	INT32 TreeDepth;
	float AverageClosestGeometryDistance;
	std::vector<DirectX::PackedVector::XMFLOAT3PK> AmbientVector;
	std::vector<DirectX::PackedVector::XMCOLOR> SHCoefficients[6];

	BrickDataImported() = default;
};

struct BrickData {
	DirectX::XMINT3 IndirectionTexturePosition;
	INT32 TreeDepth;
	float AverageClosestGeometryDistance;
	std::vector<DirectX::PackedVector::XMFLOAT3PK> AmbientVector;
	std::vector<DirectX::PackedVector::XMCOLOR> SHCoefficients[6];
	std::vector<DirectX::PackedVector::XMFLOAT3PK> LQLightColor;
	std::vector<DirectX::PackedVector::XMCOLOR> LQLightDirection;
	std::vector<DirectX::PackedVector::XMCOLOR> SkyBentNormal;
	std::vector<UINT8> DirectionalLightShadowing;

	BrickData() = default;
};

struct VLMBrickData {
	Texture_t AmbientVector;
	Texture_t SHCoefficients[6];
	Texture_t LQLightColor;
	Texture_t LQLightDirection;
	Texture_t SkyBentNormal;
	Texture_t DirectionalLightShadowing;
};

struct VLMData {
	DirectX::XMINT3 textureDimension;
	Texture_t indirectionTexture;
	
	DirectX::XMINT3 brickDataDimension;
	VLMBrickData brickData;

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

	/** Data used by the editor import process and not uploaded into textures. */
	struct FIrradianceVoxelImportProcessingData
	{
		bool bInsideGeometry;
		bool bBorderVoxel;
		float ClosestGeometryDistance;
	};

	VLMData vlmData;
	std::vector<BrickData> importedData;
	FVolumetricLightmapSettings VLMSetting;

	Importer() = default;
	virtual ~Importer();

	bool Read();
	
	template<class T>
	void ReadArray(std::vector<T>& arr);

	template<class T>
	void ReadArray(std::vector<T>& arr, std::ifstream& importer);

	bool Record(const wchar_t* filename);

	void TransformData();
	bool ImportFile(const wchar_t* brickDataFile,
		const wchar_t* bricksByDepthFile = nullptr,
		const wchar_t* indirectTextureFilename = nullptr,
		const wchar_t* ambientVectorFilename = nullptr,
		const wchar_t* SH0CoefsFilename = nullptr,
		const wchar_t* SH1CoefsFilename = nullptr,
		const wchar_t* SH2CoefsFilename = nullptr,
		const wchar_t* SH3CoefsFilename = nullptr,
		const wchar_t* SH4CoefsFilename = nullptr,
		const wchar_t* SH5CoefsFilename = nullptr);


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

	std::unique_ptr<std::ifstream> pBrickDataImporter;
	std::unique_ptr<std::ifstream> pBrickByDepthImporter;
	std::unique_ptr<std::ifstream> pIndirectionTextureImporter;
	std::unique_ptr<std::ifstream> pAmbientVectorImporter;
	std::unique_ptr<std::ifstream> pSH0CoefsImporter;
	std::unique_ptr<std::ifstream> pSH1CoefsImporter;
	std::unique_ptr<std::ifstream> pSH2CoefsImporter;
	std::unique_ptr<std::ifstream> pSH3CoefsImporter;
	std::unique_ptr<std::ifstream> pSH4CoefsImporter;
	std::unique_ptr<std::ifstream> pSH5CoefsImporter;
	bool hasAllSHCoefsTextures;
};

