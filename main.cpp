
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <C:/Users/Andre/Desktop/PG/proiectProiectareGrafica/proiectProiectareGrafica/glm/glm.hpp>
#include <C:/Users/Andre/Desktop/PG/proiectProiectareGrafica/proiectProiectareGrafica/glm/gtc/matrix_transform.hpp>
#include <C:/Users/Andre/Desktop/PG/proiectProiectareGrafica/proiectProiectareGrafica/glm/gtc/matrix_inverse.hpp>
#include <C:/Users/Andre/Desktop/PG/proiectProiectareGrafica/proiectProiectareGrafica/glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <list>



// window
gps::Window myWindow;

const unsigned int SHADOW_WIDTH = 5096;
const unsigned int SHADOW_HEIGHT = 5096;

glm::mat4 model;
GLuint modelLoc;
GLuint modelLoc2;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;


gps::Camera myCamera(
	glm::vec3(13.42f, 5.57f, 0.39f),
	glm::vec3(12.42f, 5.50f, 0.30f),
	glm::vec3(0.0f, 1.0f, 0.0f));


bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D scene;
gps::Model3D dog;
gps::Model3D dragon;
gps::Model3D rainDrop;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader depthMapShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader skyboxShader;

//SKYBOX
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;

unsigned int SCR_WIDTH = 1024;
unsigned int SCR_HEIGHT = 768;

//cameraMoves
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float cameraSpeed = 0.3f;
bool firstMouse = true;
float zoom = 45.0f;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

//functionalitati
int punctiforma = 0;
GLint spotlightLoc;
int tour = 0;
int ceata = 0;
GLint fogLoc;
GLfloat dogTranslateX;

//obiecte
glm::mat4 modelDragon;
glm::mat4 normalMatrixDragon;
glm::mat4 modelDog;
glm::mat4 normalMatrixDog;

//rain
int isRaining;
int nrDrops = 80;

std::list< std::pair< glm::vec3, float> > raindrops;
std::pair< glm::vec3, float> generateRaindrop() {
	return { glm::vec3(rand() % 20 - 10, rand() % 18 + 5, rand() % 20 - 10) , 0.2f };
}

void moveRaindrops() {
	for (auto& it : raindrops) {
		it.first.y -= it.second;
		if (it.first.y <= 1.0) {
			raindrops.push_back(generateRaindrop());
		}
	}
	raindrops.remove_if([](auto it) {return it.first.y <= 1.0; });
}


GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
	glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS) //sa se dezactiveze cand mai apas o data pe M
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

double xcurr = 0.0f;
double ycurr = 0.0f;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (xcurr == 0.0f && ycurr == 0.0f) {  //verific daca pozitia cursorului a fost setata,daca nu, o setez acum
		xcurr = xpos;
		ycurr = ypos;
	}
	else {
		double incrx = (xpos - xcurr) / 10;  //ca sa fie mai sensibil la miscrarea mouse-ului
		double incry = (ypos - ycurr) / 50;   //se misca mai incet la miscarea mouse-ului
		if ((pitch + incry < 79.0f) && (pitch + incry) > -89.0f) {
			pitch += incry;
		}

		yaw += incrx;
		myCamera.rotate(pitch, yaw);

		xcurr = xpos;
		ycurr = ypos;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom -= (float)yoffset;
	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;

	projection = glm::perspective(glm::radians(zoom), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 50.0f);
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_8]) {
		std::cout << myCamera.getCameraPosition().x << " ";
		std::cout << myCamera.getCameraPosition().y << " ";
		std::cout << myCamera.getCameraPosition().z << " ";
		std::cout << myCamera.getCameraTarget().x << " ";
		std::cout << myCamera.getCameraTarget().y << " ";
		std::cout << myCamera.getCameraTarget().z << " ";

	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);

	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_9]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_0]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_N]) {
		punctiforma = 1;
		myCustomShader.useShaderProgram();
		spotlightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_spotlight");
		glUniform1i(spotlightLoc, punctiforma);
	}

	if (pressedKeys[GLFW_KEY_B]) {
		punctiforma = 0;
		myCustomShader.useShaderProgram();
		spotlightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_spotlight");
		glUniform1i(spotlightLoc, punctiforma);
	}


	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 0.4f;
		//if(lightAngle <)
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 0.4f;
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_C]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_T]) {
		tour = 1;
		glfwSetTime(0.0f);
	}

	if (pressedKeys[GLFW_KEY_R]) {
		tour = 0;
		isRaining = 0;
		myCamera.setCameraPosition(glm::vec3(13.42f, 5.57f, 0.39f));
		myCamera.setCameraTarget(glm::vec3(12.42f, 5.50f, 0.30f));
		myCamera.setCameraUp(glm::vec3(0.0f, 1.0f, 0.0f));
		myCamera.setCameraFrontDirection(glm::normalize(myCamera.getCameraTarget() - myCamera.getCameraPosition()));
		myCamera.setCameraRightDirection(glm::normalize(glm::cross(myCamera.getCameraFrontDirection(), myCamera.getCameraUp())));
	}

	if (pressedKeys[GLFW_KEY_F]) {
		ceata = 1;
		myCustomShader.useShaderProgram();
		fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_fog");
		glUniform1i(fogLoc, ceata);
	}

	if (pressedKeys[GLFW_KEY_G])
	{
		ceata = 0;
		myCustomShader.useShaderProgram();
		fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_fog");
		glUniform1i(fogLoc, ceata);
	}

	if (pressedKeys[GLFW_KEY_UP])
	{
		dogTranslateX -= 0.01f;
	}
	if (pressedKeys[GLFW_KEY_DOWN])
	{
		dogTranslateX += 0.01f;

	}

	if (pressedKeys[GLFW_KEY_P]) {
		if (isRaining) return;

		isRaining = 1;
		raindrops.clear();
		for (int i = 1; i <= nrDrops; i++) {
			raindrops.push_back(generateRaindrop());
		}
	}
}

void initOpenGLWindow()
{
	myWindow.Create(1024, 768, "OpenGL scene");
	//glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
	glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

}

void initOpenGLState()
{
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}


void initObjects() {
	scene.LoadModel("models/scene/castle2.obj");
	dragon.LoadModel("models/scene/dragon.obj");
	dog.LoadModel("models/scene/dog.obj");
	screenQuad.LoadModel("models/scene/quad/quad.obj");
	lightCube.LoadModel("models/scene/cube/cube.obj");
	rainDrop.LoadModel("models/scene/rainDrop2.obj");
}

void initSkyBox() {
	faces.push_back("models/scene/skybox/right.tga");
	faces.push_back("models/scene/skybox/left.tga");
	faces.push_back("models/scene/skybox/top.tga");
	faces.push_back("models/scene/skybox/bottom.tga");
	faces.push_back("models/scene/skybox/back.tga");
	faces.push_back("models/scene/skybox/front.tga");
	mySkyBox.Load(faces);
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generam id fbo
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	//creem textura
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//atasam textura la fbo
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	//il dezactivam pana cand suntem gata sa il folosim
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	//modelDragon = glm::rotate(glm::mat4(1.0f), 0.02f, glm::vec3(0, 1, 0));
	//modelDog = glm::translate(glm::mat4(1.0f), glm::vec3(dogTranslateX, 0.0f, 0.0f));
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	//modelLoc2 = glGetUniformLocation(depthMapShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//normalMatrixDragon = glm::mat3(glm::inverseTranspose(view * modelDragon));
	//normalMatrixDog = glm::mat3(glm::inverseTranspose(view * modelDog));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	spotlightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_spotlight");
	glUniform1i(spotlightLoc, punctiforma);

	fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bool_fog");
	glUniform1i(fogLoc, ceata);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}




glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	//Deoarece luminile directionale nu au pozitie(sunt amplasate la infinit), putem folosi ca pozitie orice punct de - a
	//lungul directiei luminii, inclusiv directia insasi(interpretata ca un punct)
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir * 18.4f, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//Deoarece toate razele provenite de la o lumina directionala sunt paralele, vom folosi o proiectie
	//ortografica pentru a evita orice deformare perspectiva
	//const GLfloat near_plane = 1.0f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-32.8f, 31.0f, -34.6f, 32.6f, 0.03f, 45.6f);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void renderScene(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	scene.Draw(shader);
}

void renderDog(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();

	modelDog = glm::translate(modelDog, glm::vec3(dogTranslateX, 0.0f, 0.0f));
	dogTranslateX = 0.0f;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDog));
	//glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(modelDog));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrixDog = glm::mat3(glm::inverseTranspose(view * modelDog));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixDog));
	}
	dog.Draw(shader);
}

void renderDragon(gps::Shader shader, bool depthPass)
{
	shader.useShaderProgram();


	modelDragon = glm::rotate(modelDragon, 0.02f, glm::vec3(0, 1, 0));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDragon));
	//glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(modelDragon));

	//do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrixDragon = glm::mat3(glm::inverseTranspose(view * modelDragon));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrixDragon));
	}
	dragon.Draw(shader);
}


void renderSkyBox(gps::Shader shader) {
	shader.useShaderProgram();
	view = myCamera.getViewMatrix();

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	mySkyBox.Draw(shader, view, projection);

}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	//Schimbati portul de vizualizare pentru a acoperi dimensiunea texturarii de adancime
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	//Activati obiectul framebuffer;
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

	//Curatati (initializati) buffer-ul de adancime;
	glClear(GL_DEPTH_BUFFER_BIT);

	//rasterizam in harta de adancime
	renderScene(depthMapShader, 1);
	renderDog(depthMapShader, 1);
	renderDragon(depthMapShader, 1);

	//dezactivam obiectul framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {


		// final scene rendering pass (with shadows)

		//Modificati portul de vizualizare pentru a acomoda obiectul framebuffer al ferestrei OpenGL;
		glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

		//Curatati (initializati) buffer-ul de adancime si culoarea;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		if (tour == 1)
		{
			myCamera.cameraPresentation();
		}

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//activam textura de adancime
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		//rasterizam in framebuffer
		renderScene(myCustomShader, false);
		renderDog(myCustomShader, false);
		renderDragon(myCustomShader, false);
		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.0001f, 0.0001f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);


	}

	if (isRaining) {
		moveRaindrops();
		for (auto it : raindrops) {
			model = glm::translate(glm::mat4(1.0f), it.first);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

			rainDrop.Draw(myCustomShader);
		}
	}

	renderSkyBox(skyboxShader);

}

void cleanup() {
	myWindow.Delete();
}

int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	setWindowCallbacks();
	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkyBox();



	glCheckError();

	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());
		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
