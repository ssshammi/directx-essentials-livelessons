#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "BlinnPhongDemo.h"
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
		SamplerStates::Initialize(Direct3DDevice()); 
		RasterizerStates::Initialize(Direct3DDevice());

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

		mBlinnPhongDemo = make_shared<BlinnPhongDemo>(*this, camera);
		mComponents.push_back(mBlinnPhongDemo);

		auto imGui = make_shared<ImGuiComponent>(*this);
		mComponents.push_back(imGui);
		mServices.AddService(ImGuiComponent::TypeIdClass(), imGui.get());
		auto imGuiWndProcHandler = make_shared<UtilityWin32::WndProcHandler>(ImGui_ImplWin32_WndProcHandler);
		UtilityWin32::AddWndProcHandler(imGuiWndProcHandler);

		auto helpTextImGuiRenderBlock = make_shared<ImGuiComponent::RenderBlock>([this]()
		{
			ImGui::Begin("Controls");
			ImGui::SetNextWindowPos(ImVec2(10, 10));

			stringstream fpsLabel;
			fpsLabel << setprecision(3) << "Frame Rate: " << mFpsComponent->FrameRate() << "    Total Elapsed Time: " << mGameTime.TotalGameTimeSeconds().count();
			ImGui::Text(fpsLabel.str().c_str());

			ImGui::Text("Camera (WASD + Left-Click-Mouse-Look)");			
			ImGui::Text("Rotate Directional Light (Arrow Keys)");

			stringstream gridVisibleLabel;
			gridVisibleLabel << "Toggle Grid (G): " << (mGrid->Visible() ? "Visible" : "Not Visible");
			ImGui::Text(gridVisibleLabel.str().c_str());

			stringstream animationEnabledLabel;
			animationEnabledLabel << "Toggle Animation (Space): " << (mBlinnPhongDemo->AnimationEnabled() ? "Enabled" : "Disabled");
			ImGui::Text(animationEnabledLabel.str().c_str());

			stringstream ambientLightIntensityLabel;
			ambientLightIntensityLabel << setprecision(2) << "Ambient Light Intensity (+PgUp/-PgDown): " << mBlinnPhongDemo->AmbientLightIntensity();
			ImGui::Text(ambientLightIntensityLabel.str().c_str());

			stringstream directionalLightIntensityLabel;
			directionalLightIntensityLabel << setprecision(2) << "Directional Light Intensity (+Home/-End): " << mBlinnPhongDemo->DirectionalLightIntensity();
			ImGui::Text(directionalLightIntensityLabel.str().c_str());

			stringstream specularIntensityLabel;
			specularIntensityLabel << setprecision(2) << "Specular Intensity (+Insert/-Delete): " << mBlinnPhongDemo->SpecularIntensity();
			ImGui::Text(specularIntensityLabel.str().c_str());

			stringstream specularPowerLabel;
			specularPowerLabel << "Specular Power (+O/-P): " << mBlinnPhongDemo->SpecularPower();
			ImGui::Text(specularPowerLabel.str().c_str());

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
		
		camera->SetPosition(0.0f, 2.5f, 20.0f);
		mAmbientLightIntensity = mBlinnPhongDemo->AmbientLightIntensity();
		mDirectionalLightIntensity = mBlinnPhongDemo->DirectionalLightIntensity();
		mSpecularIntensity = mBlinnPhongDemo->SpecularIntensity();
		mSpecularPower = mBlinnPhongDemo->SpecularPower();
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

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			mBlinnPhongDemo->ToggleAnimation();
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::G))
		{
			mGrid->SetVisible(!mGrid->Visible());
		}

		UpdateAmbientLightIntensity(gameTime);
		UpdateDirectionalLight(gameTime);
		UpdateSpecularLight(gameTime);

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
		mBlinnPhongDemo = nullptr;
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
			mBlinnPhongDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::PageDown) && mAmbientLightIntensity > 0.0f)
		{
			mAmbientLightIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			mAmbientLightIntensity = max(mAmbientLightIntensity, 0.0f);
			mBlinnPhongDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
		}
	}

	void RenderingGame::UpdateDirectionalLight(const GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		// Update light intensity
		if (mKeyboard->IsKeyDown(Keys::Home) && mDirectionalLightIntensity < 1.0f)
		{
			mDirectionalLightIntensity += elapsedTime;
			mDirectionalLightIntensity = min(mDirectionalLightIntensity, 1.0f);
			mBlinnPhongDemo->SetDirectionalLightIntensity(mDirectionalLightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::End) && mDirectionalLightIntensity > 0.0f)
		{
			mDirectionalLightIntensity -= elapsedTime;
			mDirectionalLightIntensity = max(mDirectionalLightIntensity, 0.0f);
			mBlinnPhongDemo->SetDirectionalLightIntensity(mDirectionalLightIntensity);
		}

		// Rotate light
		const XMFLOAT2 LightRotationRate{ DirectX::XM_2PI, DirectX::XM_2PI };
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		if (mKeyboard->IsKeyDown(Keys::Left))
		{
			rotationAmount.x += LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(Keys::Right))
		{
			rotationAmount.x -= LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(Keys::Up))
		{
			rotationAmount.y += LightRotationRate.y * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(Keys::Down))
		{
			rotationAmount.y -= LightRotationRate.y * elapsedTime;
		}

		if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
		{
			mBlinnPhongDemo->RotateDirectionalLight(rotationAmount);
		}
	}

	void RenderingGame::UpdateSpecularLight(const Library::GameTime & gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		if (mKeyboard->IsKeyDown(Keys::Insert) && mSpecularIntensity < 1.0f)
		{
			mSpecularIntensity += elapsedTime;
			mSpecularIntensity = min(mSpecularIntensity, 1.0f);
			mBlinnPhongDemo->SetSpecularIntensity(mSpecularIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::Delete) && mSpecularIntensity > 0.0f)
		{
			mSpecularIntensity -= elapsedTime;
			mSpecularIntensity = max(mSpecularIntensity, 0.0f);
			mBlinnPhongDemo->SetSpecularIntensity(mSpecularIntensity);
		}

		const auto ModulationRate = numeric_limits<uint8_t>::max();
		if (mKeyboard->IsKeyDown(Keys::O) && mSpecularPower < numeric_limits<uint8_t>::max())
		{
			mSpecularPower += ModulationRate * elapsedTime;
			mSpecularPower = min(mSpecularPower, static_cast<float>(numeric_limits<uint8_t>::max()));
			mBlinnPhongDemo->SetSpecularPower(mSpecularPower);
		}
		else if (mKeyboard->IsKeyDown(Keys::P) && mSpecularPower > 1.0f)
		{
			mSpecularPower -= ModulationRate * elapsedTime;
			mSpecularPower = max(mSpecularPower, 1.0f);
			mBlinnPhongDemo->SetSpecularPower(mSpecularPower);
		}
	}
}