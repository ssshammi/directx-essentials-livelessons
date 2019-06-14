#pragma once

#include "Game.h"
#include <DirectXColors.h>

namespace Library
{
	class KeyboardComponent;
	class GamePadComponent;
}

namespace Rendering
{
	class RenderingGame final : public Library::Game
	{
	public:
		RenderingGame(std::function<void* ()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

		void Exit();

	private:
		inline static const DirectX::XMVECTORF32 BackgroundColor{ DirectX::Colors::CornflowerBlue };

		std::shared_ptr<Library::KeyboardComponent> mKeyboard;
		std::shared_ptr<Library::GamePadComponent> mGamePad;
	};
}