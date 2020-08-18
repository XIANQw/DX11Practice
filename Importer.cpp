#include "Importer.h"
#include <assert.h>

using namespace std;

Importer::~Importer() {
}

bool Importer::Read() {
	if (!pVLMSettingImporter->is_open()) {
		std::cout << "open VLMSettingImporter failed";
		return false;
	}
	pVLMSettingImporter->read(reinterpret_cast<char*>(&VLMSetting.VolumeMin), sizeof(float) * 3);
	pVLMSettingImporter->read(reinterpret_cast<char*>(&VLMSetting.VolumeSize), sizeof(float) * 3);
	pVLMSettingImporter->read(reinterpret_cast<char*>(&VLMSetting.TopLevelGridSize.x), sizeof(INT32));
	pVLMSettingImporter->read(reinterpret_cast<char*>(&VLMSetting.TopLevelGridSize.y), sizeof(INT32));
	pVLMSettingImporter->read(reinterpret_cast<char*>(&VLMSetting.TopLevelGridSize.z), sizeof(INT32));
	pVLMSettingImporter->read(reinterpret_cast<char*>(&VLMSetting.BrickSize), sizeof(INT32));
	VLMSetting.MaxRefinementLevels = 3;

	if (!pBrickByDepthImporter->is_open()) {
		std::cout << "open BrickByDepthImporter failed";
		return false;
	}

	m_BricksByDepth.resize(VLMSetting.MaxRefinementLevels);
	for (INT32 depth = 0; depth < VLMSetting.MaxRefinementLevels; depth++) {
		m_BricksByDepth[depth].clear();
	}

	pBrickByDepthImporter->read(reinterpret_cast<char*>(&m_BricksNum), sizeof(INT32));
	for (int depth = 0; depth < VLMSetting.MaxRefinementLevels; depth++) {
		int numAtDepth;
		pBrickByDepthImporter->read(reinterpret_cast<char*>(&numAtDepth), sizeof(INT32));
		for (int i = 0; i < numAtDepth; i++) {
			m_BricksByDepth[depth].push_back(BrickDataImported());
			auto& brick = m_BricksByDepth[depth].back();
			pBrickByDepthImporter->read(reinterpret_cast<char*>(&brick.IndirectionTexturePosition), sizeof(brick.IndirectionTexturePosition));
			pBrickByDepthImporter->read(reinterpret_cast<char*>(&brick.TreeDepth), sizeof(INT32));
			pBrickByDepthImporter->read(reinterpret_cast<char*>(&brick.AverageClosestGeometryDistance), sizeof(float));
			ReadArray(brick.AmbientVector, *(pBrickByDepthImporter.get()));
			for (int j = 0; j < 6; j++) {
				ReadArray(brick.SHCoefficients[j], *(pBrickByDepthImporter.get()));
			}
		}
	}

	return true;
}

template<class T>
void Importer::ReadArray(vector<T>& arr, std::ifstream& importer) {
	INT32 numArray = 0;
	importer.read(reinterpret_cast<char*>(&numArray), sizeof(INT32));
	arr.resize(numArray);
	for (INT32 i = 0; i < numArray; i++) {
		importer.read(reinterpret_cast<char*>(&arr[i]), sizeof(T));
	}
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
	const vector<vector<BrickDataImported>>& BricksByDepth,
	const INT32 maxLayoutDimension,
	const DirectX::XMINT3 Layout,
	const INT32 formatSize,
	VLMData& vlmData) {
	int BrickIndex = 0;
	for (int i = 0; i < VLMSetting.MaxRefinementLevels; i++) {
		for (int j = 0; j < BricksByDepth[i].size(); j++) {
			const auto& brick = BricksByDepth[i][j];

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

bool Importer::ImportFile(
	const wchar_t* bricksByDepthFile,
	const wchar_t* vlmSettingFile,
	const wchar_t* indirectTextureFilename,
	const wchar_t* ambientVectorFilename,
	const wchar_t* SH0CoefsFilename,
	const wchar_t* SH1CoefsFilename,
	const wchar_t* SH2CoefsFilename,
	const wchar_t* SH3CoefsFilename,
	const wchar_t* SH4CoefsFilename,
	const wchar_t* SH5CoefsFilename) {
	pBrickByDepthImporter = std::move(unique_ptr<ifstream>(new std::ifstream(bricksByDepthFile, std::ios::in | std::ios::binary)));
	if (!pBrickByDepthImporter->is_open())
		return false;
	pVLMSettingImporter = std::move(unique_ptr<ifstream>(new ifstream(vlmSettingFile, std::ios::in | std::ios::binary)));
	pIndirectionTextureImporter = std::move(unique_ptr<ifstream>(new std::ifstream(indirectTextureFilename, std::ios::in | std::ios::binary)));
	pAmbientVectorImporter = std::move(unique_ptr<ifstream>(new std::ifstream(ambientVectorFilename, std::ios::in | std::ios::binary)));
	pSH0CoefsImporter = std::move(unique_ptr<ifstream>(new std::ifstream(SH0CoefsFilename, std::ios::in | std::ios::binary)));
	pSH1CoefsImporter = std::move(unique_ptr<ifstream>(new std::ifstream(SH1CoefsFilename, std::ios::in | std::ios::binary)));
	pSH2CoefsImporter = std::move(unique_ptr<ifstream>(new std::ifstream(SH2CoefsFilename, std::ios::in | std::ios::binary)));
	pSH3CoefsImporter = std::move(unique_ptr<ifstream>(new std::ifstream(SH3CoefsFilename, std::ios::in | std::ios::binary)));
	pSH4CoefsImporter = std::move(unique_ptr<ifstream>(new std::ifstream(SH4CoefsFilename, std::ios::in | std::ios::binary)));
	pSH5CoefsImporter = std::move(unique_ptr<ifstream>(new std::ifstream(SH5CoefsFilename, std::ios::in | std::ios::binary)));
	hasAllSHCoefsTextures =
		pIndirectionTextureImporter->is_open() &&
		pAmbientVectorImporter->is_open() &&
		pSH0CoefsImporter->is_open() &&
		pSH1CoefsImporter->is_open() &&
		pSH2CoefsImporter->is_open() &&
		pSH3CoefsImporter->is_open() &&
		pSH4CoefsImporter->is_open() &&
		pSH5CoefsImporter->is_open();
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

	INT32 LayoutAllocator = m_BricksNum;
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

	const INT32 PaddedBrickSize = VLMSetting.BrickSize + 1;
	vlmData.brickDataDimension.x = BrickLayoutDimensions.x * PaddedBrickSize;
	vlmData.brickDataDimension.y = BrickLayoutDimensions.y * PaddedBrickSize;
	vlmData.brickDataDimension.z = BrickLayoutDimensions.z * PaddedBrickSize;
	const INT32 TotalBrickData = vlmData.brickDataDimension.x * vlmData.brickDataDimension.y * vlmData.brickDataDimension.z;
	vlmData.brickData.AmbientVector.FormatSize = sizeof(DirectX::PackedVector::XMFLOAT3PK);
	vlmData.brickData.AmbientVector.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	vlmData.brickData.AmbientVector.Resize(TotalBrickData);

	for (INT32 i = 0; i < 6; i++)
	{
		vlmData.brickData.SHCoefficients[i].FormatSize = sizeof(DirectX::PackedVector::XMCOLOR);
		vlmData.brickData.SHCoefficients[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		vlmData.brickData.SHCoefficients[i].Resize(TotalBrickData);
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


	if (hasAllSHCoefsTextures) {
		pIndirectionTextureImporter->read(reinterpret_cast<char*>(inTexture.data.data()), TotalTextureSize * inTexture.FormatSize);
		pAmbientVectorImporter->read(reinterpret_cast<char*>(vlmData.brickData.AmbientVector.data.data()), vlmData.brickData.AmbientVector.data.size());
		
		std::vector<std::ifstream*> SHCoefsImporter{ pSH0CoefsImporter.get(), pSH1CoefsImporter.get(), pSH2CoefsImporter.get(), pSH3CoefsImporter.get(), pSH4CoefsImporter.get(), pSH5CoefsImporter.get() };
		for (int i = 0; i < 6; i++) {
			SHCoefsImporter[i]->read(reinterpret_cast<char*>(vlmData.brickData.SHCoefficients[i].data.data()), vlmData.brickData.SHCoefficients[i].data.size());
		}
	}
	else {
		BuildIndirectionTexture(m_BricksByDepth, MaxBricksInLayoutOneDim, BrickLayoutDimensions, inTexture.FormatSize, vlmData);
		INT32 BrickIndex = 0;
		for (INT32 depth = 0; depth < VLMSetting.MaxRefinementLevels; depth++) {
			for (INT32 indexCurrentDepth = 0; indexCurrentDepth < m_BricksByDepth[depth].size(); indexCurrentDepth++) {
				const BrickDataImported& brick = m_BricksByDepth[depth][indexCurrentDepth];
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
			}
			BrickIndex += m_BricksByDepth[depth].size();
		}

		ConvertB8G8R8A8ToR8G8B8A8(vlmData.brickData.SkyBentNormal);
		for (int i = 0; i < 6; i++)
			ConvertB8G8R8A8ToR8G8B8A8(vlmData.brickData.SHCoefficients[i]);
		ConvertB8G8R8A8ToR8G8B8A8(vlmData.brickData.LQLightDirection);
	}
	
}








