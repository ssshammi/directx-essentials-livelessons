#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "SpotLightDemo.h"
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

		mSpotLightDemo = make_shared<SpotLightDemo>(*this, camera);
		mComponents.push_back(mSpotLightDemo);

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
			ImGui::Text("Move Spot Light (Num-Pad 8/2, 4/6, 3/9)");

			AddImGuiTextField("Toggle Grid (G): "s, (mGrid->Visible() ? "Visible"s : "Not Visible"s));
			AddImGuiTextField("Ambient Light Intensity (+PgUp/-PgDown): "s, mSpotLightDemo->AmbientLightIntensity(), 2);
			AddImGuiTextField("Spot Light Intensity (+Home/-End): "s, mSpotLightDemo->SpotLightIntensity(), 2);
			AddImGuiTextField("Specular Intensity (+Insert/-Delete): "s, mSpotLightDemo->SpecularIntensity(), 2);
			AddImGuiTextField("Specular Power (+O/-P): "s, mSpotLightDemo->SpecularPower());						
			AddImGuiTextField("Spot Light Radius (+B/-N): "s, mSpotLightDemo->LightRadius());
			AddImGuiTextField("Spot Light Inner Angle(+Z/-X): "s, mSpotLightDemo->SpotLightInnerAngle(), 2);
			AddImGuiTextField("Spot Light Outer Angle(+C/-V): "s, mSpotLightDemo->SpotLightOuterAngle(), 2);

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
		
		camera->SetPosition(0.0f, 5.0f, 20.0f);
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
		UpdateSpotLight(gameTime);
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
		mSpotLightDemo = nullptr;
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
		const float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		float ambientIntensity = mSpotLightDemo->AmbientLightIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::PageUp, Keys::PageDown, ambientIntensity, elapsedTime, [&](const float& ambientIntensity)
		{
			mSpotLightDemo->SetAmbientLightIntensity(ambientIntensity);
		}, 0.0f, 1.0f);
	}

	void RenderingGame::UpdateSpotLight(const GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		// Update light intensity
		{
			float spotLightIntensity = mSpotLightDemo->SpotLightIntensity();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Home, Keys::End, spotLightIntensity, elapsedTime, [&](const float& spotLightIntensity)
			{
				mSpotLightDemo->SetSpotLightIntensity(spotLightIntensity);
			}, 0.0f, 1.0f);

			// Move light
			bool updatePosition = false;
			auto updatePositionFunc = [&updatePosition](const float&) { updatePosition = true; };
			const float MovementRate = 10.0f * elapsedTime;
			XMFLOAT3 movementAmount = Vector3Helper::Zero;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad6, Keys::NumPad4, movementAmount.x, MovementRate, updatePositionFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad9, Keys::NumPad3, movementAmount.y, MovementRate, updatePositionFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::NumPad2, Keys::NumPad8, movementAmount.z, MovementRate, updatePositionFunc);

			if (updatePosition)
			{
				mSpotLightDemo->SetLightPosition(mSpotLightDemo->LightPositionVector() + XMLoadFloat3(&movementAmount));
			}
		}

		// Rotate light		
		{
			bool updateRotation = false;
			auto updateRotationFunc = [&updateRotation](const float&) { updateRotation = true; };
			const float RotationRate = XM_2PI * elapsedTime;
			XMFLOAT2 rotationAmount = Vector2Helper::Zero;
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Left, Keys::Right, rotationAmount.x, RotationRate, updateRotationFunc);
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Up, Keys::Down, rotationAmount.y, RotationRate, updateRotationFunc);

			if (updateRotation)
			{
				mSpotLightDemo->RotateSpotLight(rotationAmount);
			}
		}

		// Update the light's radius
		{
			const float LightModulationRate = static_cast<float>(numeric_limits<uint8_t>::max()) * elapsedTime;
			auto lightRadius = mSpotLightDemo->LightRadius();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::B, Keys::N, lightRadius, LightModulationRate, [&](const float& lightRadius)
			{
				mSpotLightDemo->SetLightRadius(lightRadius);
			}, 0.0f);
		}

		// Update inner and outer angles
		{
			auto spotLightInnerAngle = mSpotLightDemo->SpotLightInnerAngle();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Z, Keys::X, spotLightInnerAngle, elapsedTime, [&](const float& spotLightInnerAngle)
			{
				mSpotLightDemo->SetSpotLightInnerAngle(spotLightInnerAngle);
			}, 0.5f, 1.0f);

			auto spotLightOuterAngle = mSpotLightDemo->SpotLightOuterAngle();
			UpdateValueWithKeyboard<float>(*mKeyboard, Keys::C, Keys::V, spotLightOuterAngle, elapsedTime, [&](const float& spotLightOuterAngle)
			{
				mSpotLightDemo->SetSpotLightOuterAngle(spotLightOuterAngle);
			}, 0.0f, 0.5f);
		}
	}

	void RenderingGame::UpdateSpecularLight(const Library::GameTime & gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		
		auto specularIntensity = mSpotLightDemo->SpecularIntensity();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::Insert, Keys::Delete, specularIntensity, elapsedTime, [&](const float& specularIntensity)
		{
			mSpotLightDemo->SetSpecularIntensity(specularIntensity);
		}, 0.0f, 1.0f);

		const auto ModulationRate = numeric_limits<uint8_t>::max() * elapsedTime;
		auto specularPower = mSpotLightDemo->SpecularPower();
		UpdateValueWithKeyboard<float>(*mKeyboard, Keys::O, Keys::P, specularPower, ModulationRate, [&](const float& specularPower)
		{
			mSpotLightDemo->SetSpecularPower(specularPower);
		}, 0.1f, numeric_limits<uint8_t>::max());
	}
}