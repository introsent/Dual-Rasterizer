#include "Effect.h"
#include <sstream>
#include "Mesh3D.h"
using namespace dae;

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    m_pEffect = LoadEffect(pDevice, assetFile);

    m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
    if (!m_pTechnique->IsValid())
    {
        std::wcout << L"Technique not valid\n";
    }

    m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProjectionMatrix")->AsMatrix();
    if (!m_pMatWorldViewProjVariable->IsValid())
    {
        std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
    }

    // Initialize rasterizer states for all culling modes
    m_RasterStates[D3D11_CULL_BACK] = CreateRasterizerState(pDevice, D3D11_CULL_BACK);
    m_RasterStates[D3D11_CULL_FRONT] = CreateRasterizerState(pDevice, D3D11_CULL_FRONT);
    m_RasterStates[D3D11_CULL_NONE] = CreateRasterizerState(pDevice, D3D11_CULL_NONE);

    // Set default rasterizer state
    SetCullingMode(D3D11_CULL_BACK);
}

Effect::~Effect()
{
    //Release matrices
    if (m_pMatWorldViewProjVariable)
    {
        m_pMatWorldViewProjVariable->Release();
        m_pMatWorldViewProjVariable = nullptr;
    }

    //Release techniques
    if (m_pTechnique)
    {
        m_pTechnique->Release();
        m_pTechnique = nullptr;
    }

    //Release Effect
    if (m_pEffect)
    {
        m_pEffect->Release();
        m_pEffect = nullptr;
    }

    CleanupRasterStates();
}

ID3DX11Effect* Effect::GetEffect() const
{
    return m_pEffect;
}

ID3DX11EffectMatrixVariable* Effect::GetWorldViewProjectionMatrix() const
{
    return m_pMatWorldViewProjVariable;
}

ID3DX11EffectTechnique* Effect::GetTechnique() const
{
    return m_pTechnique;
}

void Effect::Update(const Vector3& cameraPosition, const Matrix& pWorldMatrix, const Matrix& pWorldViewProjectionMatrix)
{
    m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&pWorldViewProjectionMatrix));
}

void Effect::SetCullingMode(D3D11_CULL_MODE cullMode)
{
    if (m_RasterStates.find(cullMode) != m_RasterStates.end()) {
        m_CurrentRasterState = m_RasterStates[cullMode];
    }
}

void Effect::ApplyCullingMode(ID3D11DeviceContext* pContext)
{
    if (m_CurrentRasterState) {
        pContext->RSSetState(m_CurrentRasterState);
    }
}


ID3D11RasterizerState* Effect::GetCurrentRasterizerState() const
{
    return m_CurrentRasterState;
}

ID3D11RasterizerState* Effect::CreateRasterizerState(ID3D11Device* pDevice, D3D11_CULL_MODE cullMode)
{
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = cullMode;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;

    ID3D11RasterizerState* rasterState = nullptr;
    HRESULT hr = pDevice->CreateRasterizerState(&rasterDesc, &rasterState);
    if (FAILED(hr)) {
        // Handle error appropriately
        return nullptr;
    }
    return rasterState;
}

void Effect::CleanupRasterStates()
{
    for (auto& pair : m_RasterStates) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    m_RasterStates.clear();
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    HRESULT result = S_OK;
    ID3D10Blob* pErrorBlob = nullptr;
    ID3DX11Effect* pEffect;

    DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    result = D3DX11CompileEffectFromFile(assetFile.c_str(),
        nullptr,
        nullptr,
        shaderFlags,
        0,
        pDevice,
        &pEffect,
        &pErrorBlob);

    if (FAILED(result))
    {
        if (pErrorBlob != nullptr)
        {
            char* pErrors = (char*)pErrorBlob->GetBufferPointer();

            std::wstringstream ss;
            for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
                ss << pErrors[i];

            OutputDebugStringW(ss.str().c_str());
            pErrorBlob->Release();
            pErrorBlob = nullptr;

            std::wcout << ss.str() << '\n';
        }
        else
        {
            std::wstringstream ss;
            ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
            std::wcout << ss.str() << '\n';
            return nullptr;
        }
    }
    pErrorBlob->Release();
    return pEffect;
}