#pragma once
#include "pch.h"
#include "Texture.h"
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

protected:
    ID3DX11Effect* m_pEffect;
    ID3DX11EffectTechnique* m_pTechnique;

    ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;

    static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};
