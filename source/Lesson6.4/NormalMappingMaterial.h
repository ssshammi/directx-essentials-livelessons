#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "MatrixHelper.h"
#include "SamplerStates.h"

namespace Library
{
	class Texture2D;
}

namespace Rendering
{
	class NormalMappingMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(NormalMappingMaterial, Library::Material)

	public:
		NormalMappingMaterial(Library::Game& game, std::shared_ptr<Library::Texture2D> colorMap, std::shared_ptr<Library::Texture2D> normalMap);
		NormalMappingMaterial(const NormalMappingMaterial&) = default;
		NormalMappingMaterial& operator=(const NormalMappingMaterial&) = default;
		NormalMappingMaterial(NormalMappingMaterial&&) = default;
		NormalMappingMaterial& operator=(NormalMappingMaterial&&) = default;
		virtual ~NormalMappingMaterial() = default;

		winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);

		std::shared_ptr<Library::Texture2D> ColorMap() const;
		void SetColorMap(std::shared_ptr<Library::Texture2D> texture);

		std::shared_ptr<Library::Texture2D> NormalMap() const;
		void SetNormalMap(std::shared_ptr<Library::Texture2D> texture);

		const DirectX::XMFLOAT4& AmbientColor() const;
		void SetAmbientColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT3& LightDirection() const;
		void SetLightDirection(const DirectX::XMFLOAT3& direction);

		const DirectX::XMFLOAT4& LightColor() const;
		void SetLightColor(const DirectX::XMFLOAT4& color);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix);
		
	private:
		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT4X4 World{ Library::MatrixHelper::Identity };
		};

		struct PixelCBufferPerFrame
		{
			DirectX::XMFLOAT4 AmbientColor{ DirectX::Colors::Black };
			DirectX::XMFLOAT3 LightDirection{ 0.0f, 0.0f, 1.0f };
			float Padding;
			DirectX::XMFLOAT4 LightColor{ DirectX::Colors::White };
		};

		virtual void BeginDraw() override;

		void ResetPixelShaderResources();

		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerFrame;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		bool mPixelCBufferPerFrameDataDirty{ true };
		std::shared_ptr<Library::Texture2D> mColorMap;
		std::shared_ptr<Library::Texture2D> mNormalMap;
		winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::TrilinearClamp };
	};
}