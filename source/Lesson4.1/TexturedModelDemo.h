#pragma once

#include <gsl\gsl>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"

namespace Library
{
	class Mesh;
}

namespace Rendering
{
	class TexturedModelDemo final : public Library::DrawableGameComponent
	{
	public:
		TexturedModelDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		struct CBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
		};

		void CreateVertexBuffer(const Library::Mesh& mesh, gsl::not_null<ID3D11Buffer**> vertexBuffer) const;

		inline static const float RotationRate{ DirectX::XM_PI };

		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		CBufferPerObject mCBufferPerObject;
		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
		winrt::com_ptr<ID3D11InputLayout> mInputLayout;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		winrt::com_ptr<ID3D11Buffer> mConstantBuffer;
		winrt::com_ptr<ID3D11ShaderResourceView> mColorTexture;
		std::uint32_t mIndexCount{ 0 };
		float mRotationAngle{ 0.0f };
		bool mAnimationEnabled{ true };
		bool mUpdateConstantBuffer{ true };
	};
}
