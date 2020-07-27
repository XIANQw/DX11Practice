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


	struct VLMData {
		FGuid IntersectingLevelGuid;
		DirectX::XMINT3 IndirectionTexturePosition;
		INT32 TreeDepth;
		float AverageClosestGeometryDistance;
		std::vector <DirectX::PackedVector::XMFLOAT3PK> AmbientVector;
		std::vector<DirectX::PackedVector::XMCOLOR> SHCoefficients[6];
		std::vector<DirectX::PackedVector::XMFLOAT3PK> LQLightColor;
		std::vector<DirectX::PackedVector::XMCOLOR> LQLightDirection;
		std::vector<DirectX::PackedVector::XMCOLOR> SkyBentNormal;
		std::vector<UINT8> DirectionalLightShadowing;
		std::vector<FIrradianceVoxelImportProcessingData> TaskVoxelImportProcessingData;

		VLMData() = default;
	};

	std::vector<VLMData> importedData;
	
	Importer(const wchar_t* filename);
	virtual ~Importer();

	bool Read();

	template<class T>
	void ReadArray(std::vector<T>& arr);
	bool Record(const char* filename);
	void ShowData();

private:
	std::ifstream* pIfStream;
};

