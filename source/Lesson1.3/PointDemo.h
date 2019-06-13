#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"

namespace Rendering
{
	class PointDemo final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(PointDemo, DrawableGameComponent)

	public:
		PointDemo(Library::Game& game);

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
	};
}