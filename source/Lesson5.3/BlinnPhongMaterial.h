#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "SamplerStates.h"

namespace Library
{
	class Texture2D;
}

namespace Rendering
{
	class BlinnPhongMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(BlinnPhongMaterial, Library::Material)

	public:
		BlinnPhongMaterial(Library::Game& game, std::shared_ptr<Library::Texture2D> texture);
		BlinnPhongMaterial(const BlinnPhongMaterial&) = default;
		BlinnPhongMaterial& operator=(const BlinnPhongMaterial&) = default;
		BlinnPhongMaterial(BlinnPhongMaterial&&) = default;
		BlinnPhongMaterial& operator=(BlinnPhongMaterial&&) = default;
		virtual ~BlinnPhongMaterial() = default;

		winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);

		std::shared_ptr<Library::Texture2D> Texture() const;
		void SetTexture(std::shared_ptr<Library::Texture2D> texture);

		const DirectX::XMFLOAT4& AmbientColor() const;
		void SetAmbientColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT3& LightDirection() const;
		void SetLightDirection(const DirectX::XMFLOAT3& direction);

		const DirectX::XMFLOAT4& LightColor() const;
		void SetLightColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT3& SpecularColor() const;
		void SetSpecularColor(const DirectX::XMFLOAT3& color);

		const float SpecularPower() const;
		void SetSpecularPower(float power);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateCameraPosition(const DirectX::XMFLOAT3& position);
		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix);
		
	private:
		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT4X4 World{ Library::MatrixHelper::Identity };
		};

		struct PixelCBufferPerFrame
		{
			DirectX::XMFLOAT3 CameraPosition{ Library::Vector3Helper::Zero };
			float Padding;
			DirectX::XMFLOAT4 AmbientColor{ DirectX::Colors::Black };
			DirectX::XMFLOAT3 LightDirection{ 0.0f, 0.0f, 1.0f };
			float Padding2;
			DirectX::XMFLOAT4 LightColor{ DirectX::Colors::White };
		};

		struct PixelCBufferPerObject
		{
			DirectX::XMFLOAT3 SpecularColor{ 1.0f, 1.0f, 1.0f };
			float SpecularPower{ 128.0f };
		};

		virtual void BeginDraw() override;

		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		bool mPixelCBufferPerFrameDataDirty{ true };
		bool mPixelCBufferPerObjectDataDirty{ true };
		std::shared_ptr<Library::Texture2D> mTexture;
		winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::TrilinearClamp };
	};
}