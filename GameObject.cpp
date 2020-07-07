#include "GameObject.h"


GameObject::GameObject() :
	m_VertexStride(),
	m_IndexCount(),
	m_Material(){
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

// 绘制
void GameObject::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect) {
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
