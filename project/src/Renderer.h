#pragma once
#include <memory>
#include "Mesh3D.h"
#include "Camera.h"
#include "FireEffect.h"
#include "DataTypes.h"

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
		void RenderGPU() const;
		void RenderCPU() const;

		void ChangeShadingMode();
		void SetDisplayMode(DisplayMode displayMode);
		DisplayMode GetDisplayMode() const;
		void ChangeFilteringTechnique();
		void ChangeRenderingBackendType();
		void ChangeIsRotating();
		void ChangeToRenderFireMesh();
		void ChangeIsNormalMap();
	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };



		void OnDeviceLost();
		//DIRECTX
		HRESULT InitializeDirectX();
		
		//Hardware initialization
		ID3D11Device*			m_pDevice;
		ID3D11DeviceContext*	m_pDeviceContext{};
		IDXGISwapChain*			m_pSwapChain{};
		ID3D11Texture2D*		m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource*			m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};
		
		//Software initialization
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};


		//MESH
		Matrix m_WorldMatrix{};
		std::unique_ptr<Mesh3D> m_pVehicle;
		std::unique_ptr<Mesh3D> m_pFire;

		std::unique_ptr<Camera> m_pCamera;
		FilteringTechnique m_FilteringTechnique{ FilteringTechnique::Anisotropic };
		RenderingBackendType m_RenderingBackendType{ RenderingBackendType::Hardware };
		ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };
		DisplayMode m_CurrentDisplayMode{ DisplayMode::ShadingMode };
		bool m_IsNormalMap{ true };
		bool m_IsRotating{ true };
		bool m_ToRenderFireMesh{ true };



		std::unique_ptr<VehicleEffect> m_pVehicleEffect;
		std::unique_ptr<FireEffect> m_pFireEffect;

		void InitializeVehicle();
		void InitializeFire();
	};
}
