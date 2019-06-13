#include "pch.h"
#include "ColoredTriangleDemo.h"
#include "Utility.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include <gsl\gsl>

using namespace std;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	RTTI_DEFINITIONS(ColoredTriangleDemo)

		ColoredTriangleDemo::ColoredTriangleDemo(Library::Game& game) :
		DrawableGameComponent(game), mVertexShader(), mPixelShader()
	{
	}

	void ColoredTriangleDemo::Initialize()
	{
		// Load a compiled vertex shader
		std::vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\ColoredTriangleDemoVS.cso", compiledVertexShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.put()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		std::vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\ColoredTriangleDemoPS.cso", compiledPixelShader);
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
			VertexPositionColor(XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f), XMFLOAT4(Colors::Red.f)),
			VertexPositionColor(XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(Colors::Green.f)),
			VertexPositionColor(XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f), XMFLOAT4(Colors::Blue.f))
		};

		D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.ByteWidth = VertexPositionColor::VertexBufferByteWidth(size(vertices));
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = vertices;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed.");
	}

	void ColoredTriangleDemo::Draw(const Library::GameTime&)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.get());

		const uint32_t stride = VertexPositionColor::VertexSize();
		const uint32_t offset = 0;
		const auto vertexBuffers = mVertexBuffer.get();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, &stride, &offset);

		direct3DDeviceContext->VSSetShader(mVertexShader.get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.get(), nullptr, 0);

		direct3DDeviceContext->Draw(3, 0);
	}
}