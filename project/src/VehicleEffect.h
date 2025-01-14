#pragma once
#include "pch.h"
#include "Effect.h"
#include "Texture.h"
class VehicleEffect final : public Effect
{
public:
    VehicleEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
    virtual ~VehicleEffect();

    VehicleEffect(const Effect& other) = delete;
    VehicleEffect& operator=(const Effect& rhs) = delete;
    VehicleEffect(Effect&& other) = delete;
    VehicleEffect& operator=(Effect&& rhs) = delete;

	void SetPointSampling();
	void SetLinearSampling();
    void SetAnisotropicSampling();

    Texture* GetDiffuseTexture() override;
    Texture* GetNormalTexture() override;
    Texture* GetSpecularTexture() override;
    Texture* GetGlossinessTexture() override;

    void Update(const Vector3& cameraPosition, const Matrix& pWorldMatrix, const Matrix& pWorldViewProjectionMatrix) override;

protected:
    ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
    ID3DX11EffectVectorVariable* m_pVecCameraVariable;

    ID3D11SamplerState* m_pSamplerPoint{};
    ID3D11SamplerState* m_pSamplerLinear{};
    ID3D11SamplerState* m_pSamplerAnisotropic{};
    ID3DX11EffectSamplerVariable* m_EffectSamplerVariable{};

    std::unique_ptr<Texture> m_pUDiffuseTexture;
    std::unique_ptr<Texture> m_pUNormalTexture;
    std::unique_ptr<Texture> m_pUSpecularTexture;
    std::unique_ptr<Texture> m_pUGlossinessTexture;

};
