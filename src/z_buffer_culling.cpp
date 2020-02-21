#include "z_buffer_culling.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>


ZCulling::ZCulling(unsigned short width, unsigned short height, std::string obj_file) : TriangleRasterization(width, height, obj_file)
{
}

ZCulling::~ZCulling()
{
}

void ZCulling::Clear() {
	BlackImage::Clear();
	depth_buffer.resize(width * height);
	for (size_t i = 0; i < depth_buffer.size(); i++) {
		depth_buffer[i] = INFINITY;
	}
}

void ZCulling::DrawScene() {
	parser->Parse();

	x_center = width / 2.0f;
	y_center = height / 2.0f;
	float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	float radius = std::min(x_center, y_center) - 40.0f;

	float3 rotAngleXYZ = { 0, 0, 0 };
	rotAngleXYZ.x *= M_PI / 180.0f;
	rotAngleXYZ.y *= M_PI / 180.0f;
	rotAngleXYZ.z *= M_PI / 180.0f;

#pragma region World
	// Translate
	float4x4 translate = translation_matrix(float3{ 0, 0, 0 });

	// Scale
	float4x4 scale = scaling_matrix(float3{ 1, 1, 1 });

	// Rotate
	float4x4 rotateX {
		{1, 0, 0, 0},
		{0, cos(rotAngleXYZ.x), -sin(rotAngleXYZ.x), 0},
		{0, sin(rotAngleXYZ.x), cos(rotAngleXYZ.x), 0},
		{0, 0, 0, 1}
	}; rotateX = transpose(rotateX);

	float4x4 rotateY {
		{cos(rotAngleXYZ.y), 0, sin(rotAngleXYZ.y), 0},
		{0, 1, 0, 0},
		{-sin(rotAngleXYZ.y), 0, cos(rotAngleXYZ.y), 0},
		{0, 0, 0, 1}
	}; rotateY = transpose(rotateY);

	float4x4 rotateZ {
		{cos(rotAngleXYZ.z), -sin(rotAngleXYZ.z), 0, 0},
		{sin(rotAngleXYZ.z), cos(rotAngleXYZ.z), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	}; rotateZ = transpose(rotateZ);

	// Combine
	float4x4 rotateXYZ = mul(rotateX, rotateY, rotateZ);
	float4x4 world = mul(translate, rotateXYZ, scale);

#pragma endregion

#pragma region Camera
	float3 vEye{ 0, 0, -2 };
	float3 vAt{ 0, 0, 0 };
	float3 vUp{ 0, 1, 0 };

	float3 zAxis = normalize(vEye - vAt);
	float3 xAxis = normalize(cross(vUp, zAxis));
	float3 yAxis = cross(zAxis, xAxis);

	float4x4 view = {
		{xAxis.x, yAxis.x, zAxis.x, 0},
		{xAxis.y, yAxis.y, zAxis.y, 0},
		{xAxis.z, yAxis.z, zAxis.z, 0},
		{-dot(xAxis, vEye), -dot(yAxis, vEye), -dot(zAxis, vEye), 1}
	};

	float fovAngleY = 60.0f * M_PI / 180.0f;
	float zFarPlane = 10, zNearPlane = 1;

	float yScale = 1.0f / tan(fovAngleY / 2.0f);
	float xScale = yScale / aspect_ratio;

	float4x4 projection {
		{xScale, 0, 0, 0},
		{0, yScale, 0, 0},
		{0, 0, zFarPlane / (zFarPlane - zNearPlane), 1},
		{0, 0, -(zNearPlane * zFarPlane) / (zFarPlane - zNearPlane), 0}
	};

#pragma endregion

	float4x4 interpolator = mul(projection, view, world);

	for (auto face : parser->GetFaces()) 
	{
		float4 translated[3];
		for (unsigned short i = 0; i < 3; i++) 
		{
			translated[i] = mul(interpolator, face.vertexes[i]);
			translated[i] /= translated[i].w;
			translated[i].x = x_center + x_center * translated[i].x;
			translated[i].y = y_center - y_center * translated[i].y;
		}
		DrawTriangle(translated);
	}
}

void ZCulling::DrawTriangle(float4 triangle[3]) 
{
	float2 leftmost = max(float2{ 0, 0 },
		min(min(triangle[0].xy(), triangle[1].xy()), triangle[2].xy()));
	float2 rightmost = min(float2{ static_cast<float>(width - 1), static_cast<float>(height - 1) },
		max(max(triangle[0].xy(), triangle[1].xy()), triangle[2].xy()));

	float edge = EdgeFunction(triangle[0].xy(), triangle[1].xy(), triangle[2].xy());
	bool drawWires = false;

	for (float x = leftmost.x; x <= rightmost.x; x++) 
	{
		for (float y = leftmost.y; y <= rightmost.y; y++) 
		{
			float edge0 = EdgeFunction(triangle[0].xy(), triangle[1].xy(), float2{ x, y });
			float edge1 = EdgeFunction(triangle[1].xy(), triangle[2].xy(), float2{ x, y });
			float edge2 = EdgeFunction(triangle[2].xy(), triangle[0].xy(), float2{ x, y });

			if (edge0 > 0.0f && edge1 > 0.0f && edge2 > 0.0f) {
				drawWires = true;
				float u = edge0 / edge;
				float v = edge1 / edge;
				float w = edge2 / edge;
				float z = u * (-triangle[0].z) + v * (-triangle[1].z) + w * (-triangle[2].z);
				SetPixel(x, y, color(255 * u, 255 * v, 255 * w), z);
			}
		}
	}

	if (drawWires) {
		DrawLine(static_cast<unsigned short>(triangle[0].x),
			static_cast<unsigned short>(triangle[0].y),
			static_cast<unsigned short>(triangle[1].x),
			static_cast<unsigned short>(triangle[1].y),
			color(255, 255, 255));
		DrawLine(static_cast<unsigned short>(triangle[1].x),
			static_cast<unsigned short>(triangle[1].y),
			static_cast<unsigned short>(triangle[2].x),
			static_cast<unsigned short>(triangle[2].y),
			color(255, 255, 255));
		DrawLine(static_cast<unsigned short>(triangle[2].x),
			static_cast<unsigned short>(triangle[2].y),
			static_cast<unsigned short>(triangle[0].x),
			static_cast<unsigned short>(triangle[0].y),
			color(255, 255, 255));
	}
}

void ZCulling::DrawZBuffer() 
{
	for (size_t i = 0; i < width * height; i++) 
	{
		float pixelVal = 255 - depth_buffer[i];
		if (pixelVal < 0)
			pixelVal = 0.0f;

		frame_buffer[i] = color(static_cast<unsigned char>(pixelVal), static_cast<unsigned char>(pixelVal), static_cast<unsigned char>(pixelVal));
	}
}

void ZCulling::SetPixel(unsigned short x, unsigned short y, color m_color, float depth) 
{
	unsigned int ix = y * width + x;

	if (ix >= 0 && ix < frame_buffer.size()) {
		if (depth_buffer[ix] > depth) {
			frame_buffer[ix] = m_color;
			depth_buffer[ix] = depth;
		}
	}
}

void ZCulling::SetDepth(unsigned short x, unsigned short y, float depth) 
{
	unsigned int ix = y * width + x;

	if (ix >= 0 && ix < depth_buffer.size()) {
		depth_buffer[ix] = depth;
	}
}