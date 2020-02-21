#include "color_space.h"


ColorSpace::ColorSpace(unsigned short width, unsigned short height): BlackImage(width, height)
{

}

ColorSpace::~ColorSpace()
{
}

void ColorSpace::DrawScene()
{
	for (unsigned short x = 0; x < width; x++) {
		for (unsigned short y = 0; y < height; y++) {
			SetPixel(x, y, color(
				static_cast<unsigned char>(255 * x / width),
				static_cast<unsigned char>(255 * y / height),
				127
			));
		}
	}

	SetPixel(100, 50, color(255, 255, 255));
}

void ColorSpace::SetPixel(unsigned short x, unsigned short y, color color)
{
	unsigned int ix = y * width + x;

	if (ix >= 0 && ix < frame_buffer.size()) {
		frame_buffer[ix] = color;
	}
}

