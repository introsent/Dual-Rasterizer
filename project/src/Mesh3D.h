#pragma once
#include "Vector3.h"
#include "pch.h"
#include "ColorRGB.h"
#include <vector>
#include "Effect.h"
#include "VehicleEffect.h"
#include "DataTypes.h"
#include "Camera.h"
#include "Matrix.h"
using namespace dae;

class Mesh3D final
{
public:
	Mesh3D(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Effect* pEffect);
	~Mesh3D();

	Mesh3D(const Mesh3D& other) = delete;
	Mesh3D& operator=(const Mesh3D& rhs) = delete;
	Mesh3D(Mesh3D&& other) = delete;
	Mesh3D& operator=(Mesh3D&& rhs) = delete;

	void RenderGPU(const Vector3& cameraPosition, const Matrix& pWorldMatrix, const Matrix& pWorldViewProjectionMatrix, ID3D11DeviceContext* pDeviceContext) const;
	void RenderCPU(int width, int height, ShadingMode shadingMode, DisplayMode displayMode, CullingMode cullingMode, const Camera& camera, bool isNormalMap, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels, float* pDepthBufferPixels) const;

	void SetCullingMode(CullingMode cullingMode, ID3D11DeviceContext* context);

	void VertexTransformationFunction(const Camera& camera, const Matrix& rotationMatrix);
	ColorRGB PixelShading(Vertex_Out& v, ShadingMode shadingMode, bool isNormalMap) const;

	inline float Remap(float value, float start1, float stop1, float start2, float stop2) const
	{
		return start2 + (value - start1) * (stop2 - start2) / (stop1 - start1);
	}

	//Materials formulas
	static ColorRGB Lambert(const ColorRGB cd, const float kd = 1)
	{
		const ColorRGB rho = kd * cd;
		return rho / PI;
	}

	static ColorRGB Lambert(const ColorRGB cd, const ColorRGB& kd)
	{
		const ColorRGB rho = kd * cd;
		return rho / PI;
	}

	static ColorRGB Phong(const ColorRGB ks, const float exp, const Vector3& l, const Vector3& v, const Vector3& n)
	{
		const Vector3 reflect = l - (2 * std::max(Vector3::Dot(n, l), 0.f) * n);
		const float cosAlpha = std::max(Vector3::Dot(reflect, v), 0.f);

		return ks * std::powf(cosAlpha, exp);
	}

	
private:
	uint32_t				m_NumIndices{};
	Effect*					m_pEffect;

	ID3D11Buffer*			m_pVertexBuffer{};
	ID3D11InputLayout*		m_pVertexLayout{};
	ID3D11Buffer*			m_pIndexBuffer{};

	std::unique_ptr<Mesh>	m_pUMesh{};
};
