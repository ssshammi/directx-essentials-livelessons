#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "FogDemo.h"
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

		mFogDemo = make_shared<FogDemo>(*this, camera);
		mComponents.push_back(mFogDemo);

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
			ImGui::Text("Rotate Directional Light (Arrow Keys)");

			{
				stringstream gridVisibleLabel;
				gridVisibleLabel << "Toggle Grid (G): " << (mGrid->Visible() ? "Visible" : "Not Visible");
				ImGui::Text(gridVisibleLabel.str().c_str());
			}
			{
				stringstream ambientLightIntensityLabel;
				ambientLightIntensityLabel << setprecision(2) << "Ambient Light Intensity (+PgUp/-PgDown): " << mFogDemo->AmbientLightIntensity();
				ImGui::Text(ambientLightIntensityLabel.str().c_str());
			}
			{
				stringstream directionalLightIntensityLabel;
				directionalLightIntensityLabel << setprecision(2) << "Directional Light Intensity (+Home/-End): " << mFogDemo->DirectionalLightIntensity();
				ImGui::Text(directionalLightIntensityLabel.str().c_str());
			}
			{
				stringstream specularIntensityLabel;
				specularIntensityLabel << setprecision(2) << "Specular Intensity (+Insert/-Delete): " << mFogDemo->SpecularIntensity();
				ImGui::Text(specularIntensityLabel.str().c_str());
			}
			{
				stringstream specularPowerLabel;
				specularPowerLabel << "Specular Power (+O/-P): " << mFogDemo->SpecularPower();
				ImGui::Text(specularPowerLabel.str().c_str());
			}
			{
				stringstream fogStartLabel;
				fogStartLabel << "Fog Start (+K/-L): " << mFogDemo->FogStart();
				ImGui::Text(fogStartLabel.str().c_str());
			}
			{
				stringstream fogRangeLabel;
				fogRangeLabel << "Fog Range (+N/-M): " << mFogDemo->FogRange();
				ImGui::Text(fogRangeLabel.str().c_str());
			}

			ImGui::End();
		});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();
		
		camera->SetPosition(0.0f, 2.5f, 20.0f);
		mAmbientLightIntensity = mFogDemo->AmbientLightIntensity();
		mDirectionalLightIntensity = mFogDemo->DirectionalLightIntensity();
		mSpecularIntensity = mFogDemo->SpecularIntensity();
		mSpecularPower = mFogDemo->SpecularPower();
		mFogStart = mFogDemo->FogStart();
		mFogRange = mFogDemo->FogRange();
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
		UpdateDirectionalLight(gameTime);
		UpdateSpecularLight(gameTime);
		UpdateFog(gameTime);
		
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
		mFogDemo = nullptr;
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
			mFogDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::PageDown) && mAmbientLightIntensity > 0.0f)
		{
			mAmbientLightIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			mAmbientLightIntensity = max(mAmbientLightIntensity, 0.0f);
			mFogDemo->SetAmbientLightIntensity(mAmbientLightIntensity);
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
			mFogDemo->SetDirectionalLightIntensity(mDirectionalLightIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::End) && mDirectionalLightIntensity > 0.0f)
		{
			mDirectionalLightIntensity -= elapsedTime;
			mDirectionalLightIntensity = max(mDirectionalLightIntensity, 0.0f);
			mFogDemo->SetDirectionalLightIntensity(mDirectionalLightIntensity);
		}

		// Rotate light
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
			mFogDemo->RotateDirectionalLight(rotationAmount);
		}
	}

	void RenderingGame::UpdateSpecularLight(const Library::GameTime & gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		if (mKeyboard->IsKeyDown(Keys::Insert) && mSpecularIntensity < 1.0f)
		{
			mSpecularIntensity += elapsedTime;
			mSpecularIntensity = min(mSpecularIntensity, 1.0f);
			mFogDemo->SetSpecularIntensity(mSpecularIntensity);
		}
		else if (mKeyboard->IsKeyDown(Keys::Delete) && mSpecularIntensity > 0.0f)
		{
			mSpecularIntensity -= elapsedTime;
			mSpecularIntensity = max(mSpecularIntensity, 0.0f);
			mFogDemo->SetSpecularIntensity(mSpecularIntensity);
		}

		const auto ModulationRate = numeric_limits<uint8_t>::max();
		if (mKeyboard->IsKeyDown(Keys::O) && mSpecularPower < numeric_limits<uint8_t>::max())
		{
			mSpecularPower += ModulationRate * elapsedTime;
			mSpecularPower = min(mSpecularPower, static_cast<float>(numeric_limits<uint8_t>::max()));
			mFogDemo->SetSpecularPower(mSpecularPower);
		}
		else if (mKeyboard->IsKeyDown(Keys::P) && mSpecularPower > 1.0f)
		{
			mSpecularPower -= ModulationRate * elapsedTime;
			mSpecularPower = max(mSpecularPower, 1.0f);
			mFogDemo->SetSpecularPower(mSpecularPower);
		}
	}

	void RenderingGame::UpdateFog(const GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		if (mKeyboard->IsKeyDown(Keys::K))
		{
			mFogStart += elapsedTime * FogUpdateRate;
			mFogDemo->SetFogStart(mFogStart);
		}
		else if (mKeyboard->IsKeyDown(Keys::L) && mFogStart > 0.0f)
		{
			mFogStart -= elapsedTime * FogUpdateRate;
			mFogStart = max(mFogStart, 0.0f);
			mFogDemo->SetFogStart(mFogStart);
		}

		if (mKeyboard->IsKeyDown(Keys::N))
		{
			mFogRange += elapsedTime * FogUpdateRate;
			mFogDemo->SetFogRange(mFogRange);
		}
		else if (mKeyboard->IsKeyDown(Keys::M) && mFogRange > 0.0f)
		{
			mFogRange -= elapsedTime * FogUpdateRate;
			mFogRange = max(mFogRange, 0.0f);
			mFogDemo->SetFogRange(mFogRange);
		}
	}
}