#include "pch.h"
#include "NormalMappingDemo.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "ProxyModel.h"
#include "NormalMappingMaterial.h"
#include "Texture2D.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	NormalMappingDemo::NormalMappingDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	NormalMappingDemo::~NormalMappingDemo()
	{
	}

	bool NormalMappingDemo::RealNormalMapEnabled() const
	{
		return mRealNormalMapEnabled;
	}

	void NormalMappingDemo::SetRealNormalMapEnabled(bool enabled)
	{
		mRealNormalMapEnabled = enabled;
		mMaterial->SetNormalMap(mRealNormalMapEnabled ? mRealNormalMap : mDefaultNormalMap);
	}

	void NormalMappingDemo::ToggleRealNormalMap()
	{
		SetRealNormalMapEnabled(!mRealNormalMapEnabled);
	}

	float NormalMappingDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void NormalMappingDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float NormalMappingDemo::DirectionalLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void NormalMappingDemo::SetDirectionalLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const XMFLOAT3& NormalMappingDemo::LightDirection() const
	{
		return mDirectionalLight.Direction();
	}

	void NormalMappingDemo::RotateDirectionalLight(XMFLOAT2 amount)
	{
		XMMATRIX lightRotationMatrix = XMMatrixRotationY(amount.x) * XMMatrixRotationAxis(mDirectionalLight.RightVector(), amount.y);
		mDirectionalLight.ApplyRotation(lightRotationMatrix);
		mProxyModel->ApplyRotation(lightRotationMatrix);
		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());
	} 

	void NormalMappingDemo::Initialize()
	{
		const VertexPositionTextureNormalTangent sourceVertices[] =
		{
			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward, Vector3Helper::Right),

			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward, Vector3Helper::Right),
		};

		const span<const VertexPositionTextureNormalTangent> vertices{ sourceVertices };
		mVertexCount = narrow_cast<uint32_t>(vertices.size());
		VertexPositionTextureNormalTangent::CreateVertexBuffer(mGame->Direct3DDevice(), vertices, not_null<ID3D11Buffer**>(mVertexBuffer.put()));

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\Blocks_COLOR_RGB.png"s);
		mRealNormalMap = mGame->Content().Load<Texture2D>(L"Textures\\Blocks_NORM.png"s);
		mDefaultNormalMap = mGame->Content().Load<Texture2D>(L"Textures\\DefaultNormalMap.png"s);
		mMaterial = make_shared<NormalMappingMaterial>(*mGame, colorMap, mRealNormalMap);
		mMaterial->Initialize();

		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\DirectionalLightProxy.obj.bin"s, 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());
		RotateDirectionalLight(XMFLOAT2(-1.5f, 0.0f));

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));
	}

	void NormalMappingDemo::Update(const GameTime& gameTime)
	{
		mProxyModel->Update(gameTime);
	}

	void NormalMappingDemo::Draw(const GameTime& gameTime)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial->UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		mMaterial->Draw(not_null<ID3D11Buffer*>(mVertexBuffer.get()), mVertexCount);
		mProxyModel->Draw(gameTime);
	}
}