#include "Geometry.h"

namespace Geometry
{
	namespace Internal
	{
		/********************************************************
		* 这个部分用于处理不同的vertex类型，VertexPosColor,			*
		* VertexPosColorTex, VertexPosTex, VertexPosColorTangent*
		* 等等, 储存各个成员变量的偏移量范围，来实现模板				*
		*********************************************************/

		struct VertexData {
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT4 tangent;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 tex;
		};


		// 根据目标顶点类型选择性将数据插入
		template<class VertexType>
		inline void InsertVertexElement(VertexType& vertexDst, const VertexData& vertexSrc)
		{
			static std::string semanticName;
			// 第一步，建立内存映射关系
			static const std::map<std::string, std::pair<size_t, size_t>> semanticSizeMap = {
				{"POSITION", std::pair<size_t, size_t>(0, 12)},
				{"NORMAL", std::pair<size_t, size_t>(12, 24)},
				{"TANGENT", std::pair<size_t, size_t>(24, 40)},
				{"COLOR", std::pair<size_t, size_t>(40, 56)},
				{"TEXCOORD", std::pair<size_t, size_t>(56, 64)}
			};

			// 第二步: 
			for (size_t i = 0; i < ARRAYSIZE(VertexType::inputLayout); i++) {
				semanticName = VertexType::inputLayout[i].SemanticName;
				const auto& range = semanticSizeMap.at(semanticName);
				memcpy_s(reinterpret_cast<char*>(&vertexDst) + VertexType::inputLayout[i].AlignedByteOffset,
					range.second - range.first,
					reinterpret_cast<const char*>(&vertexSrc) + range.first,
					range.second - range.first);
			}
		}

	}

	/***************************************
	* 根据长方体的长宽高来生成任意类型的vertex *
	****************************************/

	/* ***********************************
	 设置立方体顶点
		(-x, y, z) ________________  (x, y, z)
				 / |             / |
				/  |		    /  |
      (-x,-y,z)/___|___________/   |
	  (-x,y,-z)|   |_ _ _ _ _ _|_ _| (x, -y, z)
			   |  /		       |0 /		  		         
			   | /			   | /0 
			   |/______________|/
		  (-x,-y,-z)           (x, -y, -z)
					width
	*************************************/
	template<class VertexType, class IndexType>
	MeshData<VertexType, IndexType> CreateBox(float width, float height, float depth,
		const DirectX::XMFLOAT4& color) {
		using namespace DirectX;

		MeshData<VertexType, IndexType> meshData;
		meshData.vertexVec.resize(24);

		Internal::VertexData vertexDataArr[24];
		float x = width / 2, y = height / 2, z = depth / 2;
		//right clock +x
		vertexDataArr[0].pos = XMFLOAT3(x, -y, -z);
		vertexDataArr[1].pos = XMFLOAT3(x, y, -z);
		vertexDataArr[2].pos = XMFLOAT3(x, y, z);
		vertexDataArr[3].pos = XMFLOAT3(x, -y, z);
		//left anticlock -x
		vertexDataArr[4].pos = XMFLOAT3(-x, -y, z);
		vertexDataArr[5].pos = XMFLOAT3(-x, y, z);
		vertexDataArr[6].pos = XMFLOAT3(-x, y, -z);
		vertexDataArr[7].pos = XMFLOAT3(-x, -y, -z);
		//top clock +y
		vertexDataArr[8].pos = XMFLOAT3(-x, y, -z);
		vertexDataArr[9].pos = XMFLOAT3(-x, y, z);
		vertexDataArr[10].pos = XMFLOAT3(x, y, z);
		vertexDataArr[11].pos = XMFLOAT3(x, y, -z);
		//button anticlock -y
		vertexDataArr[12].pos = XMFLOAT3(x, -y, -z);
		vertexDataArr[13].pos = XMFLOAT3(x, -y, z);
		vertexDataArr[14].pos = XMFLOAT3(-x, -y, z);
		vertexDataArr[15].pos = XMFLOAT3(-x, -y, -z);
		//back anti +z
		vertexDataArr[16].pos = XMFLOAT3(x, -y, z);
		vertexDataArr[17].pos = XMFLOAT3(x, y, z);
		vertexDataArr[18].pos = XMFLOAT3(-x, y, z);
		vertexDataArr[19].pos = XMFLOAT3(-x, -y, z);
		//font clock -z
		vertexDataArr[20].pos = XMFLOAT3(-x, -y, -z);
		vertexDataArr[21].pos = XMFLOAT3(-x, y, -z);
		vertexDataArr[22].pos = XMFLOAT3(x, y, -z);
		vertexDataArr[23].pos = XMFLOAT3(x, -y, -z);


		for (UINT i = 0; i < 4; ++i)
		{
			// 右面(+X面)
			vertexDataArr[i].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
			vertexDataArr[i].tangent = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
			vertexDataArr[i].color = color;
			// 左面(-X面)
			vertexDataArr[i + 4].normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);
			vertexDataArr[i + 4].tangent = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
			vertexDataArr[i + 4].color = color;
			// 顶面(+Y面)
			vertexDataArr[i + 8].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertexDataArr[i + 8].tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 8].color = color;
			// 底面(-Y面)
			vertexDataArr[i + 12].normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
			vertexDataArr[i + 12].tangent = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 12].color = color;
			// 背面(+Z面)
			vertexDataArr[i + 16].normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 16].tangent = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 16].color = color;
			// 正面(-Z面)
			vertexDataArr[i + 20].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
			vertexDataArr[i + 20].tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			vertexDataArr[i + 20].color = color;
		}

		for (UINT i = 0; i < 6; ++i)
		{
			vertexDataArr[i * 4].tex = XMFLOAT2(0.0f, 1.0f);
			vertexDataArr[i * 4 + 1].tex = XMFLOAT2(0.0f, 0.0f);
			vertexDataArr[i * 4 + 2].tex = XMFLOAT2(1.0f, 0.0f);
			vertexDataArr[i * 4 + 3].tex = XMFLOAT2(1.0f, 1.0f);
		}

		for (UINT i = 0; i < 24; ++i)
		{
			Internal::InsertVertexElement(meshData.vertexVec[i], vertexDataArr[i]);
		}

		meshData.indexVec = {
			0, 1, 2, 2, 3, 0,		// 右面(+X面)
			4, 5, 6, 6, 7, 4,		// 左面(-X面)
			8, 9, 10, 10, 11, 8,	// 顶面(+Y面)
			12, 13, 14, 14, 15, 12,	// 底面(-Y面)
			16, 17, 18, 18, 19, 16, // 背面(+Z面)
			20, 21, 22, 22, 23, 20	// 正面(-Z面)
		};

		return meshData;

	}


}