#include "Importer.h"
#include <assert.h>

using namespace std;

Importer::~Importer() {
	delete pIfStream;
}

bool Importer::Read() {
	if (!pIfStream->is_open()) {
		std::cout << "open ifstream failed";
		return false;
	}
	pIfStream->read(reinterpret_cast<char*>(&VLMSetting.VolumeMin), sizeof(float)*3);
	pIfStream->read(reinterpret_cast<char*>(&VLMSetting.VolumeSize), sizeof(float)*3);
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
			ReadArray(refData.AmbientVector);
			for (INT32 i = 0; i < 6; i++) {
				ReadArray(refData.SHCoefficients[i]);
			}
			ReadArray(refData.LQLightColor);
			ReadArray(refData.LQLightDirection);
			ReadArray(refData.SkyBentNormal);
			ReadArray(refData.DirectionalLightShadowing);
		}

	}
	while (!importedData.empty() && importedData.back().AmbientVector.empty()) importedData.pop_back();
	return true;
}

template<class T>
void Importer::ReadArray(vector<T>& arr) {
	INT32 numArray = 0;
	pIfStream->read(reinterpret_cast<char*>(&numArray), sizeof(INT32));
	arr.resize(numArray);
	for (INT32 i = 0; i < numArray; i++) {
		pIfStream->read(reinterpret_cast<char*>(&arr[i]), sizeof(T));
	}
}


bool Importer::Record(const wchar_t* filename) {
	std::ofstream fout(filename);
	if (!fout.is_open()) return false;
	for (int i = 0; i < importedData.size(); i++) {
		const auto& brick = importedData[i];
		fout << "Brick " << i << std::endl;
		fout << "TexturePosition " << brick.IndirectionTexturePosition.x << "," << brick.IndirectionTexturePosition.y << "," << brick.IndirectionTexturePosition.z << std::endl;
		fout << "Depth " << brick.TreeDepth << std::endl;
		fout << "Distance " << brick.AverageClosestGeometryDistance << std::endl;
		fout << "SH Coefs" << std::endl;
		for (const auto& arr : brick.SHCoefficients) {
			for (int i = 0; i < arr.size(); i++) {
				fout << (int)arr[i].b << "," << (int)arr[i].g << "," << (int)arr[i].r << "," << (int)arr[i].a << std::endl;
			}
		}
	}
	fout.close();
	return true;
}

void Importer::InitVLMSetting() {
	VLMSetting.BrickSize = 4;
	VLMSetting.MaxRefinementLevels = 3;
	VLMSetting.TopLevelGridSize = DirectX::XMINT3(1, 1, 1);
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

			assert(layoutPos.x < maxLayoutDimension&& layoutPos.y < maxLayoutDimension&& layoutPos.z < maxLayoutDimension);

			const INT32 DetailCellsPerCurrentLevelBrick = pow(VLMSetting.BrickSize, VLMSetting.MaxRefinementLevels - brick.TreeDepth);
			const INT32 NumBottomLevelBricks = DetailCellsPerCurrentLevelBrick / VLMSetting.BrickSize;
			assert(NumBottomLevelBricks < maxLayoutDimension);

			auto& texture = vlmData.indirectionTexture;
			for (int z = 0; z < NumBottomLevelBricks; z++) {
				for (int y = 0; y < NumBottomLevelBricks; y++) {
					for (int x = 0; x < NumBottomLevelBricks; x++) {
						const DirectX::XMINT3 IndirectionDestDataCoordinate = DirectX::XMINT3(brick.IndirectionTexturePosition.x + x, brick.IndirectionTexturePosition.y + y, brick.IndirectionTexturePosition.z + z);
						const INT32 IndirectionDestDataIndex = ((IndirectionDestDataCoordinate.z * vlmData.textureDimension.x * vlmData.textureDimension.y)
							+ (IndirectionDestDataCoordinate.y * vlmData.textureDimension.x) + IndirectionDestDataCoordinate.x) * texture.FormatSize;
						texture.data[IndirectionDestDataIndex] = layoutPos.x;
						texture.data[IndirectionDestDataIndex + 1] = layoutPos.y;
						texture.data[IndirectionDestDataIndex + 2] = layoutPos.z;
						texture.data[IndirectionDestDataIndex + 3] = NumBottomLevelBricks;
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
			INT32 destIndex = (layoutPos.z + Z) * brickDataDimension.y * brickDataDimension.x * formatSize + (layoutPos.y + Y) * brickDataDimension.x * formatSize + layoutPos.x * formatSize;
			memcpy(destPtr + destIndex, srcPtr + srcIndex, brickSize * formatSize);
		}
	}
}

bool Importer::ImportFile(const wchar_t* filename) {
	pIfStream = new std::ifstream(filename, std::ios::in | std::ios::binary);
	if (!pIfStream->is_open()) return false;
	return true;
}


void ConvertB8G8R8A8ToR8G8B8A8(Texture_t& tex) {
	if ((tex.Format != DXGI_FORMAT_B8G8R8A8_UNORM) && (tex.FormatSize != 4)) return;
	for (int i = 0; i < tex.data.size() / 4; i++) {
		UINT8 r, g, b, a;
		b = tex.data[i * 4 + 0];
		g = tex.data[i * 4 + 1];
		r = tex.data[i * 4 + 2];
		a = tex.data[i * 4 + 3];

		tex.data[i * 4 + 0] = r;
		tex.data[i * 4 + 1] = g;
		tex.data[i * 4 + 2] = b;
		tex.data[i * 4 + 3] = a;
	}
	tex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
}


void Importer::TransformData() {

	InitVLMSetting();
	vector<vector<const BrickData*>> BricksByDepth(3);
	for (const auto& tmp : importedData) {
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

	auto& inTexture = vlmData.indirectionTexture;
	inTexture.FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
	inTexture.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	vlmData.textureDimension.x = VLMSetting.TopLevelGridSize.x * IndirectionCellsPerTopLevelCell;
	vlmData.textureDimension.y = VLMSetting.TopLevelGridSize.y * IndirectionCellsPerTopLevelCell;
	vlmData.textureDimension.z = VLMSetting.TopLevelGridSize.z * IndirectionCellsPerTopLevelCell;
	size_t TotalTextureSize = vlmData.textureDimension.x * vlmData.textureDimension.y * vlmData.textureDimension.z;
	inTexture.Resize(TotalTextureSize);
	std::ifstream indirectionTextureImporter("Texture\\indirectionTexture_Sponza", std::ios::in | std::ios::binary);
	indirectionTextureImporter.read(reinterpret_cast<char*>(inTexture.data.data()), TotalTextureSize*inTexture.FormatSize);

	/*BuildIndirectionTexture(BricksByDepth, MaxBricksInLayoutOneDim, BrickLayoutDimensions, inTexture.FormatSize, vlmData);*/

	const INT32 PaddedBrickSize = VLMSetting.BrickSize + 1;
	vlmData.brickDataDimension.x = BrickLayoutDimensions.x * PaddedBrickSize;
	vlmData.brickDataDimension.y = BrickLayoutDimensions.y * PaddedBrickSize;
	vlmData.brickDataDimension.z = BrickLayoutDimensions.z * PaddedBrickSize;
	const INT32 TotalBrickData = vlmData.brickDataDimension.x * vlmData.brickDataDimension.y * vlmData.brickDataDimension.z;
	vlmData.brickData.AmbientVector.FormatSize = sizeof(DirectX::PackedVector::XMFLOAT3PK);
	vlmData.brickData.AmbientVector.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	vlmData.brickData.AmbientVector.Resize(TotalBrickData);
	std::ifstream ambientImporter("Texture\\AmbientVector_Sponza", std::ios::in | std::ios::binary);
	ambientImporter.read(reinterpret_cast<char*>(vlmData.brickData.AmbientVector.data.data()), vlmData.brickData.AmbientVector.data.size());

	std::ifstream* SH0Importer = new std::ifstream("Texture\\SH0_Sponza", std::ios::in | std::ios::binary);
	std::ifstream* SH1Importer = new std::ifstream("Texture\\SH1_Sponza", std::ios::in | std::ios::binary);
	std::ifstream* SH2Importer = new std::ifstream("Texture\\SH2_Sponza", std::ios::in | std::ios::binary);
	std::ifstream* SH3Importer = new std::ifstream("Texture\\SH3_Sponza", std::ios::in | std::ios::binary);
	std::ifstream* SH4Importer = new std::ifstream("Texture\\SH4_Sponza", std::ios::in | std::ios::binary);
	std::ifstream* SH5Importer = new std::ifstream("Texture\\SH5_Sponza", std::ios::in | std::ios::binary);
	std::vector<std::ifstream*> SHCoefs{ SH0Importer,SH1Importer ,SH2Importer ,SH3Importer ,SH4Importer,SH5Importer };

	for (INT32 i = 0; i < 6; i++)
	{
		vlmData.brickData.SHCoefficients[i].FormatSize = sizeof(DirectX::PackedVector::XMCOLOR); 
		vlmData.brickData.SHCoefficients[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		vlmData.brickData.SHCoefficients[i].Resize(TotalBrickData);
		SHCoefs[i]->read(reinterpret_cast<char*>(vlmData.brickData.SHCoefficients[i].data.data()), vlmData.brickData.SHCoefficients[i].data.size());
	}

	for (auto& ptr : SHCoefs) delete ptr;
	/*
	for (auto& SHCoef : vlmData.brickData.SHCoefficients) {
		SHCoef.FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
		SHCoef.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		SHCoef.Resize(TotalBrickData);
	}
	vlmData.brickData.DirectionalLightShadowing.FormatSize = sizeof(UINT8);
	vlmData.brickData.DirectionalLightShadowing.Format = DXGI_FORMAT_A8_UNORM;
	vlmData.brickData.DirectionalLightShadowing.Resize(TotalBrickData);

	vlmData.brickData.LQLightColor.FormatSize = sizeof(DirectX::PackedVector::XMFLOAT3PK);
	vlmData.brickData.LQLightColor.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	vlmData.brickData.LQLightColor.Resize(TotalBrickData);
	vlmData.brickData.LQLightDirection.FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
	vlmData.brickData.LQLightDirection.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	vlmData.brickData.LQLightDirection.Resize(TotalBrickData);

	INT32 BrickIndex = 0;
	for (INT32 depth = 0; depth < VLMSetting.MaxRefinementLevels; depth++) {
		for (INT32 indexCurrentDepth = 0; indexCurrentDepth < BricksByDepth[depth].size(); indexCurrentDepth++) {
			const BrickData& brick = *BricksByDepth[depth][indexCurrentDepth];
			DirectX::XMINT3 layoutPos = computePositionByLayout(BrickIndex + indexCurrentDepth, BrickLayoutDimensions);
			layoutPos.x *= PaddedBrickSize; layoutPos.y *= PaddedBrickSize; layoutPos.z *= PaddedBrickSize;
			CopyBrickToTexture(
				vlmData.brickData.AmbientVector.FormatSize,
				vlmData.brickDataDimension,
				layoutPos,
				(UINT8*)(brick.AmbientVector.data()),
				(UINT8*)(vlmData.brickData.AmbientVector.data.data())
			);

			for (INT32 i = 0; i < 6; i++) {
				const auto& SHCoef = brick.SHCoefficients[i];
				CopyBrickToTexture(
					vlmData.brickData.SHCoefficients[i].FormatSize,
					vlmData.brickDataDimension,
					layoutPos,
					(UINT8*)(SHCoef.data()),
					(UINT8*)(vlmData.brickData.SHCoefficients[i].data.data())
				);
			}
			CopyBrickToTexture(
				vlmData.brickData.LQLightColor.FormatSize,
				vlmData.brickDataDimension,
				layoutPos,
				(UINT8*)(brick.LQLightColor.data()),
				(UINT8*)(vlmData.brickData.LQLightColor.data.data())
			);
			CopyBrickToTexture(
				vlmData.brickData.LQLightDirection.FormatSize,
				vlmData.brickDataDimension,
				layoutPos,
				(UINT8*)(brick.LQLightDirection.data()),
				(UINT8*)(vlmData.brickData.LQLightDirection.data.data())
			);
			CopyBrickToTexture(
				vlmData.brickData.DirectionalLightShadowing.FormatSize,
				vlmData.brickDataDimension,
				layoutPos,
				(UINT8*)(brick.DirectionalLightShadowing.data()),
				(UINT8*)(vlmData.brickData.DirectionalLightShadowing.data.data())
			);
		}
		BrickIndex += BricksByDepth[depth].size();
	}

	ConvertB8G8R8A8ToR8G8B8A8(vlmData.brickData.SkyBentNormal);
	for (int i = 0; i < 6; i++)
		ConvertB8G8R8A8ToR8G8B8A8(vlmData.brickData.SHCoefficients[i]);
	ConvertB8G8R8A8ToR8G8B8A8(vlmData.brickData.LQLightDirection);*/
}








