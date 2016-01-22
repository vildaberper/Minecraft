#include "Camera.h"

Camera::Camera(){

}


Camera::~Camera(){

}

void Camera::look(){
	if (xrot > 90)
		xrot = 90;
	if (xrot < -90)
		xrot = -90;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(85.0f, aspect, near_, far_);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(xrot, 1, 0, 0);
	glRotatef(yrot, 0, 1, 0);
	glTranslatef(Block::RENDER_SIZE * -x, Block::RENDER_SIZE * -y - Block::RENDER_SIZE * height, Block::RENDER_SIZE * -z);
}