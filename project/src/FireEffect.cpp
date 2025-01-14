#include "FireEffect.h"

FireEffect::FireEffect(ID3D11Device* pDevice, const std::wstring& assetFile) : Effect(pDevice, assetFile)
{
	m_EffectSamplerVariable = m_pEffect->GetVariableByName("gSamplerState")->AsSampler();
	if (!m_EffectSamplerVariable->IsValid())
	{
		std::wcout << L"Effect sampler veriable is not valid\n";
		return;
	}


	// Point sampler
	D3D11_SAMPLER_DESC pointDesc = {};
	pointDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pointDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	pointDesc.MinLOD = 0;
	pointDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = pDevice->CreateSamplerState(&pointDesc, &m_pSamplerPoint);
	if (FAILED(hr))
	{
		std::wcout << L"Point Sampling state not valid\n";
	}

	// Linear sampler
	D3D11_SAMPLER_DESC linearDesc = {};
	linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	linearDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	linearDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	linearDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	linearDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	linearDesc.MinLOD = 0;
	linearDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pDevice->CreateSamplerState(&linearDesc, &m_pSamplerLinear);
	if (FAILED(hr))
	{
		std::wcout << L"Linear Sampling state not valid\n";
	}

	// Anisotropic sampler
	D3D11_SAMPLER_DESC anisotropicDesc = {};
	anisotropicDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	anisotropicDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	anisotropicDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	anisotropicDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	anisotropicDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	anisotropicDesc.MinLOD = 0;
	anisotropicDesc.MaxLOD = D3D11_FLOAT32_MAX;
	anisotropicDesc.MaxAnisotropy = 16; // Maximum anisotropy level
	hr = pDevice->CreateSamplerState(&anisotropicDesc, &m_pSamplerAnisotropic);
	if (FAILED(hr))
	{
		std::wcout << L"Anisotropic Sampling state not valid\n";
	}

	if (m_pSamplerAnisotropic != nullptr)
	{
		m_EffectSamplerVariable->SetSampler(0, m_pSamplerAnisotropic);
	}


	std::unique_ptr<Texture> pDiffuseTexture = Texture::LoadFromFile(pDevice, "resources/fireFX_diffuse.png");
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (m_pDiffuseMapVariable->IsValid()) {
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture.get()->GetShaderResourceView());
	}
	else
	{
		std::wcout << L"m_pDiffuseMapVariable not valid!\n";
	}
}

FireEffect::~FireEffect()
{
	//Release maps
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
		m_pDiffuseMapVariable = nullptr;
	}

	//Release samplers
	if (m_pSamplerAnisotropic)
	{
		m_pSamplerAnisotropic->Release();
		m_pSamplerAnisotropic = nullptr;
	}

	if (m_pSamplerLinear)
	{
		m_pSamplerLinear->Release();
		m_pSamplerLinear = nullptr;
	}

	if (m_pSamplerPoint)
	{
		m_pSamplerPoint->Release();
		m_pSamplerPoint = nullptr;
	}

	if (m_EffectSamplerVariable)
	{
		m_EffectSamplerVariable->Release();
		m_EffectSamplerVariable = nullptr;
	}
}

void FireEffect::SetPointSampling()
{
	HRESULT hr = m_EffectSamplerVariable->SetSampler(0, m_pSamplerPoint);
	if (SUCCEEDED(hr))
	{
		printf("Sampler state set to Point.\n");
	}
}

void FireEffect::SetLinearSampling()
{
	HRESULT hr = m_EffectSamplerVariable->SetSampler(0, m_pSamplerLinear);
	if (SUCCEEDED(hr))
	{
		printf("Sampler state set to Linear.\n");
	}
}

void FireEffect::SetAnisotropicSampling()
{
	HRESULT hr = m_EffectSamplerVariable->SetSampler(0, m_pSamplerAnisotropic);
	if (SUCCEEDED(hr))
	{
		printf("Sampler state set to Anisotropic.\n");
	}
}
