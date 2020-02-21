#include "projection.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>


Projection::Projection(unsigned short width, unsigned short height, std::string obj_file) : ReadObj(width, height, obj_file)
{
}

Projection::~Projection()
{
}

#if 1
void Projection::DrawScene() 
{
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
	float4x4 translate = translation_matrix(float3{ 0, 0, 2 });

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
	float3 vEye{ 0, 0, 0 };
	float3 vAt{ 0, 0, 1 };
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
		}
		DrawTriangle(translated, color(255, 255, 255));
	}
}
#endif

void Projection::DrawTriangle(float4 triangle[3], color m_color) 
{
	DrawLine(static_cast<unsigned short>(x_center + x_center * triangle[0].x),
		static_cast<unsigned short>(y_center - y_center * triangle[0].y),
		static_cast<unsigned short>(x_center + x_center * triangle[1].x),
		static_cast<unsigned short>(y_center - y_center * triangle[1].y),
		color(255, 0, 0));
	DrawLine(static_cast<unsigned short>(x_center + x_center * triangle[1].x),
		static_cast<unsigned short>(y_center - y_center * triangle[1].y),
		static_cast<unsigned short>(x_center + x_center * triangle[2].x),
		static_cast<unsigned short>(y_center - y_center * triangle[2].y),
		color(0, 255, 0));
	DrawLine(static_cast<unsigned short>(x_center + x_center * triangle[2].x),
		static_cast<unsigned short>(y_center - y_center * triangle[2].y),
		static_cast<unsigned short>(x_center + x_center * triangle[0].x),
		static_cast<unsigned short>(y_center - y_center * triangle[0].y),
		color(0, 0, 255));
}
