#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class Mesh;
	class PointLight;
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
	class PointLightDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(PointLightDemo, DrawableGameComponent)

	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		PointLightDemo(Game& game, Camera& camera);
		~PointLightDemo();

		PointLightDemo() = delete;
		PointLightDemo(const PointLightDemo& rhs) = delete;
		PointLightDemo& operator=(const PointLightDemo& rhs) = delete;

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
			XMFLOAT3 LightPosition;
			float LightRadius;

			VertexCBufferPerFrame() : LightPosition(0.0f, 0.0f, 0.0f), LightRadius(10.0f) { }

			VertexCBufferPerFrame(const XMFLOAT3& lightPosition, float lightRadius)
				: LightPosition(lightPosition), LightRadius(lightRadius)
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

		__declspec(align(16))
		struct PixelCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;
			XMFLOAT3 LightPosition;
			float Padding;
			XMFLOAT3 CameraPosition;

			PixelCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f),
				  LightPosition(0.0f, 0.0f, 0.0f), Padding(D3D11_FLOAT32_MAX), CameraPosition(0.0f, 0.0f, 0.0f)
			{
			}

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT3& lightPosition, const XMFLOAT3& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor),
				  LightPosition(lightPosition), CameraPosition(cameraPosition)
			{
			}
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);
				
		static const size_t Alignment;
		static const float LightModulationRate;
		static const float LightMovementRate;

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
		std::unique_ptr<PointLight> mPointLight;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;

		Keyboard* mKeyboard;
		std::unique_ptr<ProxyModel> mProxyModel;
	};
}
