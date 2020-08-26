#include "GameObject.h"
#include "ThridParty/DXTrace.h"
using namespace DirectX;

GameObject::GameObject() :
	m_VertexStride(),
	m_IndexCount(),
	m_InstancesNum(0) {
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
	m_Model.modelParts[0].pTexDiffuse = texture;
}
void GameObject::SetNormalmap(ID3D11ShaderResourceView* normalmap) {
	m_Model.modelParts[0].pNormalmap = normalmap;
}




// 设置材质
void GameObject::SetMaterial(const Material& material) {
	for (auto& part : m_Model.modelParts)
		part.material = material;
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
	m_Model = std::move(model);
	//model.modelParts.clear();
	//model.boundingBox = BoundingBox();
}

void GameObject::SetModel(const Model& model) {
	m_Model = model;
}


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
		if (m_Model.TexDiffuseMap.count(part.texDiffuse)) {
			auto& texDiffuse = m_Model.TexDiffuseMap.find(part.texDiffuse)->second;
			effect.SetTexture(texDiffuse.Get());
		}
		else {
			effect.SetTexture(part.pTexDiffuse.Get());
		}
		if (m_Model.NormalmapMap.count(part.normalMap)) {
			auto& normalMap = m_Model.NormalmapMap.find(part.normalMap)->second;
			effect.SetNormalMap(normalMap.Get());
		}
		else {
			effect.SetNormalMap(part.pNormalmap.Get());
		}

		effect.SetMaterial(part.material);

		effect.Apply(deviceContext);

		// 开始绘制
		deviceContext->DrawIndexed(part.indexCount, 0, 0);
	}
}

void CreateInstancesBuffer(ID3D11Device* device, const std::vector<Instances>& instancesData, ID3D11Buffer** ppInstanceBuffer) {
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.ByteWidth = (UINT)sizeof(Instances) * instancesData.size();
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = instancesData.data();
	HR(device->CreateBuffer(&vbd, &InitData, ppInstanceBuffer));
}


void GameObject::DrawInstance(ID3D11DeviceContext* deviceContext, BasicEffect& effect, const std::vector<Transform>& TransformData) {
	ComPtr<ID3D11Device> device;
	deviceContext->GetDevice(device.GetAddressOf());
	std::vector<Instances> InstancesData(TransformData.size());
	for (INT32 i = 0; i < TransformData.size(); i++) {
		InstancesData[i].world = DirectX::XMMatrixTranspose(TransformData[i].GetLocalToWorldMatrixXM());
		InstancesData[i].worldInvTranspose = DirectX::XMMatrixInverse(nullptr, TransformData[i].GetLocalToWorldMatrixXM());
	}

	CreateInstancesBuffer(device.Get(), InstancesData, m_pInstancesBuffer.ReleaseAndGetAddressOf());

	// 在上下文装配顶点缓冲区
	UINT stride[2] = { m_Model.vertexStride, sizeof(Instances) };
	UINT offset[2] = { 0, 0 };

	for (auto& part : m_Model.modelParts) {
		// 使用两个VSBuffer, 0存放顶点信息，1存放InstancesData
		ID3D11Buffer* VSBuffer[2] = { part.vertexBuffer.Get(), m_pInstancesBuffer.Get() };
		deviceContext->IASetVertexBuffers(0, 2, VSBuffer, stride, offset);
		// 在上下文上装配索引缓冲区
		deviceContext->IASetIndexBuffer(part.indexBuffer.Get(), part.indexFormat, 0);

		// 更新Context的drawing常量缓冲
		if (m_Model.TexDiffuseMap.count(part.texDiffuse)) {
			auto& texDiffuse = m_Model.TexDiffuseMap.find(part.texDiffuse)->second;
			effect.SetTexture(texDiffuse.Get());
		}
		else {
			effect.SetTexture(part.pTexDiffuse.Get());
		}
		if (m_Model.TexDiffuseMap.count(part.normalMap)) {
			auto& normalMap = m_Model.NormalmapMap.find(part.normalMap)->second;
			effect.SetNormalMap(normalMap.Get());
		}
		else {
			effect.SetNormalMap(part.pNormalmap.Get());
		}
		effect.SetMaterial(part.material);

		effect.Apply(deviceContext);

		// 开始绘制
		deviceContext->DrawIndexedInstanced(part.indexCount, TransformData.size(), 0, 0, 0);
	}
}



