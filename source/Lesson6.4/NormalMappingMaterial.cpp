#include "pch.h"
#include "NormalMappingMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(NormalMappingMaterial)

	NormalMappingMaterial::NormalMappingMaterial(Game& game, shared_ptr<Texture2D> colorMap, shared_ptr<Texture2D> normalMap) :
		Material(game), mColorMap(move(colorMap)), mNormalMap(move(normalMap))
	{
	}

	com_ptr<ID3D11SamplerState> NormalMappingMaterial::SamplerState() const
	{
		return mSamplerState;
	}

	void NormalMappingMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		assert(samplerState != nullptr);
		mSamplerState = samplerState;
		Material::SetSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	shared_ptr<Texture2D> NormalMappingMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void NormalMappingMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mColorMap = move(texture);
		ResetPixelShaderResources();
	}

	shared_ptr<Texture2D> NormalMappingMaterial::NormalMap() const
	{
		return mNormalMap;
	}

	void NormalMappingMaterial::SetNormalMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mNormalMap = move(texture);
		ResetPixelShaderResources();
	}

	const XMFLOAT4& NormalMappingMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void NormalMappingMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT3& NormalMappingMaterial::LightDirection() const
	{
		return mPixelCBufferPerFrameData.LightDirection;
	}

	void NormalMappingMaterial::SetLightDirection(const XMFLOAT3& direction)
	{
		mPixelCBufferPerFrameData.LightDirection = direction;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& NormalMappingMaterial::LightColor() const
	{
		return mPixelCBufferPerFrameData.LightColor;
	}

	void NormalMappingMaterial::SetLightColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.LightColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	uint32_t NormalMappingMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormalTangent);
	}

	void NormalMappingMaterial::Initialize()
	{
		Material::Initialize();

		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\NormalMappingDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\NormalMappingDemoPS.cso");
		SetShader(pixelShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPositionTextureNormalTangent>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::PS, mPixelCBufferPerFrame.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
	
		ResetPixelShaderResources();
		AddSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	void NormalMappingMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void NormalMappingMaterial::BeginDraw()
	{
		Material::BeginDraw();

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();

		if (mPixelCBufferPerFrameDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
			mPixelCBufferPerFrameDataDirty = false;
		}
	}

	void NormalMappingMaterial::ResetPixelShaderResources()
	{
		Material::ClearShaderResources(ShaderStages::PS);
		ID3D11ShaderResourceView* shaderResources[] = { mColorMap->ShaderResourceView().get(), mNormalMap->ShaderResourceView().get() };
		Material::AddShaderResources(ShaderStages::PS, shaderResources);
	}
}