#include "Importer.h"


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
			importedData.push_back(VLMData());
			VLMData& refData = importedData.back();

			pIfStream->read(reinterpret_cast<char*>(&refData.IntersectingLevelGuid), sizeof(refData.IntersectingLevelGuid));
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
			//ReadArray(refData.TaskVoxelImportProcessingData);
		}
	}
	return true;
}

template<class T>
void Importer::ReadArray(std::vector<T>& arr) {
	INT32 numArray = 0;
	pIfStream->read(reinterpret_cast<char*>(&numArray), sizeof(INT32));
	arr.resize(numArray);
	size_t size = sizeof(T);
	for (INT32 i = 0; i < numArray; i++) {
		pIfStream->read(reinterpret_cast<char*>(&arr[i]), size);
	}
}


bool Importer::Record(const char* filename) {
	std::ofstream fout(filename);
	if (!fout.is_open()) return false;
	for (int i = 0; i < importedData.size();i++) {
		const auto& brick = importedData[i];
		fout << "GUID " << i << std::endl;
		fout << brick.IntersectingLevelGuid.A << "-" << brick.IntersectingLevelGuid.B << "-" << brick.IntersectingLevelGuid.C << "-" << brick.IntersectingLevelGuid.D << std::endl;
		fout << "TexturePosition" << std::endl;
		fout << brick.IndirectionTexturePosition.x << "," << brick.IndirectionTexturePosition.y << "," << brick.IndirectionTexturePosition.z << std::endl;
		fout << "Distance" << std::endl;
		fout << brick.AverageClosestGeometryDistance << std::endl;
		fout << "SH Coefs" << std::endl;
		for (int i = 0; i < 6; i++) {
			for (const auto& v : brick.SHCoefficients[i]) {
				fout << (int)v.b << "," << (int)v.g << "," << (int)v.r << "," << (int)v.a << std::endl;
			}
		}
	}
	fout.close();
	return true;
}



