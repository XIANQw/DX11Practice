#pragma once

#include <DirectXCollision.h>
#include "Effects.h"
#include "ObjReader.h"
#include "ThridParty/Geometry.h"

/********************************************************************************************
*							Model.h															*
*		用于管理从.obj文件中读取的模型															*
*********************************************************************************************/


struct ModelPart
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	Material material;
	ComPtr<ID3D11ShaderResourceView> texDiffuse;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	UINT indexCount;
	UINT vertexCount;
	DXGI_FORMAT indexFormat;

	ModelPart() : material(), texDiffuse(), vertexBuffer(), indexBuffer(),
		indexCount(), vertexCount(), indexFormat() {};
	
	ModelPart(const ModelPart&) = default;
	ModelPart& operator=(const ModelPart&) = default;
	ModelPart(ModelPart &&) = default;
	ModelPart& operator=(ModelPart&&) = default;
};


struct Model {

	std::vector<ModelPart> modelParts;
	DirectX::BoundingBox boundingBox;
	UINT vertexStride;

	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	Model();

	Model(ID3D11Device* device, const ObjReader& model);
	// 设置缓冲区
	template<class VertexType, class IndexType>
	Model(ID3D11Device* device, 
		const Geometry::MeshData<VertexType, IndexType>& meshData);

	template<class VertexType, class IndexType>
	Model(ID3D11Device* device, 
		const std::vector<VertexType>& vertices, 
		const std::vector<IndexType>& indices);

	Model(ID3D11Device* device, 
		const void * vertices, 
		UINT vertexSize, 
		UINT vertexCount, 
		const void * indices, 
		UINT indexCount, 
		DXGI_FORMAT indexFormat);

	// 设置模型
	void SetModel(ID3D11Device* device, const ObjReader& model);
	
	// 设置网格
	template<class VertexType, class IndexType>
	void SetMesh(ID3D11Device * device, 
		const Geometry::MeshData<VertexType, IndexType>& meshData);
	
	template<class VertexType, class IndexType>
	void SetMesh(ID3D11Device* device,
		const std::vector<VertexType>& vertices,
		const std::vector<IndexType>& indices);

	void SetMesh(ID3D11Device* device,
		const void* vertices,
		UINT vertexSize,
		UINT vertexCount,
		const void* indices,
		UINT indexCount,
		DXGI_FORMAT indexFormat);

};



// 设置缓冲区
template<class VertexType, class IndexType>
Model::Model(ID3D11Device* device,
	const Geometry::MeshData<VertexType, IndexType>& meshData):modelParts(), boundingBox(), vertexStride() {
	SetMesh(device, meshData);
}

template<class VertexType, class IndexType>
Model::Model(ID3D11Device* device,
	const std::vector<VertexType>& vertices,
	const std::vector<IndexType>& indices): modelParts(), boundingBox(), vertexStride() {
	SetMesh(device, vertices, indices);
}

template<class VertexType, class IndexType>
inline void Model::SetMesh(ID3D11Device* device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
{
	static_assert(sizeof(IndexType) == 2 || sizeof(IndexType) == 4, "The size of IndexType must be 2 bytes or 4 bytes!");
	static_assert(std::is_unsigned<IndexType>::value, "IndexType must be unsigned integer!");
	SetMesh(device, vertices.data(), sizeof(VertexType),
		(UINT)vertices.size(), indices.data(), (UINT)indices.size(),
		(sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT));
}

template<class VertexType, class IndexType>
inline void Model::SetMesh(ID3D11Device* device,
	const Geometry::MeshData<VertexType, IndexType>& meshData) {
	SetMesh(device, meshData.vertexVec, meshData.indexVec);
}

