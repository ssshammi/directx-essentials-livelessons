#pragma once

#include "Common.h"
#include "Game.h"
#include "Keyboard.h"

using namespace Library;

namespace Library
{	
	class FpsComponent;
	class Keyboard;
	class Mouse;
	class FirstPersonCamera;
	class Grid;
	class RenderStateHelper;
}

namespace Rendering
{
	class BlinnPhongDemo;

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
		std::unique_ptr<Mouse> mMouse;
		std::unique_ptr<FpsComponent> mFpsComponent;
		std::unique_ptr<FirstPersonCamera> mCamera;
		std::unique_ptr<Grid> mGrid;
		std::unique_ptr<RenderStateHelper> mRenderStateHelper;

		std::unique_ptr<BlinnPhongDemo> mBlinnPhongDemo;
    };
}
