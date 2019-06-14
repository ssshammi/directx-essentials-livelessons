#include "pch.h"
#include "FogMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Texture2D.h"
#include "TextureCube.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(FogMaterial)

	FogMaterial::FogMaterial(Game& game, shared_ptr<Texture2D> colorMap, shared_ptr<Texture2D> specularMap) :
		Material(game), mColorMap(move(colorMap)), mSpecularMap(move(specularMap))
	{
	}

	com_ptr<ID3D11SamplerState> FogMaterial::SamplerState() const
	{
		return mSamplerState;
	}

	void FogMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		assert(samplerState != nullptr);
		mSamplerState = samplerState;
		Material::SetSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	shared_ptr<Texture2D> FogMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void FogMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mColorMap = move(texture);
		ResetPixelShaderResources();
	}

	shared_ptr<Texture2D> FogMaterial::SpecularMap() const
	{
		return mSpecularMap;
	}

	void FogMaterial::SetSpecularMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mSpecularMap = move(texture);
		ResetPixelShaderResources();
	}

	const XMFLOAT4& FogMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void FogMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT3& FogMaterial::LightDirection() const
	{
		return mPixelCBufferPerFrameData.LightDirection;
	}

	void FogMaterial::SetLightDirection(const XMFLOAT3& direction)
	{
		mPixelCBufferPerFrameData.LightDirection = direction;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& FogMaterial::LightColor() const
	{
		return mPixelCBufferPerFrameData.LightColor;
	}

	void FogMaterial::SetLightColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.LightColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const DirectX::XMFLOAT3& FogMaterial::SpecularColor() const
	{
		return mPixelCBufferPerObjectData.SpecularColor;
	}

	void FogMaterial::SetSpecularColor(const DirectX::XMFLOAT3& color)
	{
		mPixelCBufferPerObjectData.SpecularColor = color;
		mPixelCBufferPerObjectDataDirty = true;
	}

	const float FogMaterial::SpecularPower() const
	{
		return mPixelCBufferPerObjectData.SpecularPower;
	}

	void FogMaterial::SetSpecularPower(float power)
	{
		mPixelCBufferPerObjectData.SpecularPower = power;
		mPixelCBufferPerObjectDataDirty = true;
	}

	const float FogMaterial::FogStart() const
	{
		return mVertexCBufferPerFrameData.FogStart;
	}

	void FogMaterial::SetFogStart(float fogStart)
	{
		mVertexCBufferPerFrameData.FogStart = fogStart;
		mVertexCBufferPerFrameDataDirty = true;		
	}

	const float FogMaterial::FogRange() const
	{
		return mVertexCBufferPerFrameData.FogRange;
	}

	void FogMaterial::SetFogRange(float fogRange)
	{
		mVertexCBufferPerFrameData.FogRange = fogRange;
		mVertexCBufferPerFrameDataDirty = true;
	}

	uint32_t FogMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormal);
	}

	void FogMaterial::Initialize()
	{
		Material::Initialize();

		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\FogDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\FogDemoPS.cso");
		SetShader(pixelShader);

		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPositionTextureNormal>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerFrame.get());

		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVertexCBufferPerObject.get());

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::PS, mPixelCBufferPerFrame.get());

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerObject);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::PS, mPixelCBufferPerObject.get());

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);
	
		ResetPixelShaderResources();
		AddSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	void FogMaterial::UpdateCameraPosition(const DirectX::XMFLOAT3& position)
	{
		mVertexCBufferPerFrameData.CameraPosition = position;
		mVertexCBufferPerFrameDataDirty = true;

		mPixelCBufferPerFrameData.CameraPosition = position;
		mPixelCBufferPerFrameDataDirty = true;		
	}

	void FogMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void FogMaterial::BeginDraw()
	{
		Material::BeginDraw();

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();

		if (mVertexCBufferPerFrameDataDirty)
		{
			mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerFrame.get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
			mVertexCBufferPerFrameDataDirty = false;
		}

		if (mPixelCBufferPerFrameDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
			mPixelCBufferPerFrameDataDirty = false;
		}

		if (mPixelCBufferPerObjectDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);
			mPixelCBufferPerObjectDataDirty = false;
		}
	}

	void FogMaterial::ResetPixelShaderResources()
	{
		Material::ClearShaderResources(ShaderStages::PS);
		ID3D11ShaderResourceView* shaderResources[] = { mColorMap->ShaderResourceView().get(), mSpecularMap->ShaderResourceView().get() };
		Material::AddShaderResources(ShaderStages::PS, shaderResources);
	}
}