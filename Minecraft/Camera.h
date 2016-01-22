#pragma once

#include "GL.h"
#include "Block.h"

class Camera{
public:
	Camera();
	~Camera();

	void look();

	double
		x,
		y,
		z,
		height,
		xrot,
		yrot,
		fov = 85,
		aspect;

	float near_ = 0.01,
		far_ = 9999;
};