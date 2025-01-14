#pragma once
#include "pch.h"
#include <memory.h>

class Texture
{
	public:
		Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);
		~Texture();

		static std::unique_ptr<Texture> LoadFromFile(ID3D11Device* pDevice, const std::string& textureFile);

		ID3D11ShaderResourceView* GetShaderResourceView() const;

	private:

		ID3D11Texture2D* m_pResource = nullptr;
		ID3D11ShaderResourceView* m_pShaderResourceView = nullptr;
};