//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "initShader.h"
#include "cube.h"
#include "sphere.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "texture.hpp"

enum eShadeMode { NO_LIGHT, GOURAUD, PHONG, NUM_LIGHT_MODE };

glm::mat4 projectMat;
glm::mat4 viewMat;
int shadeMode = GOURAUD;
int isTexture = false;

GLuint shadeModeID;       
GLuint modelMatrixID;    
GLuint viewMatrixID;     
GLuint projectMatrixID;   
GLuint programID;
GLuint textureModeID;

Sphere sphere(50, 50);

float Angle = glm::radians(60.0f);
float rotAngle = 0.0f;
float jumppos = 0.0f;

typedef glm::vec4 point4;

GLuint vao, sphereVAO;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
point4 normals[NumVertices];
glm::vec2 texCoords[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

glm::vec2 quadTexCoords[4] = {
	glm::vec2(0.0f, 0.0f), // 왼쪽 아래
	glm::vec2(1.0f, 0.0f), // 오른쪽 아래
	glm::vec2(1.0f, 1.0f), // 오른쪽 위
	glm::vec2(0.0f, 1.0f)  // 왼쪽 위
};

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	glm::vec3 u = glm::vec3(vertices[b]) - glm::vec3(vertices[a]);
	glm::vec3 v = glm::vec3(vertices[c]) - glm::vec3(vertices[a]);
	glm::vec3 normal = glm::normalize(glm::cross(u, v));

	normals[Index] = glm::vec4(normal, 0.0f); points[Index] = vertices[a]; texCoords[Index] = quadTexCoords[0]; Index++;
	normals[Index] = glm::vec4(normal, 0.0f); points[Index] = vertices[b]; texCoords[Index] = quadTexCoords[1]; Index++;
	normals[Index] = glm::vec4(normal, 0.0f); points[Index] = vertices[c]; texCoords[Index] = quadTexCoords[2]; Index++;
	normals[Index] = glm::vec4(normal, 0.0f); points[Index] = vertices[a]; texCoords[Index] = quadTexCoords[0]; Index++;
	normals[Index] = glm::vec4(normal, 0.0f); points[Index] = vertices[c]; texCoords[Index] = quadTexCoords[2]; Index++;
	normals[Index] = glm::vec4(normal, 0.0f); points[Index] = vertices[d]; texCoords[Index] = quadTexCoords[3]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	colorcube();

	// Create a vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// Allocate buffer space for positions and normals
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals) + sizeof(texCoords), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), sizeof(texCoords), texCoords);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
	glUseProgram(program);
	programID = program;

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	GLuint vTexCoord = glGetAttribLocation(programID, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(normals)));


	// sphere

	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);

	// Create VBO for sphere

	GLuint sphereVBO;
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);

	int vertSize = sizeof(sphere.verts[0]) * sphere.verts.size();
	int normalSize = sizeof(sphere.normals[0]) * sphere.normals.size();
	int texSize = sizeof(sphere.texCoords[0]) * sphere.texCoords.size();

	glBufferData(GL_ARRAY_BUFFER, vertSize + normalSize + texSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, sphere.verts.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertSize, normalSize, sphere.normals.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertSize + normalSize, texSize, sphere.texCoords.data());

	// Set up vertex arrays for sphere
	glUseProgram(programID); // Ensure correct program is active

	vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vNormal = glGetAttribLocation(programID, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertSize));


	vTexCoord = glGetAttribLocation(programID, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertSize + normalSize));
	
	textureModeID = glGetUniformLocation(program, "isTexture");
	glUniform1i(textureModeID, isTexture);

	shadeModeID = glGetUniformLocation(program, "shadeMode");
	viewMatrixID = glGetUniformLocation(program, "mView");
	modelMatrixID = glGetUniformLocation(program, "mModel");
	projectMatrixID = glGetUniformLocation(program, "mProject");


	GLuint cubeTexture = loadBMP_custom("brick.bmp");
	GLuint SphereTexture = loadBMP_custom("earth.bmp");

	GLuint cubeTextureID = glGetUniformLocation(program, "cubeTexture");
	GLuint SphereTextureID = glGetUniformLocation(program, "sphereTexture");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, SphereTexture);

	glUniform1i(cubeTextureID, 0);
	glUniform1i(SphereTextureID, 1);

	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	viewMat = glm::lookAt(glm::vec3(3, 3, 13), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
	glUniform1i(shadeModeID, shadeMode);

	if (vPosition == -1 || vNormal == -1) {
		std::cerr << "Failed to get attribute location!" << std::endl;
	}

	if (modelMatrixID == -1) {
		std::cerr << "Failed to get uniform location!" << std::endl;
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}


void drawHuman(glm::mat4 humanMat)
{
	glm::mat4 modelMat;

	// body 
	glBindVertexArray(vao); // Bind cube VAO

	glUniform1i(glGetUniformLocation(programID, "objectType"), 0);
	modelMat = glm::translate(humanMat, glm::vec3(0.0, 1.5, 0.0));
	modelMat = glm::scale(modelMat, glm::vec3(1.5, 1.8, 0.8));
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// head
	
	glBindVertexArray(sphereVAO);
	glUniform1i(glGetUniformLocation(programID, "objectType"), 1);
	modelMat = glm::translate(humanMat, glm::vec3(0.0, 3.0, 0.0));
	modelMat = glm::scale(modelMat, glm::vec3(0.8, 0.8, 0.8));
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, sphere.verts.size());
	
	glBindVertexArray(vao);
	glUniform1i(glGetUniformLocation(programID, "objectType"), 0);
	// arm
	glm::vec3 armPos[4];

	armPos[0] = glm::vec3(1.2f, 1.8f, 0.0f); // left arm
	armPos[1] = glm::vec3(1.2f, 0.4f, 0.0f); // left forearm
	armPos[2] = glm::vec3(-1.2f, 1.8f, 0.0f); // right arm
	armPos[3] = glm::vec3(-1.2f, 0.4f, 0.0f); // right forearm

	for (int i = 0; i < 4; i++) 
	{

		float armAngle = 0;
		if (i < 2)
		{
			armAngle = rotAngle * -1;
		}
		else
		{
			armAngle = rotAngle;
		}

		modelMat = glm::translate(humanMat, glm::vec3(0, 2.0, 0));
		modelMat = glm::rotate(modelMat, armAngle, glm::vec3(1, 0, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, -2.0, 0));

		if (i % 2 == 1)
		{
			modelMat = glm::translate(modelMat, glm::vec3(0, 1.0, 0));
			modelMat = glm::rotate(modelMat, -abs(armAngle) * 2, glm::vec3(1, 0, 0));
			modelMat = glm::translate(modelMat, glm::vec3(0, -1.0, 0));
		}

		modelMat = glm::translate(modelMat, armPos[i]);

		if (i % 2 == 1)
		{
			modelMat = glm::scale(modelMat, glm::vec3(0.5, 1.3, 0.6));
		}
		else
		{
			modelMat = glm::scale(modelMat, glm::vec3(0.6, 0.9, 0.6));
		}
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}

	// leg
	glm::vec3 legPos[4];

	legPos[0] = glm::vec3(0.4f, -0.2f, 0); // left upper leg
	legPos[1] = glm::vec3(0.4f, -1.9f, 0); // left lower leg
	legPos[2] = glm::vec3(-0.4f, -0.2f, 0); // right upper leg
	legPos[3] = glm::vec3(-0.4f, -1.9f, 0); // right lower leg

	for (int i = 0; i < 4; i++) 
	{
		float legAngle = 0;
		if (i < 2)
		{
			legAngle = rotAngle * -1 - 0.2f;
		}
		else
		{
			legAngle = rotAngle - 0.2f;
		}

		modelMat = glm::translate(humanMat, glm::vec3(0, 0.1f, 0));
		modelMat = glm::rotate(modelMat, legAngle, glm::vec3(1, 0, 0));
		modelMat = glm::translate(modelMat, glm::vec3(0, -0.1f, 0));

		float lowlegAngle = legAngle * 1.3 - 0.2f;

		if (i % 2 == 1)
		{
			modelMat = glm::translate(modelMat, glm::vec3(0, -0.8f, 0));
			modelMat = glm::rotate(modelMat, abs(lowlegAngle), glm::vec3(1, 0, 0));
			modelMat = glm::translate(modelMat, glm::vec3(0, 0.8f, 0));

		}

		modelMat = glm::translate(modelMat, legPos[i]);

		if (i % 2 == 1)
		{
			modelMat = glm::scale(modelMat, glm::vec3(0.5, 1.7, 0.6));
		}
		else
		{
			modelMat = glm::scale(modelMat, glm::vec3(0.6, 1.3, 0.6));
		}
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	}
}



void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programID);

	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
	glUniform1i(shadeModeID, shadeMode);


	glm::mat4 worldMat;
	worldMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, jumppos * 2, 0));
	worldMat = glm::rotate(worldMat, glm::radians(8.0f), glm::vec3(1, 0, 0));

	drawHuman(worldMat);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void idle()
{
	static int prevTime = glutGet(GLUT_ELAPSED_TIME);
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	float t = abs(currTime - prevTime) / 1000.0f;

	rotAngle = Angle * std::sin(t * 8.0f);

	jumppos = (rotAngle / 3) * std::sin(t * 8.0f);

	glutPostRedisplay();

}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		viewMat = glm::lookAt(glm::vec3(5, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	case '2':
		viewMat = glm::lookAt(glm::vec3(-1, 5, -5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	case '3':
		viewMat = glm::lookAt(glm::vec3(3, 3, 13), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		break;
	case 'n':  shadeMode = NO_LIGHT; break;
	case 'g':  shadeMode = GOURAUD; break;
	case 'p':  shadeMode = PHONG; break;
	case 't':
		isTexture = !isTexture; 
		glUniform1i(textureModeID, isTexture);
		glutPostRedisplay(); 
		break;
		
	case 033:  // Escape key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}
	glUseProgram(programID);
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);
	glUniform1i(shadeModeID, shadeMode);

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f);

	glUseProgram(programID);
	glUniformMatrix4fv(projectMatrixID, 1, GL_FALSE, &projectMat[0][0]);

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("CUBEMAN RUNNING");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
