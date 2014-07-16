#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Rendering
{
	class CubeDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(CubeDemo, DrawableGameComponent)

	public:
		CubeDemo(Game& game, Camera& camera);
		~CubeDemo();

		CubeDemo() = delete;
		CubeDemo(const CubeDemo& rhs) = delete;
		CubeDemo& operator=(const CubeDemo& rhs) = delete;

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

		static const float RotationRate;

		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11Buffer* mConstantBuffer;
		
		CBufferPerObject mCBufferPerObject;
		XMFLOAT4X4 mWorldMatrix;
	};
}
