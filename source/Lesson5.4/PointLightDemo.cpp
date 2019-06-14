#include "pch.h"
#include "PointLightDemo.h"
#include "FirstPersonCamera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "ProxyModel.h"
#include "PointLightMaterial.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	PointLightDemo::PointLightDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	PointLightDemo::~PointLightDemo()
	{
	}

	bool PointLightDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void PointLightDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void PointLightDemo::ToggleAnimation()
	{
		mAnimationEnabled = !mAnimationEnabled;
	}

	float PointLightDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void PointLightDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float PointLightDemo::PointLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void PointLightDemo::SetPointLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const XMFLOAT3& PointLightDemo::LightPosition() const
	{
		return mPointLight.Position();
	}

	const XMVECTOR PointLightDemo::LightPositionVector() const
	{
		return mPointLight.PositionVector();
	}

	void PointLightDemo::SetLightPosition(const XMFLOAT3& position)
	{
		mPointLight.SetPosition(position);
		mProxyModel->SetPosition(position);
		mMaterial->SetLightPosition(position);
	}

	void PointLightDemo::SetLightPosition(FXMVECTOR position)
	{
		mPointLight.SetPosition(position);
		mProxyModel->SetPosition(position);

		XMFLOAT3 materialPosition;
		XMStoreFloat3(&materialPosition, position);
		mMaterial->SetLightPosition(materialPosition);
	}

	float PointLightDemo::LightRadius() const
	{
		return mMaterial->LightRadius();
	}

	void PointLightDemo::SetLightRadius(float radius)
	{
		mMaterial->SetLightRadius(radius);
	}

	float PointLightDemo::SpecularIntensity() const
	{
		return mMaterial->SpecularColor().x;
	}

	void PointLightDemo::SetSpecularIntensity(float intensity)
	{
		mMaterial->SetSpecularColor(XMFLOAT3(intensity, intensity, intensity));
	}

	float PointLightDemo::SpecularPower() const
	{
		return mMaterial->SpecularPower();
	}

	void PointLightDemo::SetSpecularPower(float power)
	{
		mMaterial->SetSpecularPower(power);
	}

	void PointLightDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();
		const auto model = mGame->Content().Load<Model>(L"Models\\Sphere.obj.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPositionTextureNormal::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mVertexBuffer.put()));
		mesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(mIndexBuffer.put()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthComposite.dds"s);
		auto specularMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthSpecularMap.png"s);
		mMaterial = make_shared<PointLightMaterial>(*mGame, colorMap, specularMap);
		mMaterial->Initialize();
	
		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\PointLightProxy.obj.bin"s, 0.5f);
		mProxyModel->Initialize();		

		SetLightPosition(XMFLOAT3(1.0f, 0.0, 8.0f));

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		auto firstPersonCamera = mCamera->As<FirstPersonCamera>();
		if (firstPersonCamera != nullptr)
		{
			firstPersonCamera->AddPositionUpdatedCallback([this]() {
				mMaterial->UpdateCameraPosition(mCamera->Position());
			});
		}
	}

	void PointLightDemo::Update(const GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			mModelRotationAngle += gameTime.ElapsedGameTimeSeconds().count() * RotationRate;
			XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationY(mModelRotationAngle));			
			mUpdateMaterial = true;
		}

		mProxyModel->Update(gameTime);
	}

	void PointLightDemo::Draw(const GameTime& gameTime)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial->UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mVertexBuffer.get()), not_null<ID3D11Buffer*>(mIndexBuffer.get()), mIndexCount);
		mProxyModel->Draw(gameTime);
	}
}