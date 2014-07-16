#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Rendering
{
	class ColoredTriangleDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ColoredTriangleDemo, DrawableGameComponent)

	public:
		ColoredTriangleDemo(Game& game, Camera& camera);
		~ColoredTriangleDemo();

		ColoredTriangleDemo() = delete;
		ColoredTriangleDemo(const ColoredTriangleDemo& rhs) = delete;
		ColoredTriangleDemo& operator=(const ColoredTriangleDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
	};
}
