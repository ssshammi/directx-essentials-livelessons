#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	struct VertexPositionTexture;
	class Mesh;
	class Keyboard;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class AddressingModesDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(AddressingModesDemo, DrawableGameComponent)

	public:
		AddressingModesDemo(Game& game, Camera& camera);
		~AddressingModesDemo();

		AddressingModesDemo() = delete;
		AddressingModesDemo(const AddressingModesDemo& rhs) = delete;
		AddressingModesDemo& operator=(const AddressingModesDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		struct CBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;

			CBufferPerObject() : WorldViewProjection() { }

			CBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		enum class AddressingMode
		{
			Wrap = 0,
			Mirror,
			Clamp,
			Border,
			End
		};

		void CreateVertexBuffer(VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		void CreateIndexBuffer(UINT* indices, UINT indexCount, ID3D11Buffer** indexBuffer) const;

		static const std::string AddressingModeDisplayNames[];

		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11Buffer* mConstantBuffer;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;
		
		CBufferPerObject mCBufferPerObject;
		XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		ID3D11ShaderResourceView* mColorTexture;
		Keyboard* mKeyboard;
		AddressingMode mActiveAddressingMode;
		std::map<AddressingMode, ID3D11SamplerState*> mTextureSamplersByAddressingMode;
	};
}
