#include "Texture.h"

#include <iostream>
#include <ostream>

#include "Vector2.h"
#include <SDL_image.h>

Texture::Texture(ID3D11Device* pDevice, SDL_Surface* pSurface) 
{
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;	

	if (m_pResource != 0) hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);

	SDL_FreeSurface(pSurface);
}

Texture::~Texture()
{
	if (m_pShaderResourceView)
	{
		m_pShaderResourceView->Release();
		m_pShaderResourceView = nullptr;
	}

	if (m_pResource)
	{
		m_pResource->Release();
		m_pResource = nullptr;
	}
}


std::unique_ptr<Texture> Texture::LoadFromFile(ID3D11Device* pDevice, const std::string& textureFile)
{

	return std::make_unique<Texture>(pDevice, IMG_Load(textureFile.c_str()));
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
{
	return m_pShaderResourceView;
}