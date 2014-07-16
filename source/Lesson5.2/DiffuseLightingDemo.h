#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class Mesh;
	class DirectionalLight;
	class Keyboard;
	class ProxyModel;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class DiffuseLightingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(DiffuseLightingDemo, DrawableGameComponent)

	public:
		DiffuseLightingDemo(Game& game, Camera& camera);
		~DiffuseLightingDemo();

		DiffuseLightingDemo() = delete;
		DiffuseLightingDemo(const DiffuseLightingDemo& rhs) = delete;
		DiffuseLightingDemo& operator=(const DiffuseLightingDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		struct VertexCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;
			
			VertexCBufferPerObject() : WorldViewProjection(), World() { }

			VertexCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		struct VertexCBufferPerFrame
		{
			XMFLOAT4 LightDirection;

			VertexCBufferPerFrame() : LightDirection(0.0f, 0.0f, -1.0f, 0.0f) { }

			VertexCBufferPerFrame(const XMFLOAT4& lightDirection) : LightDirection(lightDirection) { }
		};

		struct PixelCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;

			PixelCBufferPerFrame() : AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f) { }

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor) : AmbientColor(ambientColor), LightColor(lightColor) { }
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateDirectionalLight(const GameTime& gameTime);

		static const XMFLOAT2 LightRotationRate;

		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11Buffer* mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		ID3D11Buffer* mVertexCBufferPerFrame;		
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		ID3D11Buffer* mPixelCBufferPerFrame;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		UINT mIndexCount;
		ID3D11ShaderResourceView* mColorTexture;
		ID3D11SamplerState* mColorSampler;
		XMFLOAT4X4 mWorldMatrix;
		std::unique_ptr<DirectionalLight> mDirectionalLight;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;

		Keyboard* mKeyboard;
		std::unique_ptr<ProxyModel> mProxyModel;
	};
}
