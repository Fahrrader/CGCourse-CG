#pragma once


#include "read_obj.h"


class Projection: public ReadObj
{
public:
	Projection(unsigned short width, unsigned short height, std::string obj_file);
	virtual ~Projection();

	void DrawScene();
protected:
	float x_center, y_center;

	void DrawTriangle(float4 triangle[3], color m_color);
};
