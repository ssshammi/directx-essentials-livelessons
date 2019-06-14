#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "EnvironmentMappingDemo.h"
#include "Grid.h"
#include "FirstPersonCamera.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "UtilityWin32.h"
#include <limits>

using namespace std;
using namespace DirectX;
using namespace Library;

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Rendering
{
	RenderingGame::RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback) :
		Game(getWindowCallback, getRenderTargetSizeCallback)
	{
	}

	void RenderingGame::Initialize()
	{
		auto direct3DDevice = Direct3DDevice();
		SamplerStates::Initialize(direct3DDevice);
		RasterizerStates::Initialize(direct3DDevice);

		mKeyboard = make_shared<KeyboardComponent>(*this);
		mComponents.push_back(mKeyboard);
		mServices.AddService(KeyboardComponent::TypeIdClass(), mKeyboard.get());

		mMouse = make_shared<MouseComponent>(*this, MouseModes::Absolute);
		mComponents.push_back(mMouse);
		mServices.AddService(MouseComponent::TypeIdClass(), mMouse.get());

		mGamePad = make_shared<GamePadComponent>(*this);
		mComponents.push_back(mGamePad);
		mServices.AddService(GamePadComponent::TypeIdClass(), mGamePad.get());

		auto camera = make_shared<FirstPersonCamera>(*this);
		mComponents.push_back(camera);
		mServices.AddService(Camera::TypeIdClass(), camera.get());

		mGrid = make_shared<Grid>(*this, camera);
		mComponents.push_back(mGrid);

		mEnvironmentMappingDemo = make_shared<EnvironmentMappingDemo>(*this, camera);
		mComponents.push_back(mEnvironmentMappingDemo);

		auto imGui = make_shared<ImGuiComponent>(*this);
		mComponents.push_back(imGui);
		mServices.AddService(ImGuiComponent::TypeIdClass(), imGui.get());
		auto imGuiWndProcHandler = make_shared<UtilityWin32::WndProcHandler>(ImGui_ImplWin32_WndProcHandler);
		UtilityWin32::AddWndProcHandler(imGuiWndProcHandler);

		auto helpTextImGuiRenderBlock = make_shared<ImGuiComponent::RenderBlock>([this]()
		{
			ImGui::Begin("Controls");
			ImGui::SetNextWindowPos(ImVec2(10, 10));

			{
				stringstream fpsLabel;
				fpsLabel << setprecision(3) << "Frame Rate: " << mFpsComponent->FrameRate() << "    Total Elapsed Time: " << mGameTime.TotalGameTimeSeconds().count();
				ImGui::Text(fpsLabel.str().c_str());
			}

			ImGui::Text("Camera (WASD + Left-Click-Mouse-Look)");
			ImGui::Text("Move Point Light (Num-Pad 8/2, 4/6, 3/9)");			

			{
				stringstream gridVisibleLabel;
				gridVisibleLabel << "Toggle Grid (G): " << (mGrid->Visible() ? "Visible" : "Not Visible");
				ImGui::Text(gridVisibleLabel.str().c_str());
			}
			{
				stringstream ambientLightIntensityLabel;
				ambientLightIntensityLabel << setprecision(2) << "Ambient Light Intensity (+PgUp/-PgDown): " << mEnvironmentMappingDemo->AmbientLightIntensity();
				ImGui::Text(ambientLightIntensityLabel.str().c_str());
			}
			{
				stringstream environmentIntensityLabel;
				environmentIntensityLabel << setprecision(2) << "Environment Intensity (+Insert/-Delete): " << mEnvironmentMappingDemo->EnvironmentIntensity();
				ImGui::Text(environmentIntensityLabel.str().c_str());
			}
			{
				stringstream reflectionAmountLabel;
				reflectionAmountLabel << setprecision(2) << "Reflection Amount (+Up/-Down): " << mEnvironmentMappingDemo->ReflectionAmount();
				ImGui::Text(reflectionAmountLabel.str().c_str());
			}

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
		
		camera->SetPosition(0.0f, 2.5f, 20.0f);
		mAmbientLightIntensity = mEnvironmentMappingDemo->AmbientLightIntensity();
		mEnvironmentIntensity = mEnvironmentMappingDemo->EnvironmentIntensity();
		mReflectionAmount = mEnvironmentMappingDemo->ReflectionAmount();
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) || mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back))
		{
			Exit();
		}

		if (mMouse->WasButtonPressedThisFrame(MouseButtons::Left))
		{
			mMouse->SetMode(MouseModes::Relative);
		}

		if (mMouse->WasButtonReleasedThisFrame(MouseButtons::Left))
		{
			mMouse->SetMode(MouseModes::Absolute);
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::G))
		{
			mGrid->SetVisible(!mGrid->Visible());
		}

		UpdateAmbientLightIntensity(gameTime);
		UpdateEnvironmentIntensity(gameTime);
		UpdateReflectionAmount(gameTime);

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView.get(), BackgroundColor.f);
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

		HRESULT hr = mSwapChain->Present(1, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HandleDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr, "IDXGISwapChain::Present() failed.");
		}
	}

	void RenderingGame::Shutdown()
	{
		mGrid = nullptr;
		mFpsComponent = nullptr;
		mEnvironmentMappingDemo = nullptr;
		RasterizerStates::Shutdown();
		SamplerStates::Shutdown();
		Game::Shutdown();
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}

	void RenderingGame::UpdateAmbientLightIntensity(const GameTime& gameTime)
	{
		if (mKeyboard->IsKeyDown(Keys::PageUp) && mAmbientLightIntensity < 1.0f)
		{
			mAmbientLightIntensity += gameTime.ElapsedGameTimeSeconds().count();
			mAmbientLightIntensity = min(mAmbientLightIntensity, 1.0f);
			mEnvironmentMappingDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::PageDown) && mAmbientLightIntensity > 0.0f)
		{
			mAmbientLightIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			mAmbientLightIntensity = max(mAmbientLightIntensity, 0.0f);
			mEnvironmentMappingDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
		}
	}

	void RenderingGame::UpdateEnvironmentIntensity(const GameTime& gameTime)
	{
		if (mKeyboard->IsKeyDown(Keys::Insert) && mEnvironmentIntensity < 1.0f)
		{
			mEnvironmentIntensity += gameTime.ElapsedGameTimeSeconds().count();
			mEnvironmentIntensity = min(mEnvironmentIntensity, 1.0f);
			mEnvironmentMappingDemo->SetEnvironmentIntensity(mEnvironmentIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::Delete) && mEnvironmentIntensity > 0.0f)
		{
			mEnvironmentIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			mEnvironmentIntensity = max(mEnvironmentIntensity, 0.0f);
			mEnvironmentMappingDemo->SetEnvironmentIntensity(mEnvironmentIntensity);
		}
	}

	void RenderingGame::UpdateReflectionAmount(const GameTime& gameTime)
	{
		if (mKeyboard->IsKeyDown(Keys::Up) && mReflectionAmount < 1.0f)
		{
			mReflectionAmount += gameTime.ElapsedGameTimeSeconds().count();
			mReflectionAmount = min(mReflectionAmount, 1.0f);
			mEnvironmentMappingDemo->SetReflectionAmount(mReflectionAmount);
		}
		else if (mKeyboard->IsKeyDown(Keys::Down) && mReflectionAmount > 0.0f)
		{
			mReflectionAmount -= gameTime.ElapsedGameTimeSeconds().count();
			mReflectionAmount = max(mReflectionAmount, 0.0f);
			mEnvironmentMappingDemo->SetReflectionAmount(mReflectionAmount);
		}
	}
}