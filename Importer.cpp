#include "Importer.h"
#include <assert.h>

using namespace std;

Importer::Importer(const wchar_t* filename) {
	pIfStream = new std::ifstream(filename, std::ios::in | std::ios::binary);
}

Importer::~Importer() {
	delete pIfStream;
}


bool Importer::Read() {
	if (!pIfStream->is_open()) {
		std::cout << "open ifstream failed";
		return false;
	}

	while (!pIfStream->eof())
	{
		INT32 numBricks;
		pIfStream->read(reinterpret_cast<char*>(&numBricks), sizeof(INT32));
		for (INT32 i = 0; i < numBricks; i++) {
			importedData.push_back(BrickData());
			BrickData& refData = importedData.back();

			pIfStream->read(reinterpret_cast<char*>(&refData.IndirectionTexturePosition), sizeof(refData.IndirectionTexturePosition));
			pIfStream->read(reinterpret_cast<char*>(&refData.TreeDepth), sizeof(refData.TreeDepth));
			pIfStream->read(reinterpret_cast<char*>(&refData.AverageClosestGeometryDistance), sizeof(refData.AverageClosestGeometryDistance));
			refData.AmbientVector.FormatSize = sizeof(DirectX::PackedVector::XMFLOAT3PK);
			ReadArray(refData.AmbientVector);
			for (INT32 i = 0; i < 6; i++) {
				refData.SHCoefficients[i].FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
				ReadArray(refData.SHCoefficients[i]);
			}
			refData.LQLightColor.FormatSize = sizeof(DirectX::PackedVector::XMFLOAT3PK);
			ReadArray(refData.LQLightColor);
			refData.LQLightDirection.FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
			ReadArray(refData.LQLightDirection);
			refData.SkyBentNormal.FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
			ReadArray(refData.SkyBentNormal);
			refData.DirectionalLightShadowing.FormatSize = sizeof(UINT8);
			ReadArray(refData.DirectionalLightShadowing);
			//ReadArray(refData.TaskVoxelImportProcessingData);
		}
	}
	if (!importedData.empty() && importedData.back().AmbientVector.data.empty()) importedData.pop_back();
	return true;
}


void Importer::ReadArray(Texture_t& texture) {
	INT32 numArray = 0;
	pIfStream->read(reinterpret_cast<char*>(&numArray), sizeof(INT32));
	size_t size = texture.FormatSize;
	texture.Resize(numArray);
	int index = 0;
	for (INT32 i = 0; i < numArray; i++) {
		pIfStream->read(reinterpret_cast<char*>(&texture.data[index]), size);
		index += size;
	}
}


bool Importer::Record(const wchar_t* filename) {
	std::ofstream fout(filename);
	if (!fout.is_open()) return false;
	for (int i = 0; i < importedData.size();i++) {
		const auto& brick = importedData[i];
		fout << "Brick " << i << std::endl;
		fout << "TexturePosition " << brick.IndirectionTexturePosition.x << "," << brick.IndirectionTexturePosition.y << "," << brick.IndirectionTexturePosition.z << std::endl;
		fout << "Depth " << brick.TreeDepth << std::endl;
		fout << "Distance " << brick.AverageClosestGeometryDistance << std::endl;
		fout << "SH Coefs" << std::endl;
		for (const auto & texture: brick.SHCoefficients) {
			for (int i = 0; i < texture.data.size(); i+=texture.FormatSize) {
				fout << (int)texture.data[i] << "," << (int)texture.data[i+1] << "," << (int)texture.data[i+2] << "," << (int)texture.data[i+3] << std::endl;
			}
		}
	}
	fout.close();
	return true;
}

void Importer::initVLMSetting() {
	VLMSetting.BrickSize = 4;
	VLMSetting.MaxRefinementLevels = 3;
	VLMSetting.VolumeSize = DirectX::XMINT3(12800, 12800, 12800);
	VLMSetting.TopLevelGridSize = DirectX::XMINT3(1,1,1);
}

const DirectX::XMINT3 computePositionByLayout(int index, DirectX::XMINT3 layout) {
	DirectX::XMINT3 res;
	res.x = index % layout.x;
	res.y = index / layout.x % layout.y;
	res.z = index / (layout.x * layout.y);
	return res;
}

INT32 inline divideRoundUp(INT32 x, INT32 y) {
	return x % y == 0 ? x / y : x / y + 1;
}


void Importer::BuildIndirectionTexture(
	const vector<vector<const BrickData*>>& BricksByDepth,
	const INT32 maxLayoutDimension,
	const DirectX::XMINT3 Layout,
	const INT32 formatSize,
	VLMData& vlmData) {
	int BrickIndex = 0;
	for (int i = 0; i < VLMSetting.MaxRefinementLevels; i++) {
		for (int j = 0; j < BricksByDepth[i].size(); j++) {
			const BrickData& brick = *BricksByDepth[i][j];
			
			DirectX::XMINT3 layoutPos = computePositionByLayout(BrickIndex, Layout);
			
			assert(layoutPos.x < maxLayoutDimension && layoutPos.y < maxLayoutDimension && layoutPos.z < maxLayoutDimension);
			
			const INT32 DetailCellsPerCurrentLevelBrick = pow(VLMSetting.BrickSize, VLMSetting.MaxRefinementLevels - brick.TreeDepth);
			const INT32 NumBottomLevelBricks = DetailCellsPerCurrentLevelBrick / VLMSetting.BrickSize;
			assert(NumBottomLevelBricks < maxLayoutDimension);

			auto& texture = vlmData.indirectionTexture;
			for (int z = 0; z < NumBottomLevelBricks; z++) {
				for (int y = 0; y < NumBottomLevelBricks; y++) {
					for (int x = 0; x < NumBottomLevelBricks; x++) {
						const DirectX::XMINT3 IndirectionDestDataCoordinate = DirectX::XMINT3(brick.IndirectionTexturePosition.x + x , brick.IndirectionTexturePosition.y + y, brick.IndirectionTexturePosition.z + z);
						const INT32 IndirectionDestDataIndex = (IndirectionDestDataCoordinate.z * vlmData.textureDimension.x * vlmData.textureDimension.y)
							+ (IndirectionDestDataCoordinate.y * vlmData.textureDimension.x) + IndirectionDestDataCoordinate.x * texture.FormatSize;
						texture.data[IndirectionDestDataIndex] = layoutPos.x;
						texture.data[IndirectionDestDataIndex+1] = layoutPos.y;
						texture.data[IndirectionDestDataIndex+2] = layoutPos.z;
						texture.data[IndirectionDestDataIndex+3] = NumBottomLevelBricks;
					}
				}
			}

			BrickIndex++;
		}
	}

}


void Importer::CopyBrickToTexture(
	INT32 formatSize,
	const DirectX::XMINT3& brickDataDimension,
	const DirectX::XMINT3& layoutPos,
	const UINT8* srcPtr,
	UINT8* destPtr
) {
	INT32 brickSize = VLMSetting.BrickSize;
	for (INT32 Z = 0; Z < brickSize; Z++) {
		for (INT32 Y = 0; Y < brickSize; Y++) {
			INT32 srcIndex = Z * brickSize * brickSize * formatSize + Y * brickSize * formatSize;
			INT32 destIndex = (layoutPos.z + Z) * brickDataDimension.y * brickDataDimension.x * formatSize + (layoutPos.y + Y) * brickDataDimension.x * formatSize;
			memcpy(destPtr + destIndex, srcPtr + srcIndex, brickSize * formatSize);
		}
	}
}


void Importer::TransformData() {
	
	initVLMSetting();
	vector<vector<const BrickData*>> BricksByDepth(3);
	for (const auto & tmp : importedData) {
		BricksByDepth[tmp.TreeDepth].push_back(&tmp);
	}
	
	INT32 LayoutAllocator = importedData.size();
	const INT32 MaxBricksInLayoutOneDim = 1 << 8;
	DirectX::XMINT3 BrickLayoutDimensions;
	BrickLayoutDimensions.x = DirectX::XMMin(LayoutAllocator, MaxBricksInLayoutOneDim);
	LayoutAllocator = divideRoundUp(LayoutAllocator, BrickLayoutDimensions.x);
	BrickLayoutDimensions.y = DirectX::XMMin(LayoutAllocator, MaxBricksInLayoutOneDim);
	LayoutAllocator = divideRoundUp(LayoutAllocator, BrickLayoutDimensions.y);
	BrickLayoutDimensions.z = DirectX::XMMin(LayoutAllocator, MaxBricksInLayoutOneDim);

	const INT32 DetailCellsPerTopLevelBrick = pow(VLMSetting.BrickSize, VLMSetting.MaxRefinementLevels); // 64
	const INT32 IndirectionCellsPerTopLevelCell = DetailCellsPerTopLevelBrick / VLMSetting.BrickSize;	// 16

	VLMData vlmData;
	auto& inTexture = vlmData.indirectionTexture;
	inTexture.FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
	vlmData.textureDimension.x = VLMSetting.TopLevelGridSize.x * IndirectionCellsPerTopLevelCell;
	vlmData.textureDimension.y = VLMSetting.TopLevelGridSize.y * IndirectionCellsPerTopLevelCell;
	vlmData.textureDimension.z = VLMSetting.TopLevelGridSize.z * IndirectionCellsPerTopLevelCell;
	size_t TotalTextureSize = vlmData.textureDimension.x * vlmData.textureDimension.y * vlmData.textureDimension.z;
	inTexture.Resize(TotalTextureSize);
	BuildIndirectionTexture(BricksByDepth, MaxBricksInLayoutOneDim, BrickLayoutDimensions, inTexture.FormatSize, vlmData);

	const INT32 PaddedBrickSize = VLMSetting.BrickSize + 1;
	vlmData.brickDataDimension.x = BrickLayoutDimensions.x * PaddedBrickSize;
	vlmData.brickDataDimension.y = BrickLayoutDimensions.y * PaddedBrickSize;
	vlmData.brickDataDimension.z = BrickLayoutDimensions.z * PaddedBrickSize;
	const INT32 TotalBrickData = vlmData.brickDataDimension.x * vlmData.brickDataDimension.y * vlmData.brickDataDimension.z;
	vlmData.brickData.AmbientVector.Resize(TotalBrickData);
	for (auto& SHCoef : vlmData.brickData.SHCoefficients) {
		SHCoef.Resize(TotalBrickData);
	}
	vlmData.brickData.DirectionalLightShadowing.Resize(TotalBrickData);
	vlmData.brickData.LQLightColor.Resize(TotalBrickData);
	vlmData.brickData.LQLightDirection.Resize(TotalBrickData);

	INT32 BrickIndex = 0;
	for (INT32 depth = 0; depth < VLMSetting.MaxRefinementLevels; depth++) {
		for (INT32 indexCurrentDepth = 0; indexCurrentDepth < BricksByDepth[depth].size(); indexCurrentDepth++) {
			const BrickData& brick = *BricksByDepth[depth][indexCurrentDepth];
			DirectX::XMINT3 layoutPos = computePositionByLayout(BrickIndex + indexCurrentDepth, BrickLayoutDimensions);
			layoutPos.x *= PaddedBrickSize; layoutPos.y *= PaddedBrickSize; layoutPos.z *= PaddedBrickSize;
			CopyBrickToTexture(
				brick.AmbientVector.FormatSize,
				vlmData.brickDataDimension,
				layoutPos,
				(UINT8*)(&brick.AmbientVector.data),
				(UINT8*)(&vlmData.brickData.AmbientVector.data)
			);
		}
		BrickIndex += BricksByDepth[depth].size();
	}
}




