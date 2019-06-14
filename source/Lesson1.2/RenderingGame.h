#pragma once

#include "Game.h"

namespace Library
{
	class KeyboardComponent;
}

namespace Rendering
{
	class RenderingGame : public Library::Game
	{
	public:
		RenderingGame(std::function<void* ()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

		void Exit();

	private:
		inline static const DirectX::XMVECTORF32 BackgroundColor = DirectX::Colors::CornflowerBlue;

		std::shared_ptr<Library::KeyboardComponent> mKeyboard;
	};
}