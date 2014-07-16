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
	class FilteringModesDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(FilteringModesDemo, DrawableGameComponent)

	public:
		FilteringModesDemo(Game& game, Camera& camera);
		~FilteringModesDemo();

		FilteringModesDemo() = delete;
		FilteringModesDemo(const FilteringModesDemo& rhs) = delete;
		FilteringModesDemo& operator=(const FilteringModesDemo& rhs) = delete;

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

		enum class FilteringMode
		{
			Point = 0,
			TriLinear,
			Anisotropic,
			End
		};

		void CreateVertexBuffer(VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		void CreateIndexBuffer(UINT* indices, UINT indexCount, ID3D11Buffer** indexBuffer) const;

		static const std::string FilteringModeDisplayNames[];

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
		FilteringMode mActiveFilteringMode;
		std::map<FilteringMode, ID3D11SamplerState*> mTextureSamplersByFilteringMode;
	};
}
