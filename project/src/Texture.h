#pragma once
#include "pch.h"
#include <memory.h>
#include "Vector2.h"
namespace dae
{
	class Texture
	{
	public:
		Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);
		~Texture();

		static std::unique_ptr<Texture> LoadFromFile(ID3D11Device* pDevice, const std::string& textureFile);

		ID3D11ShaderResourceView* GetShaderResourceView() const;
		ColorRGB Sample(const Vector2& uv) const;

	private:
		SDL_Surface* m_pSurface;
		uint32_t* m_pSurfacePixels{ nullptr };

		ID3D11Texture2D* m_pResource = nullptr;
		ID3D11ShaderResourceView* m_pShaderResourceView = nullptr;
	};
}