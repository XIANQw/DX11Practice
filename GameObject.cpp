#include "GameObject.h"
using namespace DirectX;

GameObject::GameObject() :
	m_VertexStride(),
	m_IndexCount(),
	m_Material() {
}

// 获取物体变换
Transform& GameObject::GetTransform() {
	return m_Transform;
}
// 获取物体变换
const Transform& GameObject::GetTransform() const {
	return m_Transform;
}

// 设置纹理
void GameObject::SetTexture(ID3D11ShaderResourceView* texture) {
	m_pTexture = texture;
}

// 设置材质
void GameObject::SetMaterial(const Material& material) {
	m_Material = material;
}

BoundingBox GameObject::GetLocalBoundingBox() const {
	BoundingBox box;
	m_Model.boundingBox.Transform(box, m_Transform.GetLocalToWorldMatrixXM());
	return box;
}
BoundingBox GameObject::GetBoundingBox() const {
	return m_Model.boundingBox;
}
BoundingOrientedBox GameObject::GetBoundingOrientedBox() const {
	BoundingOrientedBox box;
	BoundingOrientedBox::CreateFromBoundingBox(box, m_Model.boundingBox);
	box.Transform(box, m_Transform.GetLocalToWorldMatrixXM());
	return box;
}

void GameObject::SetModel(Model&& model) {
	std::swap(m_Model, model);
	model.modelParts.clear();
	model.boundingBox = BoundingBox();
}

void GameObject::SetModel(const Model& model) {
	m_Model = model;
}

/*
// 绘制
void GameObject::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect, bool pad) {
	// 在上下文装配顶点缓冲区
	UINT stride = m_VertexStride;
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	// 在上下文上装配索引缓冲区
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 更新Context的drawing常量缓冲
	effect.SetWorldMatrix(m_Transform.GetLocalToWorldMatrixXM());
	effect.SetTexture(m_pTexture.Get());
	effect.SetMaterial(m_Material);
	effect.Apply(deviceContext);

	// 开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

*/

void GameObject::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect) {
	// 在上下文装配顶点缓冲区
	UINT stride = m_Model.vertexStride;
	UINT offset = 0;

	for (auto& part : m_Model.modelParts) {

		deviceContext->IASetVertexBuffers(0, 1, part.vertexBuffer.GetAddressOf(), &stride, &offset);
		// 在上下文上装配索引缓冲区
		deviceContext->IASetIndexBuffer(part.indexBuffer.Get(), part.indexFormat, 0);

		// 更新Context的drawing常量缓冲
		effect.SetWorldMatrix(m_Transform.GetLocalToWorldMatrixXM());
		effect.SetTexture(part.texDiffuse.Get());
		if (effect.isShadow) {
			effect.SetMaterial(m_Material);
		}
		else {
			effect.SetMaterial(part.material);
		}
		effect.Apply(deviceContext);

		// 开始绘制
		deviceContext->DrawIndexed(part.indexCount, 0, 0);
	}
}
