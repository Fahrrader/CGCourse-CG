#pragma once


#include "triangle_rasterization.h"


class ZCulling: public TriangleRasterization
{
public:
	ZCulling(unsigned short width, unsigned short height, std::string obj_file);
	virtual ~ZCulling();

	void DrawScene();
	void Clear();
	void DrawZBuffer();

protected:
	void DrawTriangle(float4 triangle[3]);
	void SetDepth(unsigned short x, unsigned short y, float depth);
	void SetPixel(unsigned short x, unsigned short y, color m_color, float depth);
	std::vector<float> depth_buffer;
};
