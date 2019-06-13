#pragma once

#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"

namespace Rendering
{
	class ColoredTriangleDemo final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(ColoredTriangleDemo, Library::DrawableGameComponent)

	public:
		ColoredTriangleDemo(Library::Game& game);

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
		winrt::com_ptr<ID3D11InputLayout> mInputLayout;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
	};
}
