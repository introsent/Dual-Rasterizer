#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

using namespace dae;

//ID3D11Debug* d3d11Debug;

void ShutDown(SDL_Window* pWindow)
{
	//d3d11Debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
	//
	//if (d3d11Debug)
	//{
	//	d3d11Debug->Release();
	//	d3d11Debug = nullptr;
	//}

	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}




int main(int argc, char* args[])
{
	const std::string MAGENTA = "\033[35m";
	const std::string YELLOW = "\033[33m";
	const std::string GREEN = "\033[32m";
	const std::string RESET = "\033[0m";

	std::cout << YELLOW  << "[Key Bindings - SHARED]"												<< RESET << std::endl;
	std::cout << YELLOW  << "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)"					<< RESET << std::endl;
	std::cout << YELLOW  << "   [F2]  Toggle Vehicle Rotation (ON/OFF)"								<< RESET << std::endl;
	std::cout << YELLOW  << "   [F3]  Toggle FireFX (ON/OFF)"										<< RESET << std::endl;
	std::cout << YELLOW  << "   [F9]  Cycle CullMode (BACK/FRONT/NONE)"								<< RESET << std::endl;
	std::cout << YELLOW  << "   [F10] Toggle Uniform ClearColor (ON/OFF)"							<< RESET << std::endl;
	std::cout << YELLOW  << "   [F11] Toggle Print FPS (ON/OFF)"									<< RESET << std::endl << "\n";
						 
	std::cout << GREEN   << "[Key Bindings - HARDWARE]"												<< RESET << std::endl;
	std::cout << GREEN   << "   [F4]  Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)"				<< RESET << std::endl << "\n";

	std::cout << MAGENTA << "[Key Bindings - SOFTWARE]"												<< RESET << std::endl;
	std::cout << MAGENTA << "   [F5]  Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)" << RESET << std::endl;
	std::cout << MAGENTA << "   [F6]  Toggle NormalMap (ON/OFF)"									<< RESET << std::endl;
	std::cout << MAGENTA << "   [F7]  Toggle DepthBuffer Visualization (ON/OFF)"					<< RESET << std::endl;
	std::cout << MAGENTA << "   [F8]  Toggle BoundingBox Visualization (ON/OFF)"					<< RESET << std::endl << "\n" << "\n";

	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - Ivans Minajevs / 2GD11",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool printFPS = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					pRenderer->ChangeRenderingBackendType();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRenderer->ChangeIsRotating();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pRenderer->ChangeToRenderFireMesh();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					pRenderer->ChangeFilteringTechnique();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					if (pRenderer->GetDisplayMode() == DisplayMode::DepthBuffer)
					{
						pRenderer->SetDisplayMode(DisplayMode::ShadingMode);
					}
					else
					{
						pRenderer->ChangeShadingMode();
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					pRenderer->ChangeIsNormalMap();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					if (pRenderer->GetDisplayMode() == DisplayMode::ShadingMode)
					{
						std::cout << MAGENTA << "**(SOFTWARE) DepthBuffer Visualization ON" << RESET << std::endl;
						pRenderer->SetDisplayMode(DisplayMode::DepthBuffer);
					}
					else if (pRenderer->GetDisplayMode() == DisplayMode::DepthBuffer)
					{
						std::cout << MAGENTA << "**(SOFTWARE) DepthBuffer Visualization OFF" << RESET << std::endl;
						pRenderer->SetDisplayMode(DisplayMode::ShadingMode);
					}
					
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					if (pRenderer->GetDisplayMode() == DisplayMode::ShadingMode)
					{
						std::cout << MAGENTA << "**(SOFTWARE) BoundingBox Visualization ON" << RESET << std::endl;
						pRenderer->SetDisplayMode(DisplayMode::BoundingBox);
					} 
					else if (pRenderer->GetDisplayMode() == DisplayMode::BoundingBox)
					{
						std::cout << MAGENTA << "**(SOFTWARE) BoundingBox Visualization OFF" << RESET << std::endl;
						pRenderer->SetDisplayMode(DisplayMode::ShadingMode);
					}
					
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					pRenderer->ChangeCullingMode();
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pRenderer->ChangeIsClearColorUniform();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					printFPS = !printFPS;

					if (printFPS)
					{
						std::cout << YELLOW << "**(SHARED)Print FPS ON" << RESET << std::endl;
					}
					else
					{
						std::cout << YELLOW << "**(SHARED)Print FPS OFF" << RESET << std::endl;
					}
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if (printFPS)
			{
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;

	

	delete pTimer;

	ShutDown(pWindow);
	return 0;
}