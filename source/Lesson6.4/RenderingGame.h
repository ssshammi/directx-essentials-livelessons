#pragma once

#include "Game.h"

namespace Library
{
	class KeyboardComponent;
	class MouseComponent;
	class GamePadComponent;
	class FpsComponent;
	class Grid;
}

namespace Rendering
{
	class NormalMappingDemo;

	class RenderingGame final : public Library::Game
	{
	public:
		RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;
		virtual void Shutdown() override;

		void Exit();

	private:
		inline static const DirectX::XMVECTORF32 BackgroundColor{ DirectX::Colors::CornflowerBlue };
		inline static const DirectX::XMFLOAT2 LightRotationRate{ DirectX::XMFLOAT2(DirectX::XM_2PI, DirectX::XM_2PI) };

		void UpdateAmbientLightIntensity(const Library::GameTime& gameTime);
		void UpdateDirectionalLight(const Library::GameTime& gameTime);

		std::shared_ptr<Library::KeyboardComponent> mKeyboard;
		std::shared_ptr<Library::MouseComponent> mMouse;
		std::shared_ptr<Library::GamePadComponent> mGamePad;
		std::shared_ptr<Library::FpsComponent> mFpsComponent;
		std::shared_ptr<Library::Grid> mGrid;
		std::shared_ptr<NormalMappingDemo> mNormalMappingDemo;
		float mAmbientLightIntensity{ 0.0f };
		float mDirectionalLightIntensity{ 0.0f };
	};
}