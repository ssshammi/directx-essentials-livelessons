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
	class EnvironmentMappingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(EnvironmentMappingDemo, DrawableGameComponent)

	public:		
		static void* operator new(size_t size);
		static void operator delete(void *p);

		EnvironmentMappingDemo(Game& game, Camera& camera);
		~EnvironmentMappingDemo();

		EnvironmentMappingDemo() = delete;
		EnvironmentMappingDemo(const EnvironmentMappingDemo& rhs) = delete;
		EnvironmentMappingDemo& operator=(const EnvironmentMappingDemo& rhs) = delete;

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
			XMFLOAT3 CameraPosition;

			VertexCBufferPerFrame() : CameraPosition(0.0f, 0.0f, 0.0f) { }

			VertexCBufferPerFrame(const XMFLOAT3& cameraPosition) : CameraPosition(cameraPosition) { }
		};

		__declspec(align(16))
		struct PixelCBufferPerObject
		{
			float ReflectionAmount;

			PixelCBufferPerObject() : ReflectionAmount(25.0f) { }

			PixelCBufferPerObject(float reflectionAmount) : ReflectionAmount(reflectionAmount) { }
		};

		struct PixelCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 EnvColor;						

			PixelCBufferPerFrame() : AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), EnvColor(1.0f, 1.0f, 1.0f, 1.0f) { }

			PixelCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& envColor)
				: AmbientColor(ambientColor), EnvColor(envColor)
			{
			}
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
				
		static const size_t Alignment;
		static const XMFLOAT4 EnvColor;
		static const XMFLOAT4 AmbientColor;

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
		ID3D11ShaderResourceView* mEnvironmentMap;
		ID3D11SamplerState* mTrilinearSampler;
		XMFLOAT4X4 mWorldMatrix;
		float mReflectionAmount;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;

		Keyboard* mKeyboard;
	};
}
