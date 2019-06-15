#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "PointLight.h"

namespace Rendering
{
	class EnvironmentMappingMaterial;

	class EnvironmentMappingDemo final : public Library::DrawableGameComponent
	{
	public:
		EnvironmentMappingDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		EnvironmentMappingDemo(const EnvironmentMappingDemo&) = delete;
		EnvironmentMappingDemo(EnvironmentMappingDemo&&) = default;
		EnvironmentMappingDemo& operator=(const EnvironmentMappingDemo&) = default;		
		EnvironmentMappingDemo& operator=(EnvironmentMappingDemo&&) = default;
		~EnvironmentMappingDemo();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		float EnvironmentIntensity() const;
		void SetEnvironmentIntensity(float intensity);

		float ReflectionAmount() const;
		void SetReflectionAmount(float amount);

		virtual void Initialize() override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		inline static const float RotationRate{ DirectX::XM_PI };

		std::shared_ptr<EnvironmentMappingMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		std::uint32_t mIndexCount{ 0 };
		bool mUpdateMaterial{ true };
	};
}