#include "pch.h"
#include "Renderer.h"
#include "Mesh3D.h"
#include "Utils.h"

//extern ID3D11Debug* d3d11Debug;
namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;

			m_pVehicleEffect = std::make_unique<VehicleEffect>(m_pDevice, L"resources/PosCol3D.fx");
			InitializeVehicle();

			m_pFireEffect = std::make_unique<FireEffect>(m_pDevice, L"resources/Fire3D.fx");
			InitializeFire();

			m_pCamera = std::make_unique<Camera>(Vector3{ 0.f, 0.f , -50.f }, 45.f, float(m_Width), float(m_Height));
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
	}

	Renderer::~Renderer()
	{
		CleanupDirectX();
	}


	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera.get()->Update(pTimer);
		m_WorldMatrix = Matrix::CreateRotationY(pTimer->GetTotal() * PI / 2);
	}

	void Renderer::Render() const
	{
		
	}

	void Renderer::RenderCPU() const
	{
		// Reset depth buffer and clear screen
		std::fill(m_pDepthBufferPixels, m_pDepthBufferPixels + (m_Width * m_Height), std::numeric_limits<float>::max());

		// Clear screen with black color
		SDL_Color clearColor = { 100, 100, 100, 255 };
		Uint32 color = SDL_MapRGB(m_pBackBuffer->format, clearColor.r, clearColor.g, clearColor.b);
		SDL_FillRect(m_pBackBuffer, nullptr, color);

		// Lock the back buffer before drawing
		SDL_LockSurface(m_pBackBuffer);

		// RENDER LOGIC
		for (Mesh& mesh : m_MeshesWorld) {
			// Apply transformations
			VertexTransformationFunction(mesh);

			bool isTriangleList = (mesh.primitiveTopology == PrimitiveTopology::TriangleList);

			// Parallelize over triangles
#pragma omp parallel for
			for (int inx = 0; inx < mesh.indices.size(); inx += (isTriangleList ? 3 : 1)) {

				auto t0 = mesh.indices[inx];
				auto t1 = mesh.indices[inx + 1];
				auto t2 = mesh.indices[inx + 2];

				////Perform clipping 
				//std::vector<Vertex_Out> clippedVertices;
				//std::vector<uint32_t> clippedIndices;
				//ClipTriangle(mesh.vertices_out[t0], mesh.vertices_out[t1], mesh.vertices_out[t2], clippedVertices, clippedIndices);
				//
				//if (clippedVertices.size() < 3) continue; // If there are not enough vertices left after clipping, skip this triangle

				// Skip degenerate triangles
				if (t0 == t1 || t1 == t2 || t2 == t0) continue;

				// Vertex positions
				auto v0 = mesh.vertices_out[t0].position;
				auto v1 = mesh.vertices_out[t1].position;
				auto v2 = mesh.vertices_out[t2].position;

				// Skip if any vertex is behind the camera (w < 0)
				if (v0.w < 0 || v1.w < 0 || v2.w < 0) continue;

				if ((v0.x < -1 || v0.x > 1) || (v1.x < -1 || v1.x > 1) || (v2.x < -1 || v2.x > 1)
					|| ((v0.y < -1 || v0.y > 1) || (v1.y < -1 || v1.y > 1) || (v2.y < -1 || v2.y > 1))
					|| ((v0.z < 0 || v0.z > 1) || (v1.z < 0 || v1.z > 1) || (v2.z < 0 || v2.z > 1))) continue;


				// Backface culling (skip if the triangle is facing away from the camera)
				Vector3 edge0 = v1 - v0;
				Vector3 edge1 = v2 - v0;
				Vector3 normal = Vector3::Cross(edge0, edge1);
				if (normal.z <= 0) continue;


				// Transform coordinates to screen space
				v0.x *= m_Width;
				v1.x *= m_Width;
				v2.x *= m_Width;
				v0.y *= m_Height;
				v1.y *= m_Height;
				v2.y *= m_Height;

				// Compute bounding box of the triangle
				int minX = std::max(0, static_cast<int>(std::floor(std::min({ v0.x, v1.x, v2.x }))));
				int maxX = std::min(m_Width, static_cast<int>(std::ceil(std::max({ v0.x, v1.x, v2.x }))));
				int minY = std::max(0, static_cast<int>(std::floor(std::min({ v0.y, v1.y, v2.y }))));
				int maxY = std::min(m_Height, static_cast<int>(std::ceil(std::max({ v0.y, v1.y, v2.y }))));

				// Edge vectors for barycentric coordinates
				auto e0 = v2 - v1;
				auto e1 = v0 - v2;
				auto e2 = v1 - v0;

				Vector2 edge0_2D(e0.x, e0.y);
				Vector2 edge1_2D(e1.x, e1.y);
				Vector2 edge2_2D(e2.x, e2.y);

				float wProduct = v0.w * v1.w * v2.w;



				// Parallelize over rows of pixels (py)
#pragma omp parallel for
				for (int py = minY; py < maxY; ++py) {
					for (int px = minX; px < maxX; ++px) {
						ColorRGB finalColor;
						auto P = Vector2(px + 0.5f, py + 0.5f);

						auto p0 = P - Vector2(v1.x, v1.y);
						auto p1 = P - Vector2(v2.x, v2.y);
						auto p2 = P - Vector2(v0.x, v0.y);

						auto weightP0 = Vector2::Cross(edge0_2D, p0);
						auto weightP1 = Vector2::Cross(edge1_2D, p1);
						auto weightP2 = Vector2::Cross(edge2_2D, p2);

						if (weightP0 < 0 || weightP1 < 0 || weightP2 < 0) continue;

						auto totalArea = weightP0 + weightP1 + weightP2;
						float reciprocalTotalArea = 1.0f / totalArea;

						float interpolationScale0 = weightP0 * reciprocalTotalArea;
						float interpolationScale1 = weightP1 * reciprocalTotalArea;
						float interpolationScale2 = weightP2 * reciprocalTotalArea;

						// Compute z-buffer value for depth testing
						float zBufferValue = 1.f / (1.f / v0.z * interpolationScale0 +
							1.f / v1.z * interpolationScale1 +
							1.f / v2.z * interpolationScale2);

						if (zBufferValue < 0 || zBufferValue > 1) continue;



						int pixelIndex = px + (py * m_Width);
						if (zBufferValue >= m_pDepthBufferPixels[pixelIndex]) continue;

						m_pDepthBufferPixels[pixelIndex] = zBufferValue;

						// Interpolated depth for final color calculation
						float interpolatedDepth = wProduct / (v1.w * v2.w * interpolationScale0 +
							v0.w * v2.w * interpolationScale1 +
							v0.w * v1.w * interpolationScale2);
						if (interpolatedDepth <= 0) continue;

						// Texture sampling
						Vertex_Out pixelVertex;

						pixelVertex.position = (mesh.vertices[t0].position.ToPoint4() + mesh.vertices[t1].position.ToPoint4() + mesh.vertices[t2].position.ToPoint4()) / 3.f;
						pixelVertex.position.z = zBufferValue;
						pixelVertex.position.w = interpolatedDepth;


						pixelVertex.uv = Vector2::Interpolate(mesh.vertices_out[t0].uv, mesh.vertices_out[t1].uv, mesh.vertices_out[t2].uv,
							v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);

						pixelVertex.normal = Vector3::Interpolate(mesh.vertices_out[t0].normal, mesh.vertices_out[t1].normal, mesh.vertices_out[t2].normal,
							v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);
						pixelVertex.normal.Normalize();


						pixelVertex.tangent = Vector3::Interpolate(mesh.vertices_out[t0].tangent, mesh.vertices_out[t1].tangent, mesh.vertices_out[t2].tangent,
							v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);
						pixelVertex.tangent.Normalize();

						pixelVertex.viewDirection = Vector3::Interpolate(mesh.vertices_out[t0].viewDirection, mesh.vertices_out[t1].viewDirection, mesh.vertices_out[t2].viewDirection,
							v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);
						pixelVertex.viewDirection.Normalize();

						pixelVertex.color = colors::Black;

						// If texture mapping is enabled, sample the texture
						if (m_CurrentDisplayMode == DisplayMode::FinalColor)
						{
							finalColor = m_DiffuseTexture->Sample(pixelVertex.uv);
						}
						if (m_CurrentDisplayMode == DisplayMode::DepthBuffer)
						{
							auto clampedValue = Remap(zBufferValue, 0.8f, 1.f, 0.f, 1.f);
							finalColor = ColorRGB(clampedValue, clampedValue, clampedValue);
						}
						if (m_CurrentDisplayMode == DisplayMode::ShadingMode)
						{
							PixelShading(pixelVertex);
							finalColor = pixelVertex.color;
						}

						finalColor.MaxToOne();

						m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255.f),
							static_cast<uint8_t>(finalColor.g * 255.f),
							static_cast<uint8_t>(finalColor.b * 255.f));
					}
				}
			}
		}
		// Unlock after rendering
		SDL_UnlockSurface(m_pBackBuffer);

		// Copy the back buffer to the front buffer for display
		SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void Renderer::RenderGPU() const
	{
		if (!m_IsInitialized)
			return;

		// 1. Clear RTV & DSV
		constexpr float color[4] = { 0.f, 0.f, 0.3f, 1.f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. Set pipeline + invoke draw calls (= RENDER)
		m_pVehicle.get()->Render(m_pCamera->origin, m_WorldMatrix, m_WorldMatrix * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix(), m_pDeviceContext);
		m_pFire.get()->Render(m_pCamera->origin, m_WorldMatrix, m_WorldMatrix * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix(), m_pDeviceContext);

		//3. Present backbuffer (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::ChangeFilteringTechnique()
	{
		switch (m_FilteringTechnique)
		{
		case FilteringTechnique::Point:
			m_pVehicleEffect->SetLinearSampling();
			m_FilteringTechnique = FilteringTechnique::Linear;
			break;
		case FilteringTechnique::Linear:
			m_pVehicleEffect->SetAnisotropicSampling();
			m_FilteringTechnique = FilteringTechnique::Anisotropic;
			break;
		case FilteringTechnique::Anisotropic:
			m_pVehicleEffect->SetPointSampling();
			m_FilteringTechnique = FilteringTechnique::Point;
			break;
		}
	}

	void Renderer::ChangeRenderingBackendType()
	{
		switch (m_FilteringTechnique)
		{
		case RenderingBackendType::Software:
			m_RenderingBackendType = RenderingBackendType::Hardware;
			break;
		case RenderingBackendType::Hardware:
			m_RenderingBackendType = RenderingBackendType::Software;
			break;
		}
	}

	void Renderer::OnDeviceLost()
	{
		// Release all resources tied to the device
		CleanupDirectX();

		// Recreate device and swap chain
		InitializeDirectX();

		// Recreate resources
		InitializeVehicle();
		InitializeFire();

	}
	
	void Renderer::InitializeVehicle()
	{
		//Create some data for our mesh
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		
		Utils::ParseOBJ("resources/vehicle.obj", vertices, indices);

		m_pVehicle = std::make_unique<Mesh3D>(m_pDevice, vertices, indices, m_pVehicleEffect.get());
	}

	void Renderer::InitializeFire()
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Utils::ParseOBJ("resources/fireFX.obj", vertices, indices);

		m_pFire = std::make_unique<Mesh3D>(m_pDevice, vertices, indices, m_pFireEffect.get());
	}


	HRESULT Renderer::InitializeDirectX()
	{
		//==== 1.Create Device & DeviceContext ====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
		#if defined(DEBUG) || defined(_DEBUG)
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif


		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
						1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result)) return result;
//		m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3d11Debug));

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result)) return result;

		//==== 2. Create Swapchain ====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create Swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result)) return result;
		if (pDxgiFactory)
		{
			pDxgiFactory->Release();
			pDxgiFactory = nullptr;
		}

		//==== 3. Create DepthStencil (DS) & DepthStencilView (DSV) ====
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result)) return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result)) return result;


		//==== 4. Create RenderTarget (RT) & RenderTargetView (RTV) ====
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result)) return result;

		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) return result;


		//==== 5. Bind RTV & DSV to Output merger Stage ====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//==== 6. Set Viewport ====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);
		
		//return S_FALSE;
		return S_OK;
	}

	

	void Renderer::CleanupDirectX()
	{
		// Clear state and flush the device context before releasing DeviceContext
		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
			m_pDepthStencilView = nullptr;
		}

		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
			m_pDepthStencilBuffer = nullptr;
		}

		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
			m_pRenderTargetView = nullptr;
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
			m_pRenderTargetBuffer = nullptr;
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
			m_pSwapChain = nullptr;
		}

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
			m_pDeviceContext = nullptr;
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
			m_pDevice = nullptr;
		}
	}

}
