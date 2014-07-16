#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Mesh;
}

namespace Rendering
{
	class TexturedModelDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(TexturedModelDemo, DrawableGameComponent)

	public:
		TexturedModelDemo(Game& game, Camera& camera);
		~TexturedModelDemo();

		TexturedModelDemo() = delete;
		TexturedModelDemo(const TexturedModelDemo& rhs) = delete;
		TexturedModelDemo& operator=(const TexturedModelDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		struct CBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;

			CBufferPerObject() : WorldViewProjection() { }

			CBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;

		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11Buffer* mConstantBuffer;
		
		CBufferPerObject mCBufferPerObject;
		XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		ID3D11ShaderResourceView* mColorTexture;
		ID3D11SamplerState* mColorSampler;
	};
}
