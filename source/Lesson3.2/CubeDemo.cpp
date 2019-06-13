#include "pch.h"
#include "CubeDemo.h"
#include "Utility.h"
#include "GameException.h"
#include "Game.h"
#include "VertexDeclarations.h"
#include "FirstPersonCamera.h"

using namespace std;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	CubeDemo::CubeDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	bool CubeDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void CubeDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void CubeDemo::Initialize()
	{
		// Load a compiled vertex shader
		vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\CubeDemoVS.cso", compiledVertexShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.put()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\CubeDemoPS.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.put()), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		const D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, narrow_cast<uint32_t>(size(inputElementDescriptions)), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.put()), "ID3D11Device::CreateInputLayout() failed.");

		// Create a vertex buffer
		const VertexPositionColor vertices[] =
		{
			VertexPositionColor(XMFLOAT4(-1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(&Colors::Green[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(&Colors::Yellow[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(&Colors::White[0])),
			VertexPositionColor(XMFLOAT4(-1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(&Colors::Cyan[0])),

			VertexPositionColor(XMFLOAT4(-1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(&Colors::Blue[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(&Colors::Purple[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(&Colors::Red[0])),
			VertexPositionColor(XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(&Colors::Black[0]))
		};

		D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.ByteWidth = VertexPositionColor::VertexBufferByteWidth(size(vertices));
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed.");

		// Create an index buffer
		const uint16_t indices[] =
		{
			0, 1, 2,
			0, 2, 3,

			4, 5, 6,
			4, 6, 7,

			3, 2, 5,
			3, 5, 4,

			2, 1, 6,
			2, 6, 5,

			1, 7, 6,
			1, 0, 7,

			0, 3, 4,
			0, 4, 7
		};

		mIndexCount = narrow_cast<uint32_t>(size(indices));

		D3D11_BUFFER_DESC indexBufferDesc{ 0 };
		indexBufferDesc.ByteWidth = narrow_cast<uint16_t>(sizeof(uint16_t)) * mIndexCount;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData{ 0 };
		indexSubResourceData.pSysMem = indices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, mIndexBuffer.put()), "ID3D11Device::CreateBuffer() failed.");

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = narrow_cast<uint32_t>(sizeof(CBufferPerObject));
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mConstantBuffer.put()), "ID3D11Device::CreateBuffer() failed.");

		auto firstPersonCamera = mCamera->As<FirstPersonCamera>();
		if (firstPersonCamera != nullptr)
		{
			firstPersonCamera->AddPositionUpdatedCallback([this]() {
				mUpdateConstantBuffer = true;
				});
		}
	}

	void CubeDemo::Update(const GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			mRotationAngle += gameTime.ElapsedGameTimeSeconds().count() * RotationRate;
			XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationY(mRotationAngle));
			mUpdateConstantBuffer = true;
		}
	}

	void CubeDemo::Draw(const GameTime&)
	{
		assert(mCamera != nullptr);

		ID3D11DeviceContext * direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.get());

		const uint32_t stride = VertexPositionColor::VertexSize();
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
			XMStoreFloat4x4(&mCBufferPerObject.WorldViewProjection, wvp);
			direct3DDeviceContext->UpdateSubresource(mConstantBuffer.get(), 0, nullptr, &mCBufferPerObject, 0, 0);
			mUpdateConstantBuffer = false;
		}

		const auto vsConstantBuffers = mConstantBuffer.get();
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &vsConstantBuffers);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}
}