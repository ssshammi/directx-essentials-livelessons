#include "pch.h"
#include "FilteringModesDemo.h"
#include "Utility.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "KeyboardComponent.h"
#include "Camera.h"
#include "DirectXHelper.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	RTTI_DEFINITIONS(FilteringModesDemo)

	const map<FilteringModes, string> FilteringModesDemo::FilteringModeNames
	{
		{ FilteringModes::Point, "Point"s },
		{ FilteringModes::TriLinear, "Tri-Linear"s },
		{ FilteringModes::Anisotropic, "Anisotropic"s }
	};

	FilteringModesDemo::FilteringModesDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	FilteringModes FilteringModesDemo::ActiveFilteringMode() const
	{
		return mActiveFilteringMode;
	}

	void FilteringModesDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();

		// Load a compiled vertex shader
		vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\FilteringModesDemoVS.cso", compiledVertexShader);
		ThrowIfFailed(direct3DDevice->CreateVertexShader(compiledVertexShader.data(), compiledVertexShader.size(), nullptr, mVertexShader.put()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\FilteringModesDemoPS.cso", compiledPixelShader);
		ThrowIfFailed(direct3DDevice->CreatePixelShader(compiledPixelShader.data(), compiledPixelShader.size(), nullptr, mPixelShader.put()), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		const D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(direct3DDevice->CreateInputLayout(inputElementDescriptions, narrow_cast<uint32_t>(size(inputElementDescriptions)), compiledVertexShader.data(), compiledVertexShader.size(), mInputLayout.put()), "ID3D11Device::CreateInputLayout() failed.");

		const float size = 10.0f;
		const float halfSize = size / 2.0f;

		// Create a vertex buffer
		const VertexPositionTexture vertices[] =
		{
			VertexPositionTexture(XMFLOAT4(-halfSize, 1.0f, 0.0, 1.0f), XMFLOAT2(0.0f, 1.0f)),
			VertexPositionTexture(XMFLOAT4(-halfSize, size + 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(halfSize, size + 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(halfSize, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f))
		};

		VertexPositionTexture::CreateVertexBuffer(direct3DDevice, vertices, not_null<ID3D11Buffer * *>(mVertexBuffer.put()));

		// Create an index buffer
		const uint16_t sourceIndices[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		const span<const uint16_t> indices{ sourceIndices };
		mIndexCount = narrow_cast<uint32_t>(indices.size());
		CreateIndexBuffer(direct3DDevice, indices, not_null<ID3D11Buffer * *>(mIndexBuffer.put()));

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(CBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");

		// Load a texture
		const wstring textureName = L"Content\\Textures\\EarthComposite.dds";
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile(direct3DDevice, mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, mColorTexture.put()), "CreateDDSTextureFromFile() failed.");

		// Create texture samplers
		for (FilteringModes mode = FilteringModes(0); mode < FilteringModes::End; mode = FilteringModes(static_cast<int>(mode) + 1))
		{
			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

			switch (mode)
			{
			case FilteringModes::Point:
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				break;

			case FilteringModes::TriLinear:
				samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				break;

			case FilteringModes::Anisotropic:
				samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
				samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
				break;

			default:
				throw GameException("Unsupported texture addressing mode.");
			}

			ThrowIfFailed(direct3DDevice->CreateSamplerState(&samplerDesc, mTextureSamplersByFilteringMode[mode].put()), "ID3D11Device::CreateSamplerState() failed.");
		}

		mKeyboard = reinterpret_cast<KeyboardComponent*>(mGame->Services().GetService(KeyboardComponent::TypeIdClass()));

		auto updateConstantBufferFunc = [this]() { mUpdateConstantBuffer = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateConstantBufferFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateConstantBufferFunc);
	}

	void FilteringModesDemo::Update(const GameTime&)
	{
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
			{
				FilteringModes activeMode = FilteringModes(static_cast<int>(mActiveFilteringMode) + 1);
				if (activeMode >= FilteringModes::End)
				{
					activeMode = FilteringModes(0);
				}

				mActiveFilteringMode = activeMode;
			}
		}
	}

	void FilteringModesDemo::Draw(const GameTime&)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.get());

		const uint32_t stride = VertexPositionTexture::VertexSize();
		const uint32_t offset = 0;
		const auto vertexBuffers = mVertexBuffer.get();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
		direct3DDeviceContext->VSSetShader(mVertexShader.get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.get(), nullptr, 0);

		if (mUpdateConstantBuffer)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
			wvp = XMMatrixTranspose(wvp);
			XMStoreFloat4x4(&mCBufferPerObjectData.WorldViewProjection, wvp);
			direct3DDeviceContext->UpdateSubresource(mCBufferPerObject.get(), 0, nullptr, &mCBufferPerObjectData, 0, 0);
			mUpdateConstantBuffer = false;
		}

		const auto vsConstantBuffers = mCBufferPerObject.get();
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &vsConstantBuffers);

		const auto psShaderResources = mColorTexture.get();
		direct3DDeviceContext->PSSetShaderResources(0, 1, &psShaderResources);

		const auto psSamplers = mTextureSamplersByFilteringMode[mActiveFilteringMode].get();
		direct3DDeviceContext->PSSetSamplers(0, 1, &psSamplers);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}
}