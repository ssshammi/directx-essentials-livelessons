#include "pch.h"
#include "ModelDemo.h"
#include "Utility.h"
#include "ColorHelper.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"

using namespace std;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	ModelDemo::ModelDemo(Game& game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	bool ModelDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void ModelDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void ModelDemo::Initialize()
	{
		// Load a compiled vertex shader
		vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\ModelDemoVS.cso", compiledVertexShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.put()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\ModelDemoPS.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.put()), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		const D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, narrow_cast<uint32_t>(size(inputElementDescriptions)), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.put()), "ID3D11Device::CreateInputLayout() failed.");

		// Load the model
		Library::Model model("Content\\Models\\Sphere.obj.bin");

		// Create vertex and index buffers for the model
		Mesh* mesh = model.Meshes().at(0).get();
		CreateVertexBuffer(*mesh, not_null<ID3D11Buffer * *>(mVertexBuffer.put()));
		mesh->CreateIndexBuffer(*mGame->Direct3DDevice(), not_null<ID3D11Buffer * *>(mIndexBuffer.put()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = narrow_cast<uint32_t>(sizeof(CBufferPerObject));
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mConstantBuffer.put()), "ID3D11Device::CreateBuffer() failed.");

		auto updateConstantBufferFunc = [this]() { mUpdateConstantBuffer = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateConstantBufferFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateConstantBufferFunc);
	}

	void ModelDemo::Update(const GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			mRotationAngle += gameTime.ElapsedGameTimeSeconds().count() * RotationRate;
			XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationY(mRotationAngle));
			mUpdateConstantBuffer = true;
		}
	}

	void ModelDemo::Draw(const GameTime&)
	{
		assert(mCamera != nullptr);

		ID3D11DeviceContext * direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.get());

		uint32_t stride = VertexPositionColor::VertexSize();
		uint32_t offset = 0;
		const auto vertexBuffers = mVertexBuffer.get();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);

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

	void ModelDemo::CreateVertexBuffer(const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer) const
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

		vector<VertexPositionColor> vertices;
		vertices.reserve(sourceVertices.size());
		if (mesh.VertexColors().size() > 0)
		{
			const vector<XMFLOAT4>& vertexColors = mesh.VertexColors().at(0);
			assert(vertexColors.size() == sourceVertices.size());

			for (size_t i = 0; i < sourceVertices.size(); i++)
			{
				const XMFLOAT3& position = sourceVertices.at(i);
				const XMFLOAT4& color = vertexColors.at(i);
				vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), color);
			}
		}
		else
		{
			for (size_t i = 0; i < sourceVertices.size(); i++)
			{
				const XMFLOAT3& position = sourceVertices.at(i);
				XMFLOAT4 color = ColorHelper::RandomColor();
				vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), color);
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.ByteWidth = VertexPositionColor::VertexBufferByteWidth(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}
}