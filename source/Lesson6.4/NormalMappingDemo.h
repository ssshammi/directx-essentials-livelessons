#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "DirectionalLight.h"

namespace Library
{
	class Texture2D;
	class ProxyModel;
}

namespace Rendering
{
	class NormalMappingMaterial;

	class NormalMappingDemo final : public Library::DrawableGameComponent
	{
	public:
		NormalMappingDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		NormalMappingDemo(const NormalMappingDemo&) = delete;
		NormalMappingDemo(NormalMappingDemo&&) = default;
		NormalMappingDemo& operator=(const NormalMappingDemo&) = default;		
		NormalMappingDemo& operator=(NormalMappingDemo&&) = default;
		~NormalMappingDemo();

		bool RealNormalMapEnabled() const;
		void SetRealNormalMapEnabled(bool enabled);
		void ToggleRealNormalMap();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		float DirectionalLightIntensity() const;
		void SetDirectionalLightIntensity(float intensity);

		const DirectX::XMFLOAT3& LightDirection() const;
		void RotateDirectionalLight(DirectX::XMFLOAT2 amount);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		std::shared_ptr<NormalMappingMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		std::uint32_t mVertexCount{ 0 };
		Library::DirectionalLight mDirectionalLight;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		std::shared_ptr<Library::Texture2D> mRealNormalMap;
		std::shared_ptr<Library::Texture2D> mDefaultNormalMap;
		bool mUpdateMaterial{ true };
		bool mRealNormalMapEnabled{ true };
	};
}