#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
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
	class AmbientLightingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(AmbientLightingDemo, DrawableGameComponent)

	public:
		AmbientLightingDemo(Game& game, Camera& camera);
		~AmbientLightingDemo();

		AmbientLightingDemo() = delete;
		AmbientLightingDemo(const AmbientLightingDemo& rhs) = delete;
		AmbientLightingDemo& operator=(const AmbientLightingDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		struct CBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;

			CBufferPerObject() : WorldViewProjection() {}

			CBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		struct CBufferPerFrame
		{
			XMFLOAT4 AmbientColor; // Constant buffers must have a byte width in multiples of 16

			CBufferPerFrame() : AmbientColor(1.0f, 1.0f, 1.0f, 1.0f) { }

			CBufferPerFrame(const XMFLOAT4& ambientColor) : AmbientColor(ambientColor) { }
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const GameTime& gameTime);

		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11Buffer* mCBufferPerObject;
		CBufferPerObject mCBufferPerObjectData;
		ID3D11Buffer* mCBufferPerFrame;		
		CBufferPerFrame mCBufferPerFrameData;
		XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		ID3D11ShaderResourceView* mColorTexture;
		ID3D11SamplerState* mColorSampler;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;

		Keyboard* mKeyboard;
	};
}
