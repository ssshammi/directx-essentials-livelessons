#include "PointDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "VertexDeclarations.h"

namespace Rendering
{
	RTTI_DEFINITIONS(PointDemo)

	PointDemo::PointDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mPixelShader(nullptr)
	{
	}

	PointDemo::~PointDemo()
	{
		ReleaseObject(mPixelShader)
		ReleaseObject(mVertexShader)
	}

	void PointDemo::Initialize()
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
	}

	void PointDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		direct3DDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

		direct3DDeviceContext->Draw(1, 0);
	}
}