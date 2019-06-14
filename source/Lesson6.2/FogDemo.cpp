#include "pch.h"
#include "FogDemo.h"
#include "FirstPersonCamera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "FogMaterial.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "ProxyModel.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	FogDemo::FogDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	FogDemo::~FogDemo()
	{
	}

	float FogDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void FogDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float FogDemo::DirectionalLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void FogDemo::SetDirectionalLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const XMFLOAT3& FogDemo::LightDirection() const
	{
		return mDirectionalLight.Direction();
	}

	void FogDemo::RotateDirectionalLight(XMFLOAT2 amount)
	{
		XMMATRIX lightRotationMatrix = XMMatrixRotationY(amount.x) * XMMatrixRotationAxis(mDirectionalLight.RightVector(), amount.y);
		mDirectionalLight.ApplyRotation(lightRotationMatrix);
		mProxyModel->ApplyRotation(lightRotationMatrix);
		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());
	}

	float FogDemo::SpecularIntensity() const
	{
		return mMaterial->SpecularColor().x;
	}

	void FogDemo::SetSpecularIntensity(float intensity)
	{
		mMaterial->SetSpecularColor(XMFLOAT3(intensity, intensity, intensity));
	}

	float FogDemo::SpecularPower() const
	{
		return mMaterial->SpecularPower();
	}

	void FogDemo::SetSpecularPower(float power)
	{
		mMaterial->SetSpecularPower(power);
	}

	float FogDemo::FogStart() const
	{
		return mMaterial->FogStart();
	}

	void FogDemo::SetFogStart(float fogStart)
	{
		mMaterial->SetFogStart(fogStart);
	}

	float FogDemo::FogRange() const
	{
		return mMaterial->FogRange();
	}

	void FogDemo::SetFogRange(float fogRange)
	{
		mMaterial->SetFogRange(fogRange);
	}

	void FogDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();
		const auto model = mGame->Content().Load<Model>(L"Models\\Sphere.obj.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPositionTextureNormal::CreateVertexBuffer(direct3DDevice, *mesh, not_null<ID3D11Buffer**>(mVertexBuffer.put()));
		mesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(mIndexBuffer.put()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthComposite.dds"s);
		auto specularMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthSpecularMap.png"s);
		mMaterial = make_shared<FogMaterial>(*mGame, colorMap, specularMap);
		mMaterial->Initialize();

		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\DirectionalLightProxy.obj.bin"s, 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());

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

	void FogDemo::Update(const GameTime& gameTime)
	{
		mProxyModel->Update(gameTime);
	}

	void FogDemo::Draw(const GameTime& gameTime)
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