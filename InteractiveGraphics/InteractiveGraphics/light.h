#pragma once
#include "v3.h"
class Light
{
private:
	V3 position;
	V3 color;
public:
	Light();
	Light(const V3 &position, const V3 &color);
	~Light();
};

