#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stack>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Utils.h"
using namespace std;

extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

#define numVAOs 1
#define numVBOs 3

// 相机参数
glm::vec3 cameraPos, cameraTarget, cameraUp;
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// variable allocation for display
GLuint mvLoc, projLoc;
// 在全局变量中添加
GLuint alphaLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat;

stack<glm::mat4> mvStack;

// 查看模式
int viewMode = 0; // 0: 默认视图, 1: 查看太阳, 2: 查看地球, 3: 查看月球

void setupVertices(void) {
	float vertexPositions[108] =
	{ -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
	};
	float pyramidPositions[54] =
	{ -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f,    //front
		1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,    //right
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  //back
		-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  //left
		-1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, //LF
		1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f  //RR
	};


	float triangularPrismPositions[72] = {
		0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // ABD
		0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // ADF
		0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // ACE
		0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // AEF
		-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // BCE
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // BDE
		-1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // DEF
		0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // ABC
	};


	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidPositions), pyramidPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangularPrismPositions), triangularPrismPositions, GL_STATIC_DRAW);
}

void updateCamera(double currentTime) {
	switch (viewMode) {
	case 0: // 默认视图
		cameraPos = glm::vec3(0.0f, 0.0f, 18.0f);
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		break;

	case 1: // 查看太阳（从太阳位置向外看）
		cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
		cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		break;

	case 2: // 查看地球
	{
		glm::vec3 earthPos = glm::vec3(sin((float)currentTime) * 4.0, 0.0f, cos((float)currentTime) * 4.0);
		cameraPos = earthPos;
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 看向太阳
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	break;

	case 3: // 查看月球
	{
		// 方法1：使用矩阵乘法重建月球变换（推荐）
		glm::mat4 earthTransform = glm::translate(glm::mat4(1.0f),
			glm::vec3(sin((float)currentTime) * 4.0, 0.0f, cos((float)currentTime) * 4.0));

		// 月球相对于地球的变换
		glm::mat4 moonRelativeTransform = glm::translate(glm::mat4(1.0f),
			glm::vec3(0.0f, sin((float)currentTime) * 2.0, cos((float)currentTime) * 2.0));

		// 月球的旋转
		moonRelativeTransform = glm::rotate(moonRelativeTransform, (float)currentTime, glm::vec3(0.0, 0.0, 1.0));

		// 月球的缩放
		moonRelativeTransform = glm::scale(moonRelativeTransform, glm::vec3(0.25f, 0.25f, 0.25f));

		// 月球的完整世界变换
		glm::mat4 moonWorldTransform = earthTransform * moonRelativeTransform;

		// 从变换矩阵中提取位置（第四列）
		cameraPos = glm::vec3(moonWorldTransform[3]);

		// 地球位置（看向地球）
		cameraTarget = glm::vec3(earthTransform[3]);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	}
	break;
	}
}

void init(GLFWwindow* window) {
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

	// 初始化默认相机
	viewMode = 0;
	setupVertices();
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	alphaLoc = glGetUniformLocation(renderingProgram, "alpha");

	// 更新相机位置
	updateCamera(currentTime);

	//vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
	vMat = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	mvStack.push(vMat);

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// 启用透明度混合
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// ----------------------  pyramid == sun
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	mvStack.push(mvStack.top());
	mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(1.0, 0.0, 0.0));

	// 根据视图模式设置太阳的透明度
	if (viewMode == 1) {
		glUniform1f(alphaLoc, 0.0f);  // 太阳视图时设为半透明
	}
	else {
		glUniform1f(alphaLoc, 1.0f);  // 其他视图时不透明
	}

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 18);
	mvStack.pop();

	//-----------------------  cube == planet  
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime)*4.0, 0.0f, cos((float)currentTime)*4.0));
	mvStack.push(mvStack.top());
	mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 1.0, 0.0));

	if (viewMode == 2) {
		glUniform1f(alphaLoc, 0.0f);  // 太阳视图时设为半透明
	}
	else {
		glUniform1f(alphaLoc, 1.0f);  // 其他视图时不透明
	}

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	mvStack.pop();

	//-----------------------  smaller cube == moon
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime)*2.0, cos((float)currentTime)*2.0));
	mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 0.0, 1.0));
	mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));

	if (viewMode == 3) {
		glUniform1f(alphaLoc, 0.0f);  // 太阳视图时设为半透明
	}
	else {
		glUniform1f(alphaLoc, 1.0f);  // 其他视图时不透明
	}

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	mvStack.pop(); mvStack.pop(); 
	
	//-----------------------  triangular prism == planet  
	mvStack.push(mvStack.top());
	mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime) * 8.0, 0.0f, cos((float)currentTime) * 8.0));
	mvStack.push(mvStack.top());
	mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 24);
	mvStack.pop(); mvStack.pop();

	mvStack.pop();
	mvStack.pop();  // the final pop is for the view matrix
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_0:
			viewMode = 0;
			cout << "切换到默认视图" << endl;
			break;
		case GLFW_KEY_1:
			viewMode = 1;
			cout << "切换到太阳视图" << endl;
			break;
		case GLFW_KEY_2:
			viewMode = 2;
			cout << "切换到地球视图" << endl;
			break;
		case GLFW_KEY_3:
			viewMode = 3;
			cout << "切换到月球视图" << endl;
			break;
		}
	}
}

void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(600, 600, "Chapter 4 - program 4", NULL, NULL);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);

	// 显示操作说明
	cout << "操作说明:" << endl;
	cout << "按 0: 默认视图" << endl;
	cout << "按 1: 太阳视图" << endl;
	cout << "按 2: 地球视图" << endl;
	cout << "按 3: 月球视图" << endl;

	init(window);

	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}