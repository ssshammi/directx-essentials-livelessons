#include "RenderingGame.h"
#include "GameException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "FpsComponent.h"
#include "FirstPersonCamera.h"
#include "Grid.h"
#include "RenderStateHelper.h"
#include "VectorHelper.h"
#include "ColorHelper.h"
#include "RasterizerStates.h"

#include "BlinnPhongDemo.h"

using namespace std;

namespace Rendering
{
    const XMVECTORF32 RenderingGame::BackgroundColor = ColorHelper::CornflowerBlue;

    RenderingGame::RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
		   mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mFpsComponent(nullptr), mCamera(nullptr), mRenderStateHelper(nullptr),
		   mBlinnPhongDemo(nullptr)
    {
    }

	void RenderingGame::Initialize()
	{
		ThrowIfFailed(DirectInput8Create(mInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&mDirectInput, nullptr), "DirectInput8Create() failed");

		mKeyboard = make_unique<Keyboard>(*this, mDirectInput);
		mComponents.push_back(mKeyboard.get());
		mServices.AddService(Keyboard::TypeIdClass(), mKeyboard.get());

		mMouse = make_unique<Mouse>(*this, mDirectInput);
		mComponents.push_back(mMouse.get());
		mServices.AddService(Mouse::TypeIdClass(), mMouse.get());

		mFpsComponent = make_unique<FpsComponent>(*this);
		mFpsComponent->Initialize();

		mCamera = make_unique<FirstPersonCamera>(*this);
		mComponents.push_back(mCamera.get());
		mServices.AddService(Camera::TypeIdClass(), mCamera.get());

		mGrid = make_unique<Grid>(*this, *mCamera);
		mComponents.push_back(mGrid.get());

		mRenderStateHelper = make_unique<RenderStateHelper>(*this);

		RasterizerStates::Initialize(mDirect3DDevice);

		mBlinnPhongDemo = make_unique<BlinnPhongDemo>(*this, *mCamera);
		mComponents.push_back(mBlinnPhongDemo.get());

		Game::Initialize();

		mCamera->SetPosition(0.0f, 5.0f, 20.0f);
		mCamera->ApplyRotation(XMMatrixRotationX(-XMConvertToRadians(10)));
	}

	void RenderingGame::Shutdown()
	{
		ReleaseObject(mDirectInput);
		RasterizerStates::Release();

		Game::Shutdown();
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		mFpsComponent->Update(gameTime);

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

		mRenderStateHelper->SaveAll();
		mFpsComponent->Draw(gameTime);
		mRenderStateHelper->RestoreAll();

		ThrowIfFailed(mSwapChain->Present(0, 0), "IDXGISwapChain::Present() failed.");
    }
}