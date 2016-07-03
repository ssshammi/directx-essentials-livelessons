#pragma once

#include <wrl.h>
#include <d3d11_2.h>
#include "DrawableGameComponent.h"

namespace Rendering
{
	class PointDemo final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(PointDemo, DrawableGameComponent)

	public:
		PointDemo(Library::Game& game);
		PointDemo(const PointDemo&) = default;
		PointDemo& operator=(const PointDemo&) = default;
		PointDemo(PointDemo&&) = default;
		PointDemo& operator=(PointDemo&&) = default;
		~PointDemo() = default;

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
	};
}
