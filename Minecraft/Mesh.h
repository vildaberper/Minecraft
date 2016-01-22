#pragma once

#include <vector>

#include "GL.h"

class Mesh{
public:
	Mesh();
	~Mesh();

	void render();

	GLfloat* vertices_;
	unsigned int vertices_size;

	GLfloat* normals_;
	unsigned int normals_size;

	GLfloat* texcs_;
	unsigned int texcs_size;

	unsigned int* indices_;
	unsigned int indices_size;

	GLuint vbuffer;
	GLuint nbuffer;
	GLuint cbuffer;
	GLuint ibuffer;

	bool rebind = false;
	bool bound = false;

	bool empty = false;
};