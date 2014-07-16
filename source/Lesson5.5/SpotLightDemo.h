#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class SpotLight;
	class Keyboard;
	class ProxyModel;
	struct VertexPositionTextureNormal;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class SpotLightDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(SpotLightDemo, DrawableGameComponent)

	public:		
		static void* operator new(size_t size);
		static void operator delete(void *p);

		SpotLightDemo(Game& game, Camera& camera);
		~SpotLightDemo();

		SpotLightDemo() = delete;
		SpotLightDemo(const SpotLightDemo& rhs) = delete;
		SpotLightDemo& operator=(const SpotLightDemo& rhs) = delete;

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

		__declspec(align(16))
		struct VertexCBufferPerFrame
		{
			XMFLOAT3 LightPosition;
			float LightRadius;
			XMFLOAT3 LightLookAt;

			VertexCBufferPerFrame()
				: LightPosition(0.0f, 0.0f, 0.0f), LightRadius(10.0f), LightLookAt(0.0f, 0.0f, -1.0f)
			{
			}

			VertexCBufferPerFrame(const XMFLOAT3& lightPosition, float lightRadius, const XMFLOAT3& lightLookAt)
				: LightPosition(lightPosition), LightRadius(lightRadius), LightLookAt(lightLookAt)
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
			XMFLOAT3 LightPosition;
			float SpotLightInnerAngle;
			float SpotLightOuterAngle;
			XMFLOAT3 CameraPosition;

			PixelCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f), LightPosition(0.0f, 0.0f, 0.0f),
				  SpotLightInnerAngle(0.75f), SpotLightOuterAngle(0.25f), CameraPosition(0.0f, 0.0f, 0.0f)
			{
			}

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT3& lightPosition, float spotLightInnerAngle, float spotLightOuterAngle, const XMFLOAT3& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor), LightPosition(lightPosition),
				  SpotLightInnerAngle(spotLightInnerAngle), SpotLightOuterAngle(spotLightOuterAngle), CameraPosition(cameraPosition)
			{
			}
		};

		void CreateVertexBuffer(VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateSpotLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);
				
		static const size_t Alignment;
		static const float LightModulationRate;
		static const float LightMovementRate;
		static const XMFLOAT2 LightRotationRate;

		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		ID3D11Buffer* mVertexCBufferPerFrame;		
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		ID3D11Buffer* mPixelCBufferPerObject;
		PixelCBufferPerObject mPixelCBufferPerObjectData;
		ID3D11Buffer* mPixelCBufferPerFrame;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		UINT mVertexCount;
		ID3D11ShaderResourceView* mColorTexture;
		ID3D11SamplerState* mColorSampler;
		XMFLOAT4X4 mWorldMatrix;
		std::unique_ptr<SpotLight> mSpotLight;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;

		Keyboard* mKeyboard;
		std::unique_ptr<ProxyModel> mProxyModel;
	};
}
