#pragma once
#include <vector>

struct NewColor
{
	float r, g, b;

	NewColor();
	NewColor(float r, float g, float b);
	~NewColor();
};

class Image
{
public:
	Image(int width, int height);
	~Image();

	NewColor GetColor(int x, int y) const;
	void SetColor(const NewColor& color, int x, int y);

	void Export(const char* path);

private:
	int m_width;
	int m_height;
	std::vector<NewColor> m_colors;
};
