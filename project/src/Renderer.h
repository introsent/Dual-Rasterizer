#pragma once
#include <memory>
#include "Mesh3D.h"
#include "Camera.h"
#include "FireEffect.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void CleanupDirectX();
		void Render() const;

		enum FilteringTechnique
		{
			Point,
			Linear,
			Anisotropic
		};
		void ChangeFilteringTechnique();

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };


		void OnDeviceLost();
		//DIRECTX
		HRESULT InitializeDirectX();
		
		ID3D11Device*			m_pDevice;
		ID3D11DeviceContext*	m_pDeviceContext{};
		IDXGISwapChain*			m_pSwapChain{};
		ID3D11Texture2D*		m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource*			m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};
		

		//MESH
		Matrix m_WorldMatrix{};
		std::unique_ptr<Mesh3D> m_pVehicle;
		std::unique_ptr<Mesh3D> m_pFire;

		std::unique_ptr<Camera> m_pCamera;
		FilteringTechnique m_FilteringTechnique{ FilteringTechnique::Anisotropic };

		std::unique_ptr<VehicleEffect> m_pVehicleEffect;
		std::unique_ptr<FireEffect> m_pFireEffect;

		void InitializeVehicle();
		void InitializeFire();
	};
}
