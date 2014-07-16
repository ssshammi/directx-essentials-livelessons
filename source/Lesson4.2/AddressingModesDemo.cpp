#include "AddressingModesDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "VertexDeclarations.h"
#include "ColorHelper.h"
#include "MatrixHelper.h"
#include "Camera.h"
#include <WICTextureLoader.h>
#include "Keyboard.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>

namespace Rendering
{
	RTTI_DEFINITIONS(AddressingModesDemo)

	const std::string AddressingModesDemo::AddressingModeDisplayNames[] =
	{
		"Wrap",
		"Mirror",
		"Clamp",
		"Border" 
	};

	AddressingModesDemo::AddressingModesDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), mVertexBuffer(nullptr),
		  mIndexBuffer(nullptr), mConstantBuffer(nullptr), mCBufferPerObject(), mWorldMatrix(MatrixHelper::Identity), mIndexCount(0), mColorTexture(nullptr),
		  mRenderStateHelper(game), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f),
		  mKeyboard(nullptr), mActiveAddressingMode(AddressingMode::Wrap), mTextureSamplersByAddressingMode()
	{
	}

	AddressingModesDemo::~AddressingModesDemo()
	{
		for (auto samplerPair : mTextureSamplersByAddressingMode)
		{
			ReleaseObject(samplerPair.second);
		}

		ReleaseObject(mColorTexture)
		ReleaseObject(mConstantBuffer)
		ReleaseObject(mIndexBuffer)
		ReleaseObject(mVertexBuffer)
		ReleaseObject(mPixelShader)
		ReleaseObject(mInputLayout)
		ReleaseObject(mVertexShader)
	}

	void AddressingModesDemo::Initialize()
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
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		float size = 10.0f;
		float halfSize = size / 2.0f;

		// Create a vertex buffer
		VertexPositionTexture vertices[] =
		{
			VertexPositionTexture(XMFLOAT4(-halfSize, 1.0f, 0.0, 1.0f), XMFLOAT2(0.0f, 3.0f)),
			VertexPositionTexture(XMFLOAT4(-halfSize, size + 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(halfSize, size + 1.0f, 0.0f, 1.0f), XMFLOAT2(3.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(halfSize, 1.0f, 0.0f, 1.0f), XMFLOAT2(3.0f, 3.0f))
		};

		CreateVertexBuffer(vertices, ARRAYSIZE(vertices), &mVertexBuffer);

		// Create an index buffer
		UINT indices[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		mIndexCount = ARRAYSIZE(indices);
		CreateIndexBuffer(indices, mIndexCount, &mIndexBuffer);

		// Create a constant buffer
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(CBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mConstantBuffer), "ID3D11Device::CreateBuffer() failed.");

		// Load a texture
		std::wstring textureName = L"Content\\Textures\\Cover.jpg";
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTexture), "CreateWICTextureFromFile() failed.");

		// Retrieve the keyboard service
		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		// Create texture samplers
		for (AddressingMode mode = static_cast<AddressingMode>(0); mode < AddressingMode::End; mode = AddressingMode(static_cast<int>(mode) + 1))
		{
			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

			switch (mode)
			{
				case AddressingMode::Wrap:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
					break;

				case AddressingMode::Mirror:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
					break;

				case AddressingMode::Clamp:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
					break;

				case AddressingMode::Border:
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
					memcpy(samplerDesc.BorderColor, &ColorHelper::Blue[0], sizeof(XMVECTORF32));
					break;

				default:
					throw GameException("Unsupported texture addressing mode.");
			}

			ThrowIfFailed(mGame->Direct3DDevice()->CreateSamplerState(&samplerDesc, &mTextureSamplersByAddressingMode[mode]), "ID3D11Device::CreateSamplerState() failed.");
		}

		mSpriteBatch = std::make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());
		mSpriteFont = std::make_unique<SpriteFont>(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void AddressingModesDemo::Update(const GameTime& gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			AddressingMode activeMode = AddressingMode(static_cast<int>(mActiveAddressingMode) + 1);
			if (activeMode >= AddressingMode::End)
			{
				activeMode = static_cast<AddressingMode>(0);
			}

			mActiveAddressingMode = activeMode;
		}
	}

	void AddressingModesDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(VertexPositionTexture);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		direct3DDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		wvp = XMMatrixTranspose(wvp);
		XMStoreFloat4x4(&mCBufferPerObject.WorldViewProjection, wvp);

		direct3DDeviceContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &mCBufferPerObject, 0, 0);

		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);
		direct3DDeviceContext->PSSetShaderResources(0, 1, &mColorTexture);
		direct3DDeviceContext->PSSetSamplers(0, 1, &mTextureSamplersByAddressingMode[mActiveAddressingMode]);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		mRenderStateHelper.SaveAll();

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;

		helpLabel << "Addressing Mode (Space): " << AddressingModeDisplayNames[static_cast<int>(mActiveAddressingMode)].c_str();

		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper.RestoreAll();
	}

	void AddressingModesDemo::CreateVertexBuffer(VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTexture) * vertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void AddressingModesDemo::CreateIndexBuffer(UINT* indices, UINT indexCount, ID3D11Buffer** indexBuffer) const
	{
		assert(indexBuffer != nullptr);

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(UINT) * indexCount;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = indices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, indexBuffer), "ID3D11Device::CreateBuffer() failed.");		
	}
}