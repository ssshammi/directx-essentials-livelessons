#pragma once

#include "Common.h"
#include "Game.h"

using namespace Library;

namespace Library
{
	class FpsComponent;
	class Keyboard;
}

namespace Rendering
{
    class RenderingGame : public Game
    {
    public:
        RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;

	protected:
		virtual void Shutdown() override;

    private:
        static const XMVECTORF32 BackgroundColor;

		LPDIRECTINPUT8 mDirectInput;
		std::unique_ptr<Keyboard> mKeyboard;
		std::unique_ptr<FpsComponent> mFpsComponent;
    };
}
