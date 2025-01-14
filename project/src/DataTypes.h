#pragma once
#include "Math.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position;
		Vector2 uv;
		Vector3 normal; 
		Vector3 tangent; 
	};

	struct Vertex_Out
	{
		Vector4 position;
		Vector2 uv;
		Vector3 normal;
		Vector3 tangent;
		Vector3 viewDirection;
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	enum class DisplayMode {
		FinalColor,
		DepthBuffer,
		ShadingMode
	};

	enum class ShadingMode
	{
		ObservedArea,
		Diffuse,
		Specular,
		Combined
	};

	enum FilteringTechnique
	{
		Point,
		Linear,
		Anisotropic
	};

	enum RenderingBackendType
	{
		Software,
		Hardware
	};


	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{};	};
}