#pragma once

#include <map>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"

namespace Library
{
	class KeyboardComponent;
}

namespace Rendering
{
	enum class FilteringModes
	{
		Point = 0,
		TriLinear,
		Anisotropic,
		End
	};

	class FilteringModesDemo final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(FilteringModesDemo, Library::DrawableGameComponent)

	public:
		FilteringModesDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

		FilteringModes ActiveFilteringMode() const;

		static const std::map<FilteringModes, std::string> FilteringModeNames;

	private:
		struct CBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;

			CBufferPerObject() = default;
			CBufferPerObject(const DirectX::XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		CBufferPerObject mCBufferPerObjectData;
		std::map<FilteringModes, winrt::com_ptr<ID3D11SamplerState>> mTextureSamplersByFilteringMode;
		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
		winrt::com_ptr<ID3D11InputLayout> mInputLayout;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		winrt::com_ptr<ID3D11Buffer> mCBufferPerObject;
		winrt::com_ptr<ID3D11ShaderResourceView> mColorTexture;
		std::uint32_t mIndexCount{ 0 };
		FilteringModes mActiveFilteringMode{ FilteringModes::Point };
		Library::KeyboardComponent* mKeyboard;
		bool mUpdateConstantBuffer{ true };
	};
}