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
	enum class AddressingModes
	{
		Wrap = 0,
		Mirror,
		Clamp,
		Border,
		End
	};

	class AddressingModesDemo final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(AddressingModesDemo, Library::DrawableGameComponent)

	public:
		AddressingModesDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

		AddressingModes ActiveAddressingMode() const;
		void SetActiveAddressingMode(AddressingModes mode);

		static const std::map<AddressingModes, std::string> AddressingModeNames;

	private:
		struct CBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
		};

		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		CBufferPerObject mCBufferPerObjectData;
		std::map<AddressingModes, winrt::com_ptr<ID3D11SamplerState>> mTextureSamplersByAddressingMode;
		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
		winrt::com_ptr<ID3D11InputLayout> mInputLayout;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		winrt::com_ptr<ID3D11Buffer> mCBufferPerObject;
		winrt::com_ptr<ID3D11ShaderResourceView> mColorTexture;
		std::uint32_t mIndexCount{ 0 };
		AddressingModes mActiveAddressingMode{ AddressingModes::Wrap };
		Library::KeyboardComponent* mKeyboard;
		bool mUpdateConstantBuffer{ true };
	};
}