#include "pch.h"
#include "TransparencyMappingMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "BlendStates.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(TransparencyMappingMaterial)

	TransparencyMappingMaterial::TransparencyMappingMaterial(Game& game, shared_ptr<Texture2D> colorMap, shared_ptr<Texture2D> specularMap, shared_ptr<Texture2D> transparencyMap) :
		Material(game),
		mColorMap(move(colorMap)), mSpecularMap(move(specularMap)),
		mTransparencyMap(move(transparencyMap)), mRenderStateHelper(game)
	{
	}

	com_ptr<ID3D11SamplerState> TransparencyMappingMaterial::SamplerState() const
	{
		return mSamplerState;
	}

	void TransparencyMappingMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		assert(samplerState != nullptr);
		mSamplerState = samplerState;
		Material::SetSamplerState(ShaderStages::PS, mSamplerState.get());
	}

	shared_ptr<Texture2D> TransparencyMappingMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void TransparencyMappingMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mColorMap = move(texture);
		ResetPixelShaderResources();
	}

	shared_ptr<Texture2D> TransparencyMappingMaterial::SpecularMap() const
	{
		return mSpecularMap;
	}

	void TransparencyMappingMaterial::SetSpecularMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mSpecularMap = move(texture);
		ResetPixelShaderResources();
	}

	shared_ptr<Texture2D> TransparencyMappingMaterial::TransparencyMap() const
	{
		return mTransparencyMap;
	}

	void TransparencyMappingMaterial::SetTransparencyMap(shared_ptr<Texture2D> texture)
	{
		assert(texture != nullptr);
		mTransparencyMap = move(texture);
		ResetPixelShaderResources();
	}

	const XMFLOAT4& TransparencyMappingMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void TransparencyMappingMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT3& TransparencyMappingMaterial::LightDirection() const
	{
		return mPixelCBufferPerFrameData.LightDirection;
	}

	void TransparencyMappingMaterial::SetLightDirection(const XMFLOAT3& direction)
	{
		mPixelCBufferPerFrameData.LightDirection = direction;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& TransparencyMappingMaterial::LightColor() const
	{
		return mPixelCBufferPerFrameData.LightColor;
	}

	void TransparencyMappingMaterial::SetLightColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.LightColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const DirectX::XMFLOAT3& TransparencyMappingMaterial::SpecularColor() const
	{
		return mPixelCBufferPerObjectData.SpecularColor;
	}

	void TransparencyMappingMaterial::SetSpecularColor(const DirectX::XMFLOAT3& color)
	{
		mPixelCBufferPerObjectData.SpecularColor = color;
		mPixelCBufferPerObjectDataDirty = true;
	}

	const float TransparencyMappingMaterial::SpecularPower() const
	{
		return mPixelCBufferPerObjectData.SpecularPower;
	}

	void TransparencyMappingMaterial::SetSpecularPower(float power)
	{
		mPixelCBufferPerObjectData.SpecularPower = power;
		mPixelCBufferPerObjectDataDirty = true;
	}

	const float TransparencyMappingMaterial::FogStart() const
	{
		return mVertexCBufferPerFrameData.FogStart;
	}

	void TransparencyMappingMaterial::SetFogStart(float fogStart)
	{
		mVertexCBufferPerFrameData.FogStart = fogStart;
		mVertexCBufferPerFrameDataDirty = true;		
	}

	const float TransparencyMappingMaterial::FogRange() const
	{
		return mVertexCBufferPerFrameData.FogRange;
	}

	void TransparencyMappingMaterial::SetFogRange(float fogRange)
	{
		mVertexCBufferPerFrameData.FogRange = fogRange;
		mVertexCBufferPerFrameDataDirty = true;
	}

	uint32_t TransparencyMappingMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormal);
	}

	void TransparencyMappingMaterial::Initialize()
	{
		Material::Initialize();

		auto& content = mGame->Content();
		auto vertexShader = content.Load<VertexShader>(L"Shaders\\TransparencyMappingDemoVS.cso"s);
		SetShader(vertexShader);

		auto pixelShader = content.Load<PixelShader>(L"Shaders\\TransparencyMappingDemoPS.cso");
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

	void TransparencyMappingMaterial::UpdateCameraPosition(const DirectX::XMFLOAT3& position)
	{
		mVertexCBufferPerFrameData.CameraPosition = position;
		mVertexCBufferPerFrameDataDirty = true;

		mPixelCBufferPerFrameData.CameraPosition = position;
		mPixelCBufferPerFrameDataDirty = true;		
	}

	void TransparencyMappingMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void TransparencyMappingMaterial::BeginDraw()
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

		mRenderStateHelper.SaveBlendState();
		direct3DDeviceContext->OMSetBlendState(BlendStates::AlphaBlending.get(), 0, 0xFFFFFFFF);		
	}

	void TransparencyMappingMaterial::EndDraw()
	{
		Material::EndDraw();

		mRenderStateHelper.RestoreBlendState();
	}

	void TransparencyMappingMaterial::ResetPixelShaderResources()
	{
		Material::ClearShaderResources(ShaderStages::PS);
		ID3D11ShaderResourceView* shaderResources[] = { mColorMap->ShaderResourceView().get(), mSpecularMap->ShaderResourceView().get(), mTransparencyMap->ShaderResourceView().get() };
		Material::AddShaderResources(ShaderStages::PS, shaderResources);
	}
}