#pragma once
#include "Effects.h"
#include "Geometry.h"
#include "Transform.h"
/******************************************************
*		GameObject.h                                  *
*	通过该类可以管理每一个生成的物体，比如墙壁，地板和木箱，*
*	并且对object的绘制函数也移到此类					  *
*******************************************************/

class GameObject
{
public:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	GameObject();

	// 获取物体变换
	Transform& GetTransform();
	// 获取物体变换
	const Transform& GetTransform() const;

	// 设置缓冲区
	template<class VertexType, class IndexType>
	void SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);

	// 设置纹理
	void SetTexture(ID3D11ShaderResourceView* texture);

	// 设置材质
	void SetMaterial(const Material& material);

	// 绘制
	void Draw(ID3D11DeviceContext* deviceContext, BasicEffect & effect);

private:
	Transform m_Transform;                          // 世界矩阵
	Material m_Material;								// 材质
	ComPtr<ID3D11ShaderResourceView> m_pTexture;        // 纹理
	ComPtr<ID3D11Buffer> m_pVertexBuffer;               // 顶点缓冲区
	ComPtr<ID3D11Buffer> m_pIndexBuffer;                // 索引缓冲区
	UINT m_VertexStride;                                // 顶点字节大小
	UINT m_IndexCount;                                  // 索引数目   
};


/*
	用于构造顶点和索引，resetMesh被移到这里
*/
template<class VertexType, class IndexType>
void GameObject::SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData) {
	// 1.清空顶点和索引缓冲区
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	if (device == nullptr) return;

	/*****************
	 * 2 顶点缓冲区   *
	******************/
	// 2.1 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	m_VertexStride = sizeof(VertexType);
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 2.2 设定用于初始化的数据并在Device上建立缓冲区
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = meshData.vertexVec.data();
	device->CreateBuffer(&vbd, &initData, m_pVertexBuffer.GetAddressOf());

	/**************
	 *3 索引缓冲区 *
	**************/
	m_IndexCount = (UINT)meshData.indexVec.size();
	// 3.1 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = (UINT)meshData.indexVec.size() * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 3.2 建立索引缓冲区
	initData.pSysMem = meshData.indexVec.data();
	device->CreateBuffer(&ibd, &initData, m_pIndexBuffer.GetAddressOf());

}