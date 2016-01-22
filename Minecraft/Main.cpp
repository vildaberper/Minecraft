#include <math.h>
#include <minmax.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <map>

#include <noise.h>

#include <direct.h>

#include "GL.h"

#include "SOIL.h"

#include "glsl.h"

#include "ChunkManager.h"
#include "Camera.h"
#include "RayTrace.h"
#include "IO.h"

static const double
walkspeed = 0.1,
jumpvel = 0.2;

struct V{
	ChunkManager* cm = new ChunkManager();
	Camera* cam = new Camera();
	RayTrace rt;

	GLuint atlas;

	cwc::glShaderManager shaderManager;
	cwc::glShader *shader;
	GLuint programObject;

	int vsync = 1;

	bool usecontroller = false;

	bool infocus = true;
	GLFWwindow* window;

	int centerX, centerY;
	bool mouseWarped = false;
	float sensitivity = 0.3;

	double xvel, yvel, zvel;

	bool isonground = false;
	float gravity = 0.01;
	float friction = 0.5;

	std::map<int, bool> keyStates;
};

struct V v;

static void key(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (!v.infocus)
		return;
	if (v.usecontroller)
		return;
	if (action == GLFW_PRESS)
		v.keyStates[key] = true;
	else if (action == GLFW_RELEASE)
		v.keyStates[key] = false;
}

void keyboard(){
	if (v.keyStates[GLFW_KEY_ESCAPE]){
		glfwSetWindowShouldClose(v.window, GL_TRUE);
	}
	if (!v.usecontroller){
		if (v.keyStates[GLFW_KEY_W]){
			float yrotrad = (v.cam->yrot / 180 * PI);
			v.xvel += (v.isonground ? walkspeed : walkspeed * 0.1) * float(sin(yrotrad));
			v.zvel -= (v.isonground ? walkspeed : walkspeed * 0.1) * float(cos(yrotrad));
		}

		if (v.keyStates[GLFW_KEY_S]){
			float yrotrad = (v.cam->yrot / 180 * PI);
			v.xvel -= (v.isonground ? walkspeed : walkspeed * 0.1) * float(sin(yrotrad));
			v.zvel += (v.isonground ? walkspeed : walkspeed * 0.1) * float(cos(yrotrad));
		}

		if (v.keyStates[GLFW_KEY_D]){
			float yrotrad = (v.cam->yrot / 180 * PI);
			v.xvel += (v.isonground ? walkspeed : walkspeed * 0.1) * float(cos(yrotrad));
			v.zvel += (v.isonground ? walkspeed : walkspeed * 0.1) * float(sin(yrotrad));
		}

		if (v.keyStates[GLFW_KEY_A]){
			float yrotrad = (v.cam->yrot / 180 * PI);
			v.xvel -= (v.isonground ? walkspeed : walkspeed * 0.1) * float(cos(yrotrad));
			v.zvel -= (v.isonground ? walkspeed : walkspeed * 0.1) * float(sin(yrotrad));
		}
	}
	if (v.keyStates[GLFW_KEY_SPACE]){
		if (v.isonground)
			v.yvel = jumpvel;
	}
	if (v.keyStates[GLFW_KEY_Q]){
		v.yvel = -jumpvel;
	}
}

float a(float b){
	if (abs(b) < 0.2f)
		return 0;
	return b;
}

void mousebtn(GLFWwindow* window, int button, int action, int mods){
	if (!v.infocus)
		return;
	if (action == GLFW_PRESS){
		if (button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE){
			if (button == GLFW_MOUSE_BUTTON_RIGHT){
				if (v.rt.target != NULL && v.rt.beforeTarget != NULL &&v.rt.target->type != AIR)
					v.cm->setTypeAt(v.rt.btx, v.rt.bty, v.rt.btz, COBBLESTONE);
			}
			else if (button == GLFW_MOUSE_BUTTON_LEFT){
				if (v.rt.target != NULL && v.rt.target->type != AIR)
					v.cm->setTypeAt(v.rt.tx, v.rt.ty, v.rt.tz, AIR);
			}
			else if (button == GLFW_MOUSE_BUTTON_MIDDLE){

			}
		}
	}
}

void joystick(){
	if (!v.infocus)
		return;
	bool wasUsingKeyboard = !v.usecontroller;
	v.usecontroller = false;
	if (glfwJoystickPresent(GLFW_JOYSTICK_1)){
		int buttoncount;
		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttoncount);

		int axescount;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axescount);

		if (buttoncount >= 12 && axescount >= 5){
			for (int i = 0; i < buttoncount; i++){
				if (buttons[i])
					v.usecontroller = true;
			}
			for (int i = 0; i < axescount; i++){
				if (a(axes[i]) != 0)
					v.usecontroller = true;
			}
			if (!v.usecontroller){
				if (!wasUsingKeyboard)
					v.keyStates.clear();
				return;
			}

			/*for (int i = 0; i < buttoncount; i++){
			if (buttons[i])
			printf(("Button: " + std::to_string(i) + ":" + std::to_string(buttons[i]) + "\n").c_str());
			}
			for (int i = 0; i < axescount; i++){
			if (a(axes[i]) != 0)
			printf(("Axis: " + std::to_string(i) + ":" + std::to_string(a(axes[i] * 100)) + "\n").c_str());
			}*/

			if (wasUsingKeyboard)
				v.keyStates.clear();

			if (buttons[5])
				mousebtn(v.window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0);
			if (buttons[4])
				mousebtn(v.window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);

			/*if (buttons[9])
				mousebtn(v.window, GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, 0);*/

			v.keyStates[GLFW_KEY_Q] = buttons[8] == 1;
			v.keyStates[GLFW_KEY_SPACE] = buttons[9] == 1;

			float yrotrad = (v.cam->yrot / 180 * PI);
			v.xvel += a(axes[0]) * walkspeed * float(cos(yrotrad));
			v.zvel += a(axes[0]) * walkspeed * float(sin(yrotrad));
			v.xvel -= a(axes[1]) * walkspeed * float(sin(yrotrad));
			v.zvel += a(axes[1]) * walkspeed * float(cos(yrotrad));

			v.cam->xrot += 3 * (axes[3] < 0 ? -1 : 1) * pow(a(axes[3]), 2);
			v.cam->yrot += 3 * (axes[4] < 0 ? -1 : 1) * pow(a(axes[4]), 2);
		}
	}
}

void cursor(GLFWwindow* window, double x, double y){
	if (!v.infocus)
		return;
	if (v.mouseWarped){
		v.mouseWarped = false;
		return;
	}

	float diffx = float(x - v.centerX) * v.sensitivity;
	float diffy = float(y - v.centerY) * v.sensitivity;

	glfwSetCursorPos(v.window, v.centerX, v.centerY);
	v.mouseWarped = true;

	v.cam->xrot += diffy;
	v.cam->yrot += diffx;
}

float t(float v){
	if (v > 0)
		return v + 0.2f;
	else
		return v - 0.2f;
}

bool tr(double x, long y, double z){
	Block* b = v.cm->blockAt(ceil(x) - 1, y - 1, ceil(z) - 1);

	if (b != NULL){
		return !b->isBlock();
	}
	return true;
}

long ground(){
	long y = round(v.cam->y);

	while (tr(v.cam->x, y, v.cam->z) && y >= 0){
		y--;
	}
	return y;
}

void phys(){
	if (v.cam == NULL || v.cm == NULL){
		return;
	}

	v.yvel -= v.gravity;

	v.yvel = max(-0.50, v.yvel);

	if (v.isonground){
		v.xvel *= v.friction;
		v.zvel *= v.friction;
	}
	else{
		v.xvel *= 0.9;
		v.zvel *= 0.9;
	}

	int g = ground();

	v.isonground = v.cam->y <= g;
	if (v.cam->y + v.yvel < g && v.yvel < 0){
		v.yvel = 0;
		v.cam->y = g;
	}
	else if (v.yvel > 0 && !tr(v.cam->x + v.xvel, v.cam->y + v.cam->height + 1.0 + v.yvel, v.cam->z)){
		v.yvel = 0;
	}
	if (!tr(v.cam->x + v.xvel, v.cam->y + v.yvel + 1, v.cam->z + v.zvel) && tr(v.cam->x + v.xvel, v.cam->y + v.yvel + v.cam->height + 2.0, v.cam->z + v.zvel)){
		v.yvel = max(0.08, v.yvel + 0.01);
	}
	//v.isonground = true;

	bool step = true;
	bool collided = false;
	float nxv = v.xvel, nzv = v.zvel;
	for (int i = 1; i <= ceil(v.cam->height) + 1; i++){
		bool both = true;
		if (!tr(v.cam->x + t(v.xvel), v.cam->y + i, v.cam->z)){
			both = false;
			if (i != 1)
				step = false;
			if (i != ceil(v.cam->height) + 1){
				nxv = 0;
				collided = true;
			}
		}
		if (!tr(v.cam->x, v.cam->y + i, v.cam->z + t(v.zvel))){
			both = false;
			if (i != 1)
				step = false;
			if (i != ceil(v.cam->height) + 1){
				nzv = 0;
				collided = true;
			}
		}
		if (both && !tr(v.cam->x + t(v.xvel), v.cam->y + i, v.cam->z + t(v.zvel))){
			if (i != 1)
				step = false;
			if (i != ceil(v.cam->height) + 1){
				nzv = 0;
				collided = true;
			}
		}
	}
	v.xvel = nxv;
	v.zvel = nzv;

	/*if (collided && step && tr(v.cam->x, v.cam->y + v.cam->height + 1, v.cam->z))
		v.yvel = min(0.10f, v.yvel + 0.90f);*/

	v.cam->x += v.xvel;
	v.cam->z += v.zvel;
	v.cam->y += v.yvel;
}

void display(void){
	phys();
	keyboard();
	joystick();

	glClearColor(0.28, 0.32, 0.95, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (v.shader) v.shader->begin();
	GLint ticks = glGetUniformLocation(v.programObject, "ticks");
	if (ticks != -1)
	{
		glUniform1f(ticks, v.cm->framecount);
	}

	v.cam->look();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	//glDisable(GL_BLEND);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0, 1, 0);
	glEnable(GL_LIGHTING);
	GLfloat plight_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat plight_diffuse[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat plight_specular[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat plight_position[] = { -1, 0, -1, 1.0f };
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, plight_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, plight_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, plight_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, plight_position);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glPopMatrix();
	
	//glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, v.atlas);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	v.cm->frame(v.cam);
	//glDisable(GL_TEXTURE_2D);

	v.rt = RayTrace::trace(v.cam, v.cm);
	if (v.rt.target != NULL && v.rt.target->type != AIR){
		glPushMatrix();
		glTranslatef(Block::RENDER_SIZE * v.rt.tx + Block::RENDER_SIZE / 2, Block::RENDER_SIZE * v.rt.ty + Block::RENDER_SIZE / 2, Block::RENDER_SIZE * v.rt.tz + Block::RENDER_SIZE / 2);
		glColor3f(0, 0, 0);
		glLineWidth(2);
		glutWireCube(Block::RENDER_SIZE + 0.005);
		glPopMatrix();
	}

	glPushMatrix();
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, -0.5f);
	glColor3f(0, 0, 0);
	float size = 0.01f;
	glLineWidth(1);
	glBegin(GL_LINES);
	glVertex3f(-size, 0, 0);
	glVertex3f(size, 0, 0);
	glVertex3f(0, -size, 0);
	glVertex3f(0, size, 0);
	glEnd();
	glPopMatrix();

	if (v.shader) v.shader->end();
}

void focus(GLFWwindow* window, int focused){
	if (v.infocus = focused == GL_TRUE){
		v.mouseWarped = true;
		glfwSetCursorPos(v.window, v.centerX, v.centerY);
	}
	else{
		v.keyStates.clear();
	}
}

void error_callback(int error, const char* description){
	fputs(description, stderr);
}

int main(int argc, char **argv){
	int seed = 1;
	v.cam->x = 205;
	v.cam->y = 50;
	v.cam->z = -34;
	v.cam->height = 1.8;

	_mkdir("world");

	ifstream myfile("player.txt");
	if (myfile.is_open())
	{
		myfile >> v.cam->x;
		myfile >> v.cam->y;
		myfile >> v.cam->z;
		myfile >> v.cam->xrot;
		myfile >> v.cam->yrot;
		myfile >> v.cam->fov;
		myfile >> seed;
		myfile >> v.cm->viewDistance;
		myfile >> v.vsync;
		myfile.close();
	}
	else{
		printf("Unable to load player\n");
	}
	v.cm->groundm->SetSeed(seed);
	v.cm->mtmm->SetSeed(seed);
	v.cm->hillsm->SetSeed(seed);
	v.cm->cavem->SetSeed(seed);

	v.cm->load(v.cam);
	v.cm->unload(v.cam);
	v.cm->generate();
	v.cm->updateMesh();

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);
	v.window = glfwCreateWindow(1280, 720, "Minecraft", NULL, NULL);
	if (!v.window){
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(v.window);
	glfwSwapInterval(v.vsync);

	glfwSetKeyCallback(v.window, key);
	glfwSetCursorPosCallback(v.window, cursor);
	glfwSetMouseButtonCallback(v.window, mousebtn);
	glfwSetWindowFocusCallback(v.window, focus);

	if (glfwJoystickPresent(GLFW_JOYSTICK_1)){
		printf(("Detected controller: " + std::string(glfwGetJoystickName(GLFW_JOYSTICK_1)) + "\n").c_str());
	}

	glfwSetInputMode(v.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glewInit();
	glEnable(GL_TEXTURE_2D);
	v.atlas = SOIL_load_OGL_texture
		(
		"atlas.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);

	if (0 == v.atlas)
	{
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}
	else{
		printf("Texture bound\n");
		printf((std::to_string(v.atlas)).c_str());
	}

	v.shader = v.shaderManager.loadfromFile("vertexshader.txt", "fragmentshader.txt");
	if (v.shader == 0)
		std::cout << "Error Loading, compiling or linking shader\n";
	v.programObject = v.shader->GetProgramObject();

	GLint tex = glGetUniformLocation(v.programObject, "texture");
	if (tex != -1){
		glUniform1i(tex, 0);
		printf("Texture sent to shader\n");
	}

	while (!glfwWindowShouldClose(v.window)){
		float ratio;
		int width, height;
		glfwGetFramebufferSize(v.window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);

		v.centerX = width / 2;
		v.centerY = height / 2;

		glClear(GL_COLOR_BUFFER_BIT);
		v.cam->aspect = ratio;
		v.cam->look();
		display();
		glfwSwapBuffers(v.window);
		glfwPollEvents();
	}
	glfwDestroyWindow(v.window);
	glfwTerminate();

	for (auto const &ent1 : v.cm->chunks) {
		for (auto const &ent2 : ent1.second) {
			for (auto const &ent3 : ent2.second) {
				Chunk* chunk = ent3.second;
				if (chunk != NULL && chunk->modified)
					IO::saveChunk(chunk);
			}
		}
	}

	ofstream omyfile("player.txt");
	if (omyfile.is_open())
	{
		omyfile << v.cam->x << "\n";
		omyfile << v.cam->y << "\n";
		omyfile << v.cam->z << "\n";
		omyfile << v.cam->xrot << "\n";
		omyfile << v.cam->yrot << "\n";
		omyfile << v.cam->fov << "\n";
		omyfile << seed << "\n";
		omyfile << v.cm->viewDistance << "\n";
		omyfile << v.vsync << "\n";
		omyfile.close();
	}
	else{
		printf("Unable to save player\n");
	}

	delete v.cm;
	delete v.cam;
	exit(EXIT_SUCCESS);
	return 0;
}