#include "Mesh3D.h"
#include "Camera.h"
#include "Texture.h"
#include <memory.h>
constexpr float eps = float( 1e-6);
Mesh3D::Mesh3D(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Effect* pEffect) : m_pEffect(pEffect)
{
	m_pUMesh = std::unique_ptr<Mesh>(new Mesh());
	m_pUMesh->vertices = vertices;
	m_pUMesh->indices = indices;
	m_pUMesh->primitiveTopology = PrimitiveTopology::TriangleStrip;


	//1. Create Vertex Layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//2. Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT result{ pDevice->CreateInputLayout
		(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pVertexLayout )};
	if (FAILED(result)) return;

	//3. Create vertex buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result)) return;

	//4. Create index buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result)) return;
}

Mesh3D::~Mesh3D()
{
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}

	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}

	if (m_pVertexLayout) 
	{
		m_pVertexLayout->Release();
		m_pVertexLayout = nullptr;
	}
}

void Mesh3D::RenderGPU(const Vector3& cameraPosition, const Matrix& pWorldMatrix, const Matrix& pWorldViewProjectionMatrix, ID3D11DeviceContext* pDeviceContext) const
{	
	//1. Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//3. Set vertex buffer
	constexpr UINT stride{ sizeof(Vertex) };
	constexpr UINT offset{};
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Set World View Projection Matrix
	m_pEffect->Update(cameraPosition, pWorldMatrix, pWorldViewProjectionMatrix);
	pDeviceContext->RSSetState(m_pEffect->GetCurrentRasterizerState());

	//6. Draw
	D3DX11_TECHNIQUE_DESC techniqueDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techniqueDesc);
	for (UINT p{}; p < techniqueDesc.Passes; ++p)
	{
		auto passIndexPoint = m_pEffect->GetTechnique()->GetPassByIndex(p);
		passIndexPoint->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		if (passIndexPoint) passIndexPoint->Release();
	}
}

void Mesh3D::RenderCPU(int width, int height, ShadingMode shadingMode, DisplayMode displayMode, CullingMode cullingMode, const Camera& camera, bool isNormalMap, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels, float* pDepthBufferPixels) const
{
	bool isTriangleList = m_pUMesh->primitiveTopology == PrimitiveTopology::TriangleStrip;
	// Parallelize over triangles
#pragma omp parallel for
	for (int inx = 0; inx < m_pUMesh->indices.size(); inx += (isTriangleList ? 3 : 1)) {

		auto t0 = m_pUMesh->indices[inx];
		auto t1 = m_pUMesh->indices[inx + 1];
		auto t2 = m_pUMesh->indices[inx + 2];

		////Perform clipping 
		//std::vector<Vertex_Out> clippedVertices;
		//std::vector<uint32_t> clippedIndices;
		//ClipTriangle(mesh.vertices_out[t0], mesh.vertices_out[t1], mesh.vertices_out[t2], clippedVertices, clippedIndices);
		//
		//if (clippedVertices.size() < 3) continue; // If there are not enough vertices left after clipping, skip this triangle

		// Skip degenerate triangles
		if (t0 == t1 || t1 == t2 || t2 == t0) continue;

		// Vertex positions
		auto v0 = m_pUMesh->vertices_out[t0].position;
		auto v1 = m_pUMesh->vertices_out[t1].position;
		auto v2 = m_pUMesh->vertices_out[t2].position;

		// Skip if any vertex is behind the camera (w < 0)
		if (v0.w < 0 || v1.w < 0 || v2.w < 0) continue;

		if ((v0.x < -1 || v0.x > 1) || (v1.x < -1 || v1.x > 1) || (v2.x < -1 || v2.x > 1)
			|| ((v0.y < -1 || v0.y > 1) || (v1.y < -1 || v1.y > 1) || (v2.y < -1 || v2.y > 1))
			|| ((v0.z < 0 || v0.z > 1) || (v1.z < 0 || v1.z > 1) || (v2.z < 0 || v2.z > 1))) continue;



		// Backface culling (skip if the triangle is facing away from the camera)
		//if (cullingMode == CullingMode::Back)
		//{
		//	Vector3 edge0 = v1 - v0;
		//	Vector3 edge1 = v2 - v0;
		//	Vector3 normal = Vector3::Cross(edge0, edge1);
		//	if (Vector3::Dot(normal.Normalized(), camera.forward) <= 0) continue;
		//}
		//else if (cullingMode == CullingMode::Front)
		//{
		//	Vector3 edge0 = v1 - v0;
		//	Vector3 edge1 = v2 - v0;
		//	Vector3 normal = Vector3::Cross(edge0, edge1);
		//	if (Vector3::Dot(normal.Normalized(), camera.forward) > 0.f) continue;
		//}


		// Transform coordinates to screen space
		v0.x *= width;
		v1.x *= width;
		v2.x *= width;
		v0.y *= height;
		v1.y *= height;
		v2.y *= height;


		// Compute bounding box of the triangle
		int minX = std::max(0, static_cast<int>(std::floor(std::min({ v0.x, v1.x, v2.x }))));
		int maxX = std::min(width, static_cast<int>(std::ceil(std::max({ v0.x, v1.x, v2.x }))));
		int minY = std::max(0, static_cast<int>(std::floor(std::min({ v0.y, v1.y, v2.y }))));
		int maxY = std::min(height, static_cast<int>(std::ceil(std::max({ v0.y, v1.y, v2.y }))));


		if (displayMode == DisplayMode::BoundingBox)
		{
			Uint32 color = SDL_MapRGB(pBackBuffer->format, 255, 255, 255);
			SDL_Rect rect;
			rect.x = minX;
			rect.y = minY;
			rect.w = maxX - minX;
			rect.h = maxY - minY;
			SDL_FillRect(pBackBuffer, &rect, color);
		}
		else
		{
			// Edge vectors for barycentric coordinates
			auto e0 = v2 - v1;
			auto e1 = v0 - v2;
			auto e2 = v1 - v0;

			Vector2 edge0_2D(e0.x, e0.y);
			Vector2 edge1_2D(e1.x, e1.y);
			Vector2 edge2_2D(e2.x, e2.y);

			float wProduct = v0.w * v1.w * v2.w;

			auto area = std::abs(Vector2::Cross(edge0_2D, edge1_2D));

			// Parallelize over rows of pixels (py)
#pragma omp parallel for
			for (int py = minY; py < maxY; ++py) {
				for (int px = minX; px < maxX; ++px) {
					ColorRGB finalColor;
					auto P = Vector2(px + 0.5f, py + 0.5f);

					auto p0 = P - Vector2(v1.x, v1.y);
					auto p1 = P - Vector2(v2.x, v2.y);
					auto p2 = P - Vector2(v0.x, v0.y);

					auto weightP0 = Vector2::Cross(edge0_2D, p0) / area;
					auto weightP1 = Vector2::Cross(edge1_2D, p1) / area;
					auto weightP2 = Vector2::Cross(edge2_2D, p2) / area;

					auto totalArea = weightP0 + weightP1 + weightP2;
					if (!(abs(totalArea - 1) <= eps) && !(abs(totalArea + 1) <= eps)) continue;

					if (cullingMode == CullingMode::Back)
					{
						if (!(weightP0 >= 0.f && weightP1 >= 0.f && weightP2 >= 0.f)) continue;
					}
					else if (cullingMode == CullingMode::Front)
					{
						if (!(weightP0 < 0.f && weightP1 < 0.f && weightP2 < 0.f)) continue;
					}
					else if (cullingMode == CullingMode::No)
					{
						if (!((weightP0 < 0.f && weightP1 < 0.f && weightP2 < 0.f) || (weightP0 >= 0.f && weightP1 >= 0.f && weightP2 >= 0.f))) continue;
					}

					
					float reciprocalTotalArea = 1.0f / totalArea;

					float interpolationScale0 = weightP0 * reciprocalTotalArea;
					float interpolationScale1 = weightP1 * reciprocalTotalArea;
					float interpolationScale2 = weightP2 * reciprocalTotalArea;

					// Compute z-buffer value for depth testing
					float zBufferValue = 1.f / (1.f / v0.z * interpolationScale0 +
						1.f / v1.z * interpolationScale1 +
						1.f / v2.z * interpolationScale2);

					if (zBufferValue < 0 || zBufferValue > 1) continue;



					int pixelIndex = px + (py * width);
					if (zBufferValue >= pDepthBufferPixels[pixelIndex]) continue;

					pDepthBufferPixels[pixelIndex] = zBufferValue;

					// Interpolated depth for final color calculation
					float interpolatedDepth = wProduct / (v1.w * v2.w * interpolationScale0 +
						v0.w * v2.w * interpolationScale1 +
						v0.w * v1.w * interpolationScale2);
					if (interpolatedDepth <= 0) continue;

					// Texture sampling
					Vertex_Out pixelVertex;

					pixelVertex.position = (m_pUMesh->vertices[t0].position.ToPoint4() + m_pUMesh->vertices[t1].position.ToPoint4() + m_pUMesh->vertices[t2].position.ToPoint4()) / 3.f;
					pixelVertex.position.z = zBufferValue;
					pixelVertex.position.w = interpolatedDepth;


					pixelVertex.uv = Vector2::Interpolate(m_pUMesh->vertices_out[t0].uv, m_pUMesh->vertices_out[t1].uv, m_pUMesh->vertices_out[t2].uv,
						v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);

					pixelVertex.normal = Vector3::Interpolate(m_pUMesh->vertices_out[t0].normal, m_pUMesh->vertices_out[t1].normal, m_pUMesh->vertices_out[t2].normal,
						v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);
					pixelVertex.normal.Normalize();


					pixelVertex.tangent = Vector3::Interpolate(m_pUMesh->vertices_out[t0].tangent, m_pUMesh->vertices_out[t1].tangent, m_pUMesh->vertices_out[t2].tangent,
						v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);
					pixelVertex.tangent.Normalize();

					pixelVertex.viewDirection = Vector3::Interpolate(m_pUMesh->vertices_out[t0].viewDirection, m_pUMesh->vertices_out[t1].viewDirection, m_pUMesh->vertices_out[t2].viewDirection,
						v0.w, v1.w, v2.w, interpolationScale0, interpolationScale1, interpolationScale2, interpolatedDepth, wProduct);
					pixelVertex.viewDirection.Normalize();

					// If texture mapping is enabled, sample the texture
					if (displayMode == DisplayMode::DepthBuffer)
					{
						auto clampedValue = std::clamp(Remap(zBufferValue, 0.995f, 1.f, 0.f, 1.f), 0.f, 1.f);
						finalColor = ColorRGB(clampedValue, clampedValue, clampedValue);
					}
					if (displayMode == DisplayMode::ShadingMode)
					{
						finalColor = PixelShading(pixelVertex, shadingMode, isNormalMap);
					}

					finalColor.MaxToOne();

					pBackBufferPixels[pixelIndex] = SDL_MapRGB(pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255.f),
						static_cast<uint8_t>(finalColor.g * 255.f),
						static_cast<uint8_t>(finalColor.b * 255.f));
				}
			}
		}

	}
}

void Mesh3D::SetCullingMode(CullingMode cullingMode, ID3D11DeviceContext* context)
{	
	switch (cullingMode)
	{
	case CullingMode::Back:
		m_pEffect->SetCullingMode(D3D11_CULL_BACK);
		m_pEffect->ApplyCullingMode(context);
		break;
	case CullingMode::Front:
		m_pEffect->SetCullingMode(D3D11_CULL_FRONT);
		m_pEffect->ApplyCullingMode(context);
		break;
	case CullingMode::No:
		m_pEffect->SetCullingMode(D3D11_CULL_NONE);
		m_pEffect->ApplyCullingMode(context);
		break;
	}
}

void Mesh3D::VertexTransformationFunction(const Camera& camera, const Matrix& rotationMatrix)
{
	// Precompute transformation matrix
	auto rotatedWorldMatrix = rotationMatrix * m_pUMesh->worldMatrix;
	auto overallMatrix = rotatedWorldMatrix * camera.viewMatrix * camera.projectionMatrix;

	// Resize vertices_out to match input vertices
	m_pUMesh->vertices_out.resize(m_pUMesh->vertices.size());

	// Transform vertices in parallel
#pragma omp parallel for
	for (size_t i = 0; i < m_pUMesh->vertices.size(); ++i) {
		m_pUMesh->vertices_out[i].normal = rotatedWorldMatrix.TransformVector(m_pUMesh->vertices[i].normal).Normalized();

		m_pUMesh->vertices_out[i].tangent = rotatedWorldMatrix.TransformVector(m_pUMesh->vertices[i].tangent).Normalized();


		auto rotatedWorldPosition = rotatedWorldMatrix.TransformPoint(m_pUMesh->vertices[i].position);
		m_pUMesh->vertices_out[i].viewDirection = rotatedWorldPosition - camera.origin;
		m_pUMesh->vertices_out[i].viewDirection.Normalize();

		Vector4 viewSpacePosition = overallMatrix.TransformPoint(m_pUMesh->vertices[i].position.ToVector4());
		Vector4 projectionSpacePosition = viewSpacePosition / viewSpacePosition.w;

		projectionSpacePosition.x = projectionSpacePosition.x * 0.5f + 0.5f;
		projectionSpacePosition.y = (1.0f - projectionSpacePosition.y) * 0.5f;

		m_pUMesh->vertices_out[i].position = projectionSpacePosition;
		m_pUMesh->vertices_out[i].uv = m_pUMesh->vertices[i].uv;
	}
}

ColorRGB Mesh3D::PixelShading(Vertex_Out& v, ShadingMode shadingMode, bool isNormalMap) const
{
	ColorRGB finalColor;

	Vector3 lightDirection = { .577f, -.577f,  .577f };
	constexpr float lightIntensity = 7.f;
	constexpr float shininess = 25.f;
	constexpr ColorRGB ambient = { .03f,.03f,.03f };

	if (isNormalMap)
	{
		Vector3 binormal = Vector3::Cross(v.normal, v.tangent);
		Matrix tangentSpaceAxis = Matrix{ v.tangent, binormal, v.normal, Vector3::Zero };

		ColorRGB normalMapSample = m_pEffect->GetNormalTexture()->Sample(v.uv);
		v.normal = (v.tangent * (2.f * normalMapSample.r - 1.f) + binormal * (2.f * normalMapSample.g - 1.f) + v.normal * (2.f * normalMapSample.b - 1.f)).Normalized();
	}

	float cosOfAngle{ Vector3::Dot(v.normal, -lightDirection) };

	if (cosOfAngle < 0.f) return ColorRGB(0.f, 0.f, 0.f);

	ColorRGB observedArea = { cosOfAngle, cosOfAngle, cosOfAngle };

	ColorRGB diffuse = Lambert(m_pEffect->GetDiffuseTexture()->Sample(v.uv));

	ColorRGB gloss = m_pEffect->GetGlossinessTexture()->Sample(v.uv);
	float exp = gloss.r * shininess;

	ColorRGB specular = Phong(m_pEffect->GetSpecularTexture()->Sample(v.uv), exp, -lightDirection, v.viewDirection, v.normal);

	switch (shadingMode)
	{
	case ShadingMode::ObservedArea:
		finalColor += observedArea;
		break;

	case ShadingMode::Diffuse:
		finalColor += diffuse * observedArea * lightIntensity;
		break;

	case ShadingMode::Specular:
		finalColor += specular;
		break;

	case ShadingMode::Combined:
		finalColor += ambient + specular + diffuse * observedArea * lightIntensity;
		break;
	}

	return finalColor;
}
