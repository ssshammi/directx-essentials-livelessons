#include "CubeDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "VertexDeclarations.h"
#include "ColorHelper.h"
#include "MatrixHelper.h"
#include "Camera.h"

namespace Rendering
{
	RTTI_DEFINITIONS(CubeDemo)

	const float CubeDemo::RotationRate = XM_PI;

	CubeDemo::CubeDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), mVertexBuffer(nullptr),
		  mIndexBuffer(nullptr), mConstantBuffer(nullptr), mCBufferPerObject(), mWorldMatrix(MatrixHelper::Identity)
	{
	}

	CubeDemo::~CubeDemo()
	{
		ReleaseObject(mConstantBuffer)
		ReleaseObject(mIndexBuffer)
		ReleaseObject(mVertexBuffer)
		ReleaseObject(mPixelShader)
		ReleaseObject(mInputLayout)
		ReleaseObject(mVertexShader)
	}

	void CubeDemo::Initialize()
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
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		// Create a vertex buffer
		VertexPositionColor vertices[] =
		{
			VertexPositionColor(XMFLOAT4(-1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(&ColorHelper::Green[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, +1.0f, -1.0f, 1.0f), XMFLOAT4(&ColorHelper::Yellow[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(&ColorHelper::White[0])),
			VertexPositionColor(XMFLOAT4(-1.0f, +1.0f, +1.0f, 1.0f), XMFLOAT4(&ColorHelper::BlueGreen[0])),

			VertexPositionColor(XMFLOAT4(-1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(&ColorHelper::Blue[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, -1.0f, +1.0f, 1.0f), XMFLOAT4(&ColorHelper::Purple[0])),
			VertexPositionColor(XMFLOAT4(+1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(&ColorHelper::Red[0])),
			VertexPositionColor(XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(&ColorHelper::Black[0]))
		};

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionColor) * ARRAYSIZE(vertices);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, &mVertexBuffer), "ID3D11Device::CreateBuffer() failed.");

		// Create an index buffer
		UINT indices[] =
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

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(UINT) * ARRAYSIZE(indices);
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = indices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, &mIndexBuffer), "ID3D11Device::CreateBuffer() failed.");

		// Create a constant buffer
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(CBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &mConstantBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void CubeDemo::Update(const GameTime& gameTime)
	{
		static float angle = 0.0f;

		angle += static_cast<float>(gameTime.ElapsedGameTime()) * RotationRate;

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationY(angle));
	}

	void CubeDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(VertexPositionColor);
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

		direct3DDeviceContext->DrawIndexed(36, 0, 0);
	}
}