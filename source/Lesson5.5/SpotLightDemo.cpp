#include "SpotLightDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "VertexDeclarations.h"
#include "ColorHelper.h"
#include "MatrixHelper.h"
#include "VectorHelper.h"
#include "Camera.h"
#include "SpotLight.h"
#include <WICTextureLoader.h>
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "Keyboard.h"
#include "ProxyModel.h"

namespace Rendering
{
	RTTI_DEFINITIONS(SpotLightDemo)

	const size_t SpotLightDemo::Alignment = 16;
	const float SpotLightDemo::LightModulationRate = UCHAR_MAX;
	const float SpotLightDemo::LightMovementRate = 10.0f;
	const XMFLOAT2 SpotLightDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);

	void* SpotLightDemo::operator new(size_t size)
	{
		#if defined(DEBUG) || defined(_DEBUG)
			return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
		#else
			return _aligned_malloc(size, Alignment);
		#endif
	}

	void SpotLightDemo::operator delete(void *p)
	{
		if (p != nullptr)
		{
			#if defined(DEBUG) || defined(_DEBUG)
				_aligned_free_dbg(p);
			#else
				_aligned_free(p);
			#endif
		}
	}

	SpotLightDemo::SpotLightDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), mVertexBuffer(nullptr),
		  mVertexCBufferPerObject(nullptr), mVertexCBufferPerObjectData(), mVertexCBufferPerFrame(nullptr), mVertexCBufferPerFrameData(),
		  mPixelCBufferPerObject(nullptr), mPixelCBufferPerObjectData(), mPixelCBufferPerFrame(nullptr), mPixelCBufferPerFrameData(),
		  mWorldMatrix(MatrixHelper::Identity), mSpotLight(nullptr), mVertexCount(0), mColorTexture(nullptr), mColorSampler(nullptr),
		  mRenderStateHelper(game), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f),
		  mKeyboard(nullptr), mProxyModel(nullptr)
	{
	}

	SpotLightDemo::~SpotLightDemo()
	{
		ReleaseObject(mColorSampler)
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

	void SpotLightDemo::Initialize()
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
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		// Create a vertex buffer
		VertexPositionTextureNormal vertices[] =
		{
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
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
		std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTexture), "CreateWICTextureFromFile() failed.");

		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		ThrowIfFailed(mGame->Direct3DDevice()->CreateSamplerState(&samplerDesc, &mColorSampler), "ID3D11Device::CreateSamplerState() failed.");

		// Create text rendering helpers
		mSpriteBatch = std::make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());
		mSpriteFont = std::make_unique<SpriteFont>(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the keyboard service
		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mProxyModel = std::make_unique<ProxyModel>(*mGame, *mCamera, "Content\\Models\\SpotLightProxy.obj", 0.5f);
		mProxyModel->Initialize();
		mProxyModel->ApplyRotation(XMMatrixRotationX(XM_PIDIV2));

		mSpotLight = std::make_unique<SpotLight>(*mGame);
		mSpotLight->SetRadius(50.0f);
		mSpotLight->SetPosition(0.0f, 5.0f, 2.0f);
		mVertexCBufferPerFrameData.LightPosition = mSpotLight->Position();
		mVertexCBufferPerFrameData.LightRadius = mSpotLight->Radius();
		mVertexCBufferPerFrameData.LightLookAt = mSpotLight->Direction();
		
		mPixelCBufferPerFrameData.LightPosition = mSpotLight->Position();
		mPixelCBufferPerFrameData.SpotLightInnerAngle = mSpotLight->InnerAngle();
		mPixelCBufferPerFrameData.SpotLightOuterAngle = mSpotLight->OuterAngle();
		mPixelCBufferPerFrameData.LightColor = ColorHelper::ToFloat4(mSpotLight->Color(), true);
		mPixelCBufferPerFrameData.CameraPosition = mCamera->Position();

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));
	}

	void SpotLightDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdateSpotLight(gameTime);
		UpdateSpecularLight(gameTime);
		mProxyModel->Update(gameTime);
	}

	void SpotLightDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(VertexPositionTextureNormal);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		direct3DDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));		

		mPixelCBufferPerFrameData.CameraPosition = mCamera->Position();

		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame, 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject, 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame, 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject, 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

		static ID3D11Buffer* VSConstantBuffers[] = { mVertexCBufferPerFrame, mVertexCBufferPerObject };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		static ID3D11Buffer* PSConstantBuffers[] = { mPixelCBufferPerFrame, mPixelCBufferPerObject };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		direct3DDeviceContext->PSSetShaderResources(0, 1, &mColorTexture);
		direct3DDeviceContext->PSSetSamplers(0, 1, &mColorSampler);

		direct3DDeviceContext->Draw(mVertexCount, 0);

		mProxyModel->Draw(gameTime);

		mRenderStateHelper.SaveAll();

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;

		helpLabel << "Ambient Intensity (+PgUp/-PgDn): " << mPixelCBufferPerFrameData.AmbientColor.x << "\n";
		helpLabel << L"Specular Intensity (+Insert/-Delete): " << mPixelCBufferPerObjectData.SpecularColor.x << "\n";
		helpLabel << L"Specular Power (+O/-P): " << mPixelCBufferPerObjectData.SpecularPower << "\n";
		helpLabel << L"Spot Light Intensity (+Home/-End): " << mPixelCBufferPerFrameData.LightColor.x << "\n";
		helpLabel << L"Move Spot Light (8/2, 4/6, 3/9)\n";
		helpLabel << L"Rotate Spot Light (Arrow Keys)\n";
		helpLabel << L"Spot Light Radius (+B/-N): " << mSpotLight->Radius() << "\n";
		helpLabel << L"Spot Light Inner Angle(+Z/-X): " << mSpotLight->InnerAngle() << "\n";
		helpLabel << L"Spot Light Outer Angle(+C/-V): " << mSpotLight->OuterAngle() << "\n";
	
		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();
	}

	void SpotLightDemo::CreateVertexBuffer(VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTextureNormal) * vertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void SpotLightDemo::UpdateAmbientLight(const GameTime& gameTime)
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

	void SpotLightDemo::UpdateSpotLight(const GameTime& gameTime)
	{
		static float spotLightIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime());

		// Update spot light intensity		
		if (mKeyboard->IsKeyDown(DIK_HOME) && spotLightIntensity < 1.0f)
		{
			spotLightIntensity += elapsedTime;
			spotLightIntensity = min(spotLightIntensity, 1.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(spotLightIntensity, spotLightIntensity, spotLightIntensity, 1.0f);
			mSpotLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}
		if (mKeyboard->IsKeyDown(DIK_END) && spotLightIntensity > 0.0f)
		{
			spotLightIntensity -= elapsedTime;
			spotLightIntensity = max(spotLightIntensity, 0.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(spotLightIntensity, spotLightIntensity, spotLightIntensity, 1.0f);
			mSpotLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}

		// Move spot light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_NUMPAD4))
			{
				movementAmount.x = -1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_NUMPAD6))
			{
				movementAmount.x = 1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_NUMPAD9))
			{
				movementAmount.y = 1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_NUMPAD3))
			{
				movementAmount.y = -1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_NUMPAD8))
			{
				movementAmount.z = -1.0f;
			}

			if (mKeyboard->IsKeyDown(DIK_NUMPAD2))
			{
				movementAmount.z = 1.0f;
			}
		}

		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mSpotLight->SetPosition(mSpotLight->PositionVector() + movement);
		mProxyModel->SetPosition(mSpotLight->Position());
		mVertexCBufferPerFrameData.LightPosition = mSpotLight->Position();
		mPixelCBufferPerFrameData.LightPosition = mSpotLight->Position();

		// Rotate spot light
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
			XMMATRIX lightRotationAxisMatrix = XMMatrixRotationAxis(mSpotLight->RightVector(), rotationAmount.y);
			lightRotationMatrix *= lightRotationAxisMatrix;
		}

		if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
		{
			mSpotLight->ApplyRotation(lightRotationMatrix);
			mProxyModel->ApplyRotation(lightRotationMatrix);
			mVertexCBufferPerFrameData.LightLookAt = mSpotLight->Direction();
		}

		// Update the light's radius
		if (mKeyboard->IsKeyDown(DIK_B))
		{
			float radius = mSpotLight->Radius() + LightModulationRate * elapsedTime;
			mSpotLight->SetRadius(radius);
			mVertexCBufferPerFrameData.LightRadius = mSpotLight->Radius();
		}

		if (mKeyboard->IsKeyDown(DIK_N))
		{
			float radius = mSpotLight->Radius() - LightModulationRate * elapsedTime;
			radius = max(radius, 0.0f);
			mSpotLight->SetRadius(radius);
			mVertexCBufferPerFrameData.LightRadius = mSpotLight->Radius();
		}
		
		// Update inner and outer angles
		static float innerAngle = mSpotLight->InnerAngle();
		if (mKeyboard->IsKeyDown(DIK_Z) && innerAngle < 1.0f)
		{
			innerAngle += elapsedTime;
			innerAngle = min(innerAngle, 1.0f);

			mSpotLight->SetInnerAngle(innerAngle);
			mPixelCBufferPerFrameData.SpotLightInnerAngle = mSpotLight->InnerAngle();
		}
		if (mKeyboard->IsKeyDown(DIK_X) && innerAngle > 0.5f)
		{
			innerAngle -= elapsedTime;
			innerAngle = max(innerAngle, 0.5f);

			mSpotLight->SetInnerAngle(innerAngle);
			mPixelCBufferPerFrameData.SpotLightInnerAngle = mSpotLight->InnerAngle();
		}

		static float outerAngle = mSpotLight->OuterAngle();
		if (mKeyboard->IsKeyDown(DIK_C) && outerAngle < 0.5f)
		{
			outerAngle += elapsedTime;
			outerAngle = min(outerAngle, 0.5f);

			mSpotLight->SetOuterAngle(outerAngle);
			mPixelCBufferPerFrameData.SpotLightOuterAngle = mSpotLight->OuterAngle();
		}
		if (mKeyboard->IsKeyDown(DIK_V) && outerAngle > 0.0f)
		{
			outerAngle -= elapsedTime;
			outerAngle = max(outerAngle, 0.0f);

			mSpotLight->SetOuterAngle(outerAngle);
			mPixelCBufferPerFrameData.SpotLightOuterAngle = mSpotLight->OuterAngle();
		}
	}

	void SpotLightDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = 1.0f;

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