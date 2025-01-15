#pragma once
#include "pch.h"
#include "Texture.h"
#include <unordered_map>
using namespace dae;

class Effect
{
public:
    Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
    virtual ~Effect();

    Effect(const Effect& other) = delete;
    Effect& operator=(const Effect& rhs) = delete;
    Effect(Effect&& other) = delete;
    Effect& operator=(Effect&& rhs) = delete;

    ID3DX11Effect* GetEffect() const;

    ID3DX11EffectMatrixVariable* GetWorldViewProjectionMatrix() const;

    ID3DX11EffectTechnique* GetTechnique() const;

    virtual void Update(const Vector3& cameraPosition, const Matrix& pWorldMatrix, const Matrix& pWorldViewProjectionMatrix);


    void SetCullingMode(D3D11_CULL_MODE cullMode);
    void ApplyCullingMode(ID3D11DeviceContext* pContext);

    virtual Texture* GetDiffuseTexture() 
    {
        return nullptr;
    }
    virtual Texture* GetNormalTexture()
    {
        return nullptr;
    }
    virtual Texture* GetSpecularTexture()
    {
        return nullptr;
    }
    virtual Texture* GetGlossinessTexture()
    {
        return nullptr;
    }

    ID3D11RasterizerState* GetCurrentRasterizerState() const;

protected:
    ID3DX11Effect* m_pEffect;
    ID3DX11EffectTechnique* m_pTechnique;

    ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;

    ID3D11RasterizerState* m_CurrentRasterState;

    std::unordered_map<D3D11_CULL_MODE, ID3D11RasterizerState*> m_RasterStates;

    ID3D11RasterizerState* CreateRasterizerState(ID3D11Device* pDevice, D3D11_CULL_MODE cullMode);
    void CleanupRasterStates();

    static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};
