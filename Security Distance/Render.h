#pragma once

class Model;

#include <GL/glew.h>
#include <GLFW/glfw3.h>


class Render
{
public:
	Render();
	~Render();

	void setModel(Model*);

	void setView(double, double); // longitude, latitude
	void draw();
	GLFWwindow* window();

private:
	Model* _mdl;
	int _width, _height;
	GLFWwindow* _wndw;
	GLuint _vrtxArrayId, _axisBuffer, _colorBuffer;

	// shaders: 3D
	GLuint _prgmId, _matrixId;
	// shaders: text
	GLuint _textPrgmId, _textMatrixId;

	bool _allGood;

	void initVAO();
	GLuint initShaders(const char*, const char*);
	void initAxis();
	void initText2D(const char*);
	void printText2D(const char* text, int x, int y, int size);
	void cleanupText2D();

	void drawAxis();
	void drawModel();

};

