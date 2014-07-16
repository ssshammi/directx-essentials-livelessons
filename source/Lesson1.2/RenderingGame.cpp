#include "RenderingGame.h"
#include "GameException.h"
#include "Keyboard.h"
#include "FpsComponent.h"
#include "ColorHelper.h"

using namespace std;

namespace Rendering
{
    const XMVECTORF32 RenderingGame::BackgroundColor = ColorHelper::CornflowerBlue;

    RenderingGame::RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
		  mDirectInput(nullptr), mKeyboard(nullptr), mFpsComponent(nullptr)
    {
    }

	void RenderingGame::Initialize()
	{
		ThrowIfFailed(DirectInput8Create(mInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&mDirectInput, nullptr), "DirectInput8Create() failed");

		mKeyboard = make_unique<Keyboard>(*this, mDirectInput);
		mComponents.push_back(mKeyboard.get());
		mServices.AddService(Keyboard::TypeIdClass(), mKeyboard.get());

		mFpsComponent = make_unique<FpsComponent>(*this);
		mComponents.push_back(mFpsComponent.get());

		Game::Initialize();
	}

	void RenderingGame::Shutdown()
	{
		ReleaseObject(mDirectInput);

		Game::Shutdown();
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
		{
			Exit();
		}

		Game::Update(gameTime);
	}

    void RenderingGame::Draw(const GameTime &gameTime)
    {
        mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        Game::Draw(gameTime);

		ThrowIfFailed(mSwapChain->Present(0, 0), "IDXGISwapChain::Present() failed.");
    }
}