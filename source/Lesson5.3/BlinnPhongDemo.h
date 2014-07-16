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
	class BlinnPhongDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(BlinnPhongDemo, DrawableGameComponent)

	public:
		BlinnPhongDemo(Game& game, Camera& camera);
		~BlinnPhongDemo();

		BlinnPhongDemo() = delete;
		BlinnPhongDemo(const BlinnPhongDemo& rhs) = delete;
		BlinnPhongDemo& operator=(const BlinnPhongDemo& rhs) = delete;

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
			XMFLOAT4 CameraPosition;

			VertexCBufferPerFrame() : LightDirection(0.0f, 0.0f, -1.0f, 0.0f), CameraPosition(0.0f, 0.0f, 0.0f, 1.0f) { }

			VertexCBufferPerFrame(const XMFLOAT4& lightDirection, const XMFLOAT4& cameraPosition)
				: LightDirection(lightDirection), CameraPosition(cameraPosition)
			{
			}
		};

		struct PixelCBufferPerObject
		{
			XMFLOAT3 SpecularColor;
			float SpecularPower;

			PixelCBufferPerObject() : SpecularColor(1.0f, 1.0f, 1.0f), SpecularPower(25.0f) { }

			PixelCBufferPerObject(const XMFLOAT3& specularColor, float specularPower)
				: SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}
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
		void UpdateSpecularLight(const GameTime& gameTime);

		static const XMFLOAT2 LightRotationRate;
		static const float LightModulationRate;

		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11Buffer* mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		ID3D11Buffer* mVertexCBufferPerFrame;		
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		ID3D11Buffer* mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
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
