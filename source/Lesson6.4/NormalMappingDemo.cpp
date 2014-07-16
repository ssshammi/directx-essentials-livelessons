#include "NormalMappingDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "VertexDeclarations.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"
#include "DirectionalLight.h"
#include <WICTextureLoader.h>
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "Keyboard.h"
#include "ProxyModel.h"
#include "BlendStates.h"

namespace Rendering
{
	RTTI_DEFINITIONS(NormalMappingDemo)

	const XMFLOAT2 NormalMappingDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
	const float NormalMappingDemo::LightModulationRate = UCHAR_MAX;

	NormalMappingDemo::NormalMappingDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), mVertexBuffer(nullptr),
		  mVertexCBufferPerObject(nullptr), mVertexCBufferPerObjectData(), mVertexCBufferPerFrame(nullptr), mVertexCBufferPerFrameData(),
		  mPixelCBufferPerObject(nullptr), mPixelCBufferPerObjectData(), mPixelCBufferPerFrame(nullptr), mPixelCBufferPerFrameData(),
		  mWorldMatrix(MatrixHelper::Identity), mDirectionalLight(nullptr), mVertexCount(0), mColorTexture(nullptr),
		  mTrilinearSampler(nullptr), mRenderStateHelper(game), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f),
		  mKeyboard(nullptr), mProxyModel(nullptr),
		  mShowNormalMapping(true), mActiveNormalMap(nullptr), mRealNormalMap(nullptr), mBlankNormalMap(nullptr)
	{
	}

	NormalMappingDemo::~NormalMappingDemo()
	{
		ReleaseObject(mTrilinearSampler)
		ReleaseObject(mBlankNormalMap)
		ReleaseObject(mRealNormalMap)
		ReleaseObject(mColorTexture)
		ReleaseObject(mPixelCBufferPerFrame)
		ReleaseObject(mPixelCBufferPerObject)
		ReleaseObject(mVertexCBufferPerFrame)
		ReleaseObject(mVertexCBufferPerObject)
		ReleaseObject(mVertexBuffer)
		ReleaseObject(mPixelShader)
		ReleaseObject(mInputLayout)
		ReleaseObject(mVertexShader)
	}

	void NormalMappingDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Load a compiled vertex shader
		std::vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Effects\\VertexShader.cso", compiledVertexShader);		
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, &mVertexShader), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		std::vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Effects\\PixelShader.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, &mPixelShader), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		// Create a vertex buffer
		VertexPositionTextureNormalTangent vertices[] =
		{
			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward, Vector3Helper::Right),

			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward, Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward, Vector3Helper::Right),
		};

		mVertexCount = ARRAYSIZE(vertices);
		CreateVertexBuffer(vertices, mVertexCount, &mVertexBuffer);

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mVertexCBufferPerObject), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerFrameData);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mVertexCBufferPerFrame), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerObjectData);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mPixelCBufferPerObject), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerFrameData);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mPixelCBufferPerFrame), "ID3D11Device::CreateBuffer() failed.");

		// Load a texture
		std::wstring textureName = L"Content\\Textures\\Blocks_COLOR_RGB.png";
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTexture), "CreateWICTextureFromFile() failed.");

		textureName = L"Content\\Textures\\Blocks_NORM.png";
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mRealNormalMap), "CreateWICTextureFromFile() failed.");

		textureName = L"Content\\Textures\\DefaultNormalMap.png";
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mBlankNormalMap), "CreateWICTextureFromFile() failed.");

		mActiveNormalMap = mRealNormalMap;
	
		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		ThrowIfFailed(mGame->Direct3DDevice()->CreateSamplerState(&samplerDesc, &mTrilinearSampler), "ID3D11Device::CreateSamplerState() failed.");

		// Create text rendering helpers
		mSpriteBatch = std::make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());
		mSpriteFont = std::make_unique<SpriteFont>(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the keyboard service
		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mProxyModel = std::make_unique<ProxyModel>(*mGame, *mCamera, "Content\\Models\\DirectionalLightProxy.obj", 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mDirectionalLight = std::make_unique<DirectionalLight>(*mGame);
		const XMFLOAT3& lightdirection = mDirectionalLight->Direction();		
		mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);

		const XMFLOAT3& cameraPosition = mCamera->Position();
		mPixelCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));
	}

	void NormalMappingDemo::Update(const GameTime& gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mShowNormalMapping = !mShowNormalMapping;
			mActiveNormalMap = (mShowNormalMapping ? mRealNormalMap : mBlankNormalMap);
		}

		UpdateAmbientLight(gameTime);
		UpdateDirectionalLight(gameTime);
		UpdateSpecularLight(gameTime);
		mProxyModel->Update(gameTime);
	}

	void NormalMappingDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(VertexPositionTextureNormalTangent);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		direct3DDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));		

		const XMFLOAT3& cameraPosition = mCamera->Position();
		mPixelCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);

		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame, 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject, 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame, 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject, 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

		static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame, mVertexCBufferPerObject };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame, mPixelCBufferPerObject };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture, mActiveNormalMap };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		direct3DDeviceContext->PSSetSamplers(0, 1, &mTrilinearSampler);

		mRenderStateHelper.SaveAll();
		
		direct3DDeviceContext->OMSetBlendState(BlendStates::AlphaBlending, 0, 0xFFFFFFFF);
		direct3DDeviceContext->Draw(mVertexCount, 0);
		mRenderStateHelper.RestoreBlendState();

		mProxyModel->Draw(gameTime);

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;

		helpLabel << "Ambient Intensity (+PgUp/-PgDn): " << mPixelCBufferPerFrameData.AmbientColor.x << "\n";
		helpLabel << L"Specular Intensity (+Insert/-Delete): " << mPixelCBufferPerObjectData.SpecularColor.x << "\n";
		helpLabel << L"Specular Power (+O/-P): " << mPixelCBufferPerObjectData.SpecularPower << "\n";
		helpLabel << L"Directional Light Intensity (+Home/-End): " << mPixelCBufferPerFrameData.LightColor.x << "\n";
		helpLabel << L"Rotate Directional Light (Arrow Keys)\n";
		helpLabel << L"Use Normal Map(Space): " << (mShowNormalMapping ? "Yes" : "No") << "\n";
	
		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();
	}

	void NormalMappingDemo::CreateVertexBuffer(VertexPositionTextureNormalTangent* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTextureNormalTangent) * vertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void NormalMappingDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = 0.0f;

		if (mKeyboard->IsKeyDown(DIK_PGUP) && ambientIntensity < 1.0f)
		{
			ambientIntensity += static_cast<float>(gameTime.ElapsedGameTime());
			ambientIntensity = min(ambientIntensity, 1.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}

		if (mKeyboard->IsKeyDown(DIK_PGDN) && ambientIntensity > 0.0f)
		{
			ambientIntensity -= (float)gameTime.ElapsedGameTime();
			ambientIntensity = max(ambientIntensity, 0.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}
	}

	void NormalMappingDemo::UpdateDirectionalLight(const GameTime& gameTime)
	{
		static float directionalIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime());

		// Update directional light intensity		
		if (mKeyboard->IsKeyDown(DIK_HOME) && directionalIntensity < 1.0f)
		{
			directionalIntensity += elapsedTime;
			directionalIntensity = min(directionalIntensity, 1.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
			mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}
		if (mKeyboard->IsKeyDown(DIK_END) && directionalIntensity > 0.0f)
		{
			directionalIntensity -= elapsedTime;
			directionalIntensity = max(directionalIntensity, 0.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
			mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}

		// Rotate directional light
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		if (mKeyboard->IsKeyDown(DIK_LEFTARROW))
		{
			rotationAmount.x += LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(DIK_RIGHTARROW))
		{
			rotationAmount.x -= LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(DIK_UPARROW))
		{
			rotationAmount.y += LightRotationRate.y * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(DIK_DOWNARROW))
		{
			rotationAmount.y -= LightRotationRate.y * elapsedTime;
		}

		XMMATRIX lightRotationMatrix = XMMatrixIdentity();
		if (rotationAmount.x != 0)
		{
			lightRotationMatrix = XMMatrixRotationY(rotationAmount.x);
		}

		if (rotationAmount.y != 0)
		{
			XMMATRIX lightRotationAxisMatrix = XMMatrixRotationAxis(mDirectionalLight->RightVector(), rotationAmount.y);
			lightRotationMatrix *= lightRotationAxisMatrix;
		}

		if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
		{
			mDirectionalLight->ApplyRotation(lightRotationMatrix);			
			mProxyModel->ApplyRotation(lightRotationMatrix);

			const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
			mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);
		}
	}

	void NormalMappingDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = 0.0f;

		if (mKeyboard->IsKeyDown(DIK_INSERT) && specularIntensity < 1.0f)
		{
			specularIntensity += static_cast<float>(gameTime.ElapsedGameTime());
			specularIntensity = min(specularIntensity, 1.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		if (mKeyboard->IsKeyDown(DIK_DELETE) && specularIntensity > 0.0f)
		{
			specularIntensity -= (float)gameTime.ElapsedGameTime();
			specularIntensity = max(specularIntensity, 0.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		static float specularPower = mPixelCBufferPerObjectData.SpecularPower;

		if (mKeyboard->IsKeyDown(DIK_O) && specularPower < UCHAR_MAX)
		{
			specularPower += LightModulationRate * static_cast<float>(gameTime.ElapsedGameTime());
			specularPower = min(specularPower, static_cast<float>(UCHAR_MAX));

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}

		if (mKeyboard->IsKeyDown(DIK_P) && specularPower > 1.0f)
		{
			specularPower -= LightModulationRate * static_cast<float>(gameTime.ElapsedGameTime());
			specularPower = max(specularPower, 1.0f);

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}
	}
}