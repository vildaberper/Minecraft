#include "Mesh.h"

Mesh::Mesh(){
	
}

Mesh::~Mesh(){
	if (bound){
		glDeleteBuffers(1, &vbuffer);
		glDeleteBuffers(1, &nbuffer);
		glDeleteBuffers(1, &cbuffer);
		glDeleteBuffers(1, &ibuffer);
	}
}

void Mesh::render(){
	if (rebind){
		if (bound){
			glDeleteBuffers(1, &vbuffer);
			glDeleteBuffers(1, &nbuffer);
			glDeleteBuffers(1, &cbuffer);
			glDeleteBuffers(1, &ibuffer);
		}
		glGenBuffers(1, &vbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(GLfloat)* vertices_size,
			vertices_, GL_STATIC_DRAW);

		glGenBuffers(1, &nbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, nbuffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(GLfloat)* normals_size,
			normals_, GL_STATIC_DRAW);

		glGenBuffers(1, &cbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, cbuffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(GLfloat)* texcs_size,
			texcs_, GL_STATIC_DRAW);

		glGenBuffers(1, &ibuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			sizeof(unsigned int)* indices_size,
			indices_, GL_STATIC_DRAW);

		bound = true;
		rebind = false;

		delete vertices_;
		delete normals_;
		delete texcs_;
		delete indices_;
	}
	if (bound){
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, nbuffer);
		glNormalPointer(GL_FLOAT, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, cbuffer);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
		glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}