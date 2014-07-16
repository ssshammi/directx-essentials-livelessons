#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"
#include "ColorHelper.h"

using namespace Library;

namespace Library
{
	class DirectionalLight;
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
	class TransparencyMappingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(TransparencyMappingDemo, DrawableGameComponent)

	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		TransparencyMappingDemo(Game& game, Camera& camera);
		~TransparencyMappingDemo();

		TransparencyMappingDemo() = delete;
		TransparencyMappingDemo(const TransparencyMappingDemo& rhs) = delete;
		TransparencyMappingDemo& operator=(const TransparencyMappingDemo& rhs) = delete;

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
			XMFLOAT4 LightDirection;
			XMFLOAT4 CameraPosition;
			float FogStart;
			float FogRange;

			VertexCBufferPerFrame()
				: LightDirection(0.0f, 0.0f, -1.0f, 0.0f), CameraPosition(0.0f, 0.0f, 0.0f, 1.0f), FogStart(20.0f), FogRange(40.0f)
			{
			}

			VertexCBufferPerFrame(const XMFLOAT4& lightDirection, const XMFLOAT4& cameraPosition, float fogStart, float fogRange)
				: LightDirection(lightDirection), CameraPosition(cameraPosition), FogStart(fogStart), FogRange(fogRange)
			{
			}
		};

		struct PixelCBufferPerObject
		{
			XMFLOAT3 SpecularColor;
			float SpecularPower;

			PixelCBufferPerObject() : SpecularColor(0.5f, 0.5f, 0.5f), SpecularPower(255.0f) { }

			PixelCBufferPerObject(const XMFLOAT3& specularColor, float specularPower)
				: SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}
		};

		struct PixelCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;
			XMFLOAT4 FogColor;
			XMFLOAT4 CameraPosition;

			PixelCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f), FogColor(&ColorHelper::CornflowerBlue[0]),
				  CameraPosition(0.0f, 0.0f, 0.0f, 1.0f)
			{
			}

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT4& fogColor, const XMFLOAT4& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor), FogColor(fogColor), CameraPosition(cameraPosition)
			{
			}
		};

		void CreateVertexBuffer(VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateDirectionalLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const size_t Alignment;
		static const XMFLOAT2 LightRotationRate;
		static const float LightModulationRate;
		static const XMFLOAT4 FogColor;

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
		ID3D11ShaderResourceView* mTransparencyMap;
		ID3D11SamplerState* mTrilinearSampler;
		XMFLOAT4X4 mWorldMatrix;
		std::unique_ptr<DirectionalLight> mDirectionalLight;
		float mFogStart;
		float mFogRange;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;

		Keyboard* mKeyboard;
		std::unique_ptr<ProxyModel> mProxyModel;
	};
}
