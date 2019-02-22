#include "renderingmanager.h"
#include "core/broker.h"
#include "loading/loadingmanager.h"
#include <iostream>
#include <string>
#include "ShaderTools.h"

#include "PxRigidDynamic.h"

//**Must include glad and GLFW in this order or it breaks**
#include <glad/glad.h>
#include <GLFW/glfw3.h>


RenderingManager::RenderingManager(Broker *broker)
	: _broker(broker)
{
	
}

RenderingManager::~RenderingManager() {

}


void RenderingManager::init() {
	
	openWindow();
	glEnable(GL_DEPTH_TEST);
	shaderProgram = ShaderTools::InitializeShaders();
	if (shaderProgram == 0) {
		std::cout << "Program could not initialize shaders, TERMINATING" << std::endl;
		return;
	}

}


void RenderingManager::RenderScene(const std::vector<Geometry>& objects) {
	//Clears the screen to a dark grey background
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(shaderProgram);

	float fov = 60.0f;
	int width = 1024;
	int height = 512;

	glm::mat4 Projection = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 2000.0f);

	std::shared_ptr<ShoppingCartPlayer> player = _broker->getPhysicsManager()->getActiveScene()->getAllShoppingCartPlayers().at(0);
	physx::PxRigidDynamic* playerDyn = player->_actor->is<physx::PxRigidDynamic>();
	physx::PxTransform playerTransform = playerDyn->getGlobalPose();
	physx::PxVec3 playerPos = playerTransform.p;
	physx::PxQuat playerRot = playerTransform.q;

	physx::PxVec3 testVec(0, 50, -50);
	testVec = playerRot.rotate(testVec);

	glm::mat4 View = glm::lookAt(
		glm::vec3(playerPos.x + testVec.x, playerPos.y + testVec.y, playerPos.z + testVec.z), // camera position
		glm::vec3(playerPos.x, playerPos.y, playerPos.z), // looks at 
		glm::vec3(0, 1, 0)  // up vector
	);

	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 mvp = Projection * View * Model;

	GLuint MatrixID = glGetUniformLocation(shaderProgram, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);


	for (const Geometry& g : objects) {
		glBindVertexArray(g.vao);
		glDrawArrays(g.drawMode, 0, g.verts.size());
		// reset state to default (no shader or geometry bound)
		glBindVertexArray(0);
	}
	glUseProgram(0); //unbind the shader

	// check for an report any OpenGL errors
	CheckGLErrors();
}



void RenderingManager::assignBuffers(Geometry& geometry) {
	//Generate vao for the object
	//Constant 1 means 1 vao is being generated
	glGenVertexArrays(1, &geometry.vao);
	glBindVertexArray(geometry.vao);

	//Generate vbos for the object
	//Constant 1 means 1 vbo is being generated
	glGenBuffers(1, &geometry.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry.vertexBuffer);
	//Parameters in order: Index of vbo in the vao, number of primitives per element, primitive type, etc.
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &geometry.normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry.normalBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &geometry.colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry.colorBuffer);
	//Parameters in order: Index of vbo in the vao, number of primitives per element, primitive type, etc.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	/*
	glGenBuffers(1, &geometry.uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry.uvBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	*/


}

void RenderingManager::setBufferData(Geometry& geometry) {
	//Send geometry to the GPU
	//Must be called whenever anything is updated about the object
	glBindBuffer(GL_ARRAY_BUFFER, geometry.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * geometry.verts.size(), geometry.verts.data(), GL_STATIC_DRAW);

	/*glBindBuffer(GL_ARRAY_BUFFER, geometry.normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * geometry.normals.size(), geometry.normals.data(), GL_STATIC_DRAW);*/

	glBindBuffer(GL_ARRAY_BUFFER, geometry.colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * geometry.colors.size(), geometry.colors.data(), GL_STATIC_DRAW);
	/*
	glBindBuffer(GL_ARRAY_BUFFER, geometry.uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * geometry.uvs.size(), geometry.uvs.data(), GL_STATIC_DRAW);
	*/
}

void RenderingManager::deleteBufferData(Geometry& geometry) {
	glDeleteBuffers(1, &geometry.vertexBuffer);
	glDeleteBuffers(1, &geometry.normalBuffer);
	glDeleteBuffers(1, &geometry.colorBuffer);
	glDeleteBuffers(1, &geometry.uvBuffer);
	glDeleteVertexArrays(1, &geometry.vao);
}


bool RenderingManager::CheckGLErrors() {
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		std::cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			std::cout << "GL_INVALID_ENUM" << std::endl; break;
		case GL_INVALID_VALUE:
			std::cout << "GL_INVALID_VALUE" << std::endl; break;
		case GL_INVALID_OPERATION:
			std::cout << "GL_INVALID_OPERATION" << std::endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
		case GL_OUT_OF_MEMORY:
			std::cout << "GL_OUT_OF_MEMORY" << std::endl; break;
		default:
			std::cout << "[unknown error code]" << std::endl;
		}
		error = true;
	}
	return error;
}


GLFWwindow* RenderingManager::getWindow() {
	return _window;
}





glm::mat4 RenderingManager::Camera(float theta, float radius, float phi) {
	int height;
	int width;
	glfwGetWindowSize(_window, &width, &height);
	//float fov = 60.0f;

	glm::mat4 Projection = glm::perspective(glm::radians(theta), (float)width / (float)height, 0.1f, 200.0f);


	float x, y, z;

	theta *= 3.14f / 180.f;
	x = float(radius * sin(theta) * sin(phi));
	y = float(radius * cos(theta));
	z = float(radius * cos(phi) * sin(theta));

	glm::mat4 View = glm::lookAt(
		glm::vec3(x, y, z), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // up vector
	);

	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 mvp = Projection * View * Model;
	return mvp;
}


void RenderingManager::updateMilliseconds(double deltaTime) {
	// call LATEUPDATE() for all behaviour scripts...
	for (std::shared_ptr<Entity> &entity : _broker->getPhysicsManager()->getActiveScene()->_entities) {
		std::shared_ptr<Component> comp = entity->getComponent(ComponentTypes::BEHAVIOUR_SCRIPT);
		if (comp != nullptr) {
			std::shared_ptr<BehaviourScript> script = std::static_pointer_cast<BehaviourScript>(comp);
			script->lateUpdate(deltaTime);
		}
	}


	for (Geometry& geoDel : _objects) {
		deleteBufferData(geoDel);
	}

	_objects.clear();


	Geometry ground = *(_broker->getLoadingManager()->getGeometry(GeometryTypes::GROUND_GEO));

	for (int i = 0; i < ground.verts.size(); i++) {
		ground.colors.push_back(glm::vec3(0.0f, 1.0f,0.0f));
	}
	ground.drawMode = GL_TRIANGLES;

	assignBuffers(ground);
	setBufferData(ground);


	_objects.push_back(ground);
	

	std::shared_ptr<ShoppingCartPlayer> player = _broker->getPhysicsManager()->getActiveScene()->getAllShoppingCartPlayers().at(0);
	physx::PxRigidDynamic* playerDyn = player->_actor->is<physx::PxRigidDynamic>();
	physx::PxTransform playerTransform = playerDyn->getGlobalPose();
	physx::PxVec3 playerPos = playerTransform.p;
	physx::PxQuat playerRot = playerTransform.q;

	Geometry chassisDefault = *(_broker->getLoadingManager()->getGeometry(GeometryTypes::VEHICLE_CHASSIS_GEO));

	std::vector<glm::vec4> newChassisVerts;
	std::vector<glm::vec3> newChassisColors;

	for (glm::vec4 v : chassisDefault.verts) {
		physx::PxVec3 vPhys(v.x, v.y, v.z);
		vPhys = playerRot.rotate(vPhys);
		vPhys += playerPos;
		newChassisVerts.push_back(glm::vec4(vPhys.x, vPhys.y, vPhys.z, 1.0f));
		newChassisColors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
	}

	chassisDefault.verts = newChassisVerts;
	chassisDefault.colors = newChassisColors;

	chassisDefault.drawMode = GL_TRIANGLES;
	assignBuffers(chassisDefault);
	setBufferData(chassisDefault);

	_objects.push_back(chassisDefault);



	std::shared_ptr<SpareChange> spareChange = _broker->getPhysicsManager()->getActiveScene()->getAllSpareChange().at(0);

	physx::PxRigidDynamic* scDyn = spareChange->_actor->is<physx::PxRigidDynamic>();
	physx::PxTransform scTransform = scDyn->getGlobalPose();
	physx::PxVec3 scPos = scTransform.p;

	Geometry scDefault = *(_broker->getLoadingManager()->getGeometry(GeometryTypes::SPARE_CHANGE_GEO));

	std::vector<glm::vec4> newSCVerts;
	std::vector<glm::vec3> newSCColors;

	// TODO: change later to use an actual detailed mesh and accoutn for rotation
	for (glm::vec4 v : scDefault.verts) {
		physx::PxVec3 vPhys(v.x, v.y, v.z);
		vPhys += scPos;
		newSCVerts.push_back(glm::vec4(vPhys.x, vPhys.y, vPhys.z, 1.0f));
		newSCColors.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
	}

	scDefault.verts = newSCVerts;
	scDefault.colors = newSCColors;

	scDefault.drawMode = GL_TRIANGLES;
	assignBuffers(scDefault);
	setBufferData(scDefault);

	_objects.push_back(scDefault);
	


	RenderScene(_objects);
	glfwSwapBuffers(_window);
	
}



void RenderingManager::cleanup() {
	glfwTerminate();
}



void RenderingManager::openWindow() {

	//Initialize the GLFW windowing system
	if (!glfwInit()) {
		std::cout << "ERROR: GLFW failed to initialize, TERMINATING" << std::endl;
		return;
	}

	//Attempt to create a window with an OpenGL 4.1 core profile context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	int width = 1024;
	int height = 512;
	_window = glfwCreateWindow(width, height, "Top Shopper", 0, 0);
	if (!_window) {
		std::cout << "Program failed to create GLFW window, TERMINATING" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(_window);
	glfwSetWindowUserPointer(_window, this);

	if (!gladLoadGL()) {
		std::cout << "GLAD init failed" << std::endl;
		return;
	}

	QueryGLVersion();
}

void RenderingManager::QueryGLVersion() {
	// query opengl version and renderer information
	std::string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	std::string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	std::string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	std::cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << std::endl;
}

void ErrorCallback(int error, const char* description) {
	std::cout << "GLFW ERROR " << error << ":" << std::endl;
	std::cout << description << std::endl;
}

