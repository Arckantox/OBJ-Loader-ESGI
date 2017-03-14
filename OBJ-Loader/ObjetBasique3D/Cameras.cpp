#include <vector>
#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#include <windows.h>
#endif

#include "glm.hpp"

#include "glew.h"
#ifdef _MSC_VER
#pragma comment(lib, "glew32.lib")
#endif

#include "freeglut.h"

#include "../common/EsgiShader.h"
#include "../common/mat4.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

// format des vertices : X, Y, Z, ?, ?, ?, ?, ? = 8 floats
//#include "../data/DragonData.h"

/*
#if _MSC_VER
uint32_t dragonVertexCount = _countof(DragonVertices);
uint32_t dragonIndexCount = _countof(DragonIndices);
#endif
*/

EsgiShader g_BasicShader;

// alternativement unsigned short, GLushort, uint16_t
static const GLushort g_Indices[] = { 0, 1, 2, 3 };

int light = 2;
int textu = 1;

int width = 1000;
int height = 1000;

bool renderMode = true;		//True = Polygon, False = Wireframe
bool backfaceMode = true;

GLuint VBO;	// identifiant du Vertex Buffer Object
GLuint IBO;	// identifiant du Index Buffer Object
GLuint TexObj; // identifiant du Texture Object

// -----------------------------------------------------------------------------------------------TYPE DE CAMERA----------------------------------------------------------------------------------------------
int CamType = 0;
// FPS = 0
// Orbit = 1
// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Camera variables
float posX = 0.0f;

float posY = -0.5f;
float posZ = -10.0f;
float moveSpeed = 1000.0f;
float rotSpeed = 0.08f;

float rotX = 0.0f;
float rotY = 0.0f;
float distance = -10.0f;

int lastposX = height * 0.5f;
int lastposY = width * 0.5f;

float deltaAngle = 0.0f;
int xOrigin = -1;


float TimeInSeconds;
int TimeSinceAppStartedInMS;
int OldTime = 0;
float DeltaTime;

//end Camera Variables

void ChangeCam(int type)
{
	switch (type)
	{
	case 0:
		CamType = 0;
		rotX = 0.0f;
		rotY = 0.0f;
		posX = 0.0f;
		posY = -0.5f;
		posZ = -10.0f;
		rotSpeed = 0.3f;
		break;
	case 1:
		CamType = 1;
		rotX = 0.0f;
		rotY = 0.0f;
		posX = 0.0f;
		posY = -0.5f;
		posZ = 0.0f;
		distance = -10.0f;
		break;
	}

}


std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;


bool loadOBJ(const char * path,std::vector < glm::vec3 > & out_vertices,std::vector < glm::vec2 > & out_uvs,std::vector < glm::vec3 > & out_normals
	//float[] Vertices,
	//uint16_t[] Indices
)
{
	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)

			break; // EOF = End Of File. Quit the loop.
		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);

		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexindex = vertexIndices[i];

		glm::vec3 vertex = temp_vertices[vertexindex - 1];
		//glm::vec3 vertex = temp_vertices[i];
		out_vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < uvIndices.size(); i++) {
		unsigned int vertexIndex = uvIndices[i];

		glm::vec2 uv = temp_uvs[vertexIndex - 1];

		out_uvs.push_back(uv);
	}

	for (unsigned int i = 0; i < normalIndices.size(); i++) {
		unsigned int vertexIndex = normalIndices[i];

		glm::vec3 vertex = temp_normals[vertexIndex - 1];

		out_normals.push_back(vertex);
	}
	

}

bool loadMTL(const char * path, float* Ka, float* Kd, float* Ks, float& shine)
{
	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)

			break; // EOF = End Of File. Quit the loop.
		if (strcmp(lineHeader, "Ka") == 0) {
			fscanf(file, "%f %f %f\n", &Ka[0], &Ka[1] ,&Ka[2]);
		}
		if (strcmp(lineHeader, "Kd") == 0) {
			fscanf(file, "%f %f %f\n", &Kd[0], &Kd[1], &Kd[2]);
		}
		if (strcmp(lineHeader, "Ks") == 0) {
			fscanf(file, "%f %f %f\n", &Ks[0], &Ks[1], &Ks[2]);
		}
		if (strcmp(lineHeader, "Ns") == 0) {
			fscanf(file, "%f\n", &shine);
		}
	}
}



//--------------------------------------------------------------GEOMETRY SHADER--------------------------------------------------------

const float step = 1.0f;
const int nbLines = 100;
float gridPoints[nbLines*2*6];

GLuint vbo, vao;
GLuint shaderProgram, gridVertexShader, gridFragmentShader, geometryShader;

GLuint CreateShader(uint32_t type, const char* filename)
{
	GLuint shader = glCreateShader(type);
	FILE* file = fopen(filename, "rb");
	fseek(file, 0, SEEK_END);
	auto len = ftell(file);			// auto permet de détecter automatiquement le type de retour de la fonction à laquelle la variable est affecté
	rewind(file);
	auto buffer = new char[len + 1];
	fread(buffer, len, 1, file);
	buffer[len] = '\0';			// = 0 fonctionne aussi
	glShaderSource(shader, 1, &buffer, nullptr);
	glCompileShader(shader);
	delete[] buffer;

	return shader;
}

void MakeGeometryShader()
{
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	
	for (int i = 0; i < nbLines; i++)
	{
		gridPoints[i * 6] = -step*0.5f*nbLines+i*step;
		gridPoints[i * 6 + 1] = 0;
		gridPoints[i * 6 + 2] = step*0.5*nbLines;
		gridPoints[i * 6 + 3] = -step*0.5f*nbLines + i*step;
		gridPoints[i * 6 + 4] = 0;
		gridPoints[i * 6 + 5] = -step*0.5*nbLines;
	}
	for (int i = nbLines; i < nbLines*2; i++)
	{
		gridPoints[i * 6] = step*0.5*nbLines;
		gridPoints[i * 6 + 1] = 0;
		gridPoints[i * 6 + 2] = -step*0.5f*nbLines + (i - nbLines)*step;
		gridPoints[i * 6 + 3] = -step*0.5*nbLines;
		gridPoints[i * 6 + 4] = 0;
		gridPoints[i * 6 + 5] = -step*0.5f*nbLines + (i - nbLines)*step;
	}
	
	gridVertexShader = CreateShader(GL_VERTEX_SHADER, "grid.vs");
	//geometryShader = CreateShader(GL_GEOMETRY_SHADER, "geometry.gs");
	gridFragmentShader = CreateShader(GL_FRAGMENT_SHADER, "grid.fs");

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, gridVertexShader);
	//glAttachShader(shaderProgram, geometryShader);
	glAttachShader(shaderProgram, gridFragmentShader);
	glLinkProgram(shaderProgram);
}


void DisplayGrid()
{
	glUseProgram(shaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridPoints), gridPoints, GL_STATIC_DRAW);

	glBindVertexArray(vao);
	GLint posAttrib = glGetAttribLocation(shaderProgram, "pos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINES, 0,(nbLines * 2 * 6));

	glUseProgram(0);
}

//--------------------------------------------------------------MENU----------------------------------------------------------------------

bool showGrid = true;

void menu_Selection(int option)
{
	switch (option)
	{
	case 1:
		//Grid ON/OFF
		showGrid = !showGrid;
		break;
	case 2:
		//Backface ON/OFF
		backfaceMode = !backfaceMode;
		break;
	case 3:
		if (textu == 1)
		{
			textu = 0;
		}
		else {
			textu = 1;
		}
		break;
	case 4:
		light = 0;
		break;
	case 5:
		exit(0);
		break;
	}
}
void camera_Menu(int option)
{
	switch (option)
	{
	case 1:
		ChangeCam(0);	//Free
		break;
	case 2:
		ChangeCam(1);	//Orbit
		break;
	}
}
void render_Menu(int option)
{
	switch (option)
	{
	case 1:
		//Polygon
		renderMode = true;
		break;
	case 2:
		//Wireframe
		renderMode = false;
		break;
	}
}
void light_Menu(int option)
{
	
	switch (option)
	{
	case 1:
		light = 1;
		break;
	case 2:
		light = 2;
		break;
	case 3:
		light = 3;
		break;
	}
}

void InitMenu()
{
	// Création du menu
	int menu = glutCreateMenu(menu_Selection);

	// Index des sous-menus
	GLint cameraMenu, renderMenu, lightMenu;

	cameraMenu = glutCreateMenu(camera_Menu);
	glutAddMenuEntry("Free Camera", 1);
	glutAddMenuEntry("Orbital Camera", 2);

	renderMenu = glutCreateMenu(render_Menu);
	glutAddMenuEntry("Polygon", 1);
	glutAddMenuEntry("Wireframe", 2);

	lightMenu = glutCreateMenu(light_Menu);
	glutAddMenuEntry("Lambert", 1);
	glutAddMenuEntry("Phong", 2);
	glutAddMenuEntry("Blinn-Phong", 3);

	glutCreateMenu(menu_Selection);
	glutAddSubMenu("Camera", cameraMenu);
	glutAddMenuEntry("Grid ON/OFF", 1);
	glutAddSubMenu("Render", renderMenu);
	glutAddMenuEntry("Backface ON/OFF", 2);
	glutAddMenuEntry("Texture ON/OFF", 3);
	glutAddMenuEntry("Light ON/OFF", 4);
	glutAddSubMenu("Light Mode", lightMenu);
	glutAddMenuEntry("Quit", 5);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//float mAl[3] = { 0.5882,0.5882,0.5882 };
//float mDl[3] = { 0.5882,0.5882,0.5882 };

float mAl[3];
float mDl[3];
float mSl[3] = { 1,1,1 };
float shininess = 1;

bool Initialize()
{
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals; // Won't be used at the moment.
	bool res = loadOBJ("Captain America Shield 3D Model.obj", vertices, uvs, normals);

	loadMTL("Captain America Shield 3D Model.MTL", mAl, mDl, mSl, shininess);

	float* vertex1 = new float[vertices.size() * 8];
	unsigned int* indices1 = new unsigned int[vertexIndices.size()];

	for (int i = 0; i < vertices.size() * 8; i += 8) {
		vertex1[i] = vertices[i/8].x;
		vertex1[i+1] = vertices[i/8].y;
		vertex1[i+2] = vertices[i/8].z;
		vertex1[i+3] = normals[i / 8].x;
		vertex1[i+4] = normals[i / 8].y;
		vertex1[i+5] = normals[i / 8].z;
		vertex1[i+6] = uvs[i / 8].x;
		vertex1[i+7] = uvs[i / 8].y;
		
		
	}

	for (int i = 0; i < vertexIndices.size(); i++)
	{
		indices1[i] = i;
	}
	

	glewInit();
	g_BasicShader.LoadVertexShader("basicLight.vs");
	g_BasicShader.LoadFragmentShader("basicLight.fs");

	//g_BasicShader.LoadVertexShader("basic.vs");
	//g_BasicShader.LoadFragmentShader("basic.fs");

	g_BasicShader.CreateProgram();

	glGenTextures(1, &TexObj);
	glBindTexture(GL_TEXTURE_2D, TexObj);
	int w, h, c; //largeur, hauteur et # de composantes du fichier
	uint8_t* bitmapRGBA = stbi_load("Captain_America_Shield_3D_Model_MAP.png",
		&w, &h, &c, STBI_rgb_alpha);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //GL_NEAREST)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, // Destination
		GL_RGBA, GL_UNSIGNED_BYTE, bitmapRGBA);		// Source

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bitmapRGBA);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &VBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, dragonVertexCount * sizeof(float), DragonVertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 8 * sizeof(float), vertex1, GL_STATIC_DRAW);

	// rendu indexe
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, dragonIndexCount *  sizeof(GLushort), DragonIndices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(unsigned int), indices1, GL_STATIC_DRAW);

	// le fait de specifier 0 comme BO desactive l'usage des BOs
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	ChangeCam(CamType);

	MakeGeometryShader();

	InitMenu();

	return true;

}

void Terminate()
{
	glDeleteTextures(1, &TexObj);
	glDeleteBuffers(1, &IBO);
	glDeleteBuffers(1, &VBO);

	g_BasicShader.DestroyProgram();
}

void update()
{
	glutPostRedisplay();
}

void animate()
{
	//BackFace Culling

	glEnable(GL_CULL_FACE);
	if (backfaceMode)
	{
		glCullFace(GL_BACK);
	}
	else
	{
		glCullFace(GL_FRONT);
	}
	
	

	if (renderMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	//Rendu filaire
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// afin d'obtenir le deltatime actuel
	TimeSinceAppStartedInMS = glutGet(GLUT_ELAPSED_TIME);
	TimeInSeconds = TimeSinceAppStartedInMS / 1000.0f;
	DeltaTime = (TimeSinceAppStartedInMS - OldTime )/ 1000.0f;
	OldTime = TimeSinceAppStartedInMS;

	glViewport(0, 0, width, height);
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	//glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	auto program = g_BasicShader.GetProgram();
	glUseProgram(program);
	
	uint32_t texUnit = 0;
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, TexObj);
	auto texture_location = glGetUniformLocation(program, "u_Texture");
	glUniform1i(texture_location, texUnit);
	
	// UNIFORMS
	Esgi::Mat4 worldMatrix;
	worldMatrix.MakeScale(1.0f, 1.0f, 1.0f);

	//  Camera Matrix
	Esgi::Mat4 cameraMatrix;
	switch (CamType)
	{
	case 0:	//FPS
		cameraMatrix = FPSCamera(posX, posY, posZ, rotX, rotY);
		break;
	case 1:	//Orbit
		cameraMatrix = OrbitCamera(posX, posY, posZ, distance, rotX, rotY);
		break;
	}
	
	auto textuMode = glGetUniformLocation(program, "u_textuMode");
	glUniform1i(textuMode, textu);

	auto lightMode = glGetUniformLocation(program, "u_lightMode");
	glUniform1i(lightMode, light);
	auto world_location = glGetUniformLocation(program, "u_WorldMatrix");
	glUniformMatrix4fv(world_location, 1, GL_FALSE, worldMatrix.m);

	Esgi::Mat4 projectionMatrix;
	float w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
	// ProjectionMatrix
	float aspectRatio = w / h;			// facteur d'aspect
	float fovy = 45.0f;					// degree d'ouverture
	float nearZ = 0.1f;
	float farZ = 1000.0f;
	projectionMatrix.Perspective(fovy, aspectRatio, nearZ, farZ);

	//projectionMatrix.MakeScale(1.0f / (0.5f*w), 1.0f / (0.5f*h), 1.0f);

	auto projection_location = glGetUniformLocation(program, "u_ProjectionMatrix");
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, projectionMatrix.m);

	auto camera_location = glGetUniformLocation(program, "u_CameraMatrix");
	glUniformMatrix4fv(camera_location, 1, GL_FALSE, cameraMatrix.m);

	auto time_location = glGetUniformLocation(program, "u_Time");
	glUniform1f(time_location, TimeInSeconds);

	auto lightDir_location = glGetUniformLocation(program
		, "u_PositionOrDirection");
	float lightVector[4] = {0, -1,-4,0 };
	glUniform4fv(lightDir_location, 1, lightVector);

	

	auto materialAmbiant_location = glGetUniformLocation(program, "u_materialAmbiantColor");
	glUniform3fv(materialAmbiant_location, 1,  mAl);

	auto materialDiffuse_location = glGetUniformLocation(program, "u_materialDiffuseColor");
	glUniform3fv(materialDiffuse_location, 1, mDl);

	auto materialSpecular_location = glGetUniformLocation(program, "u_materialSpecularColor");
	glUniform3fv(materialSpecular_location, 1, mSl);

	auto shininess_location = glGetUniformLocation(program, "u_Shininess");
	glUniform1f(shininess_location, shininess);

	// ATTRIBUTES
	auto position_location = glGetAttribLocation(program, "a_Position");
	auto normal_location = glGetAttribLocation(program, "a_Normal");
	auto texcoords_location = glGetAttribLocation(program, "a_TexCoords");

	glUseProgram(shaderProgram);		// GEOMETRY SHADER SPACE LOCATION REQUIERED VARIABLES

	auto world_locationG = glGetUniformLocation(shaderProgram, "u_WorldMatrix");
	glUniformMatrix4fv(world_locationG, 1, GL_FALSE, worldMatrix.m);
	auto projection_locationG = glGetUniformLocation(shaderProgram, "u_ProjectionMatrix");
	glUniformMatrix4fv(projection_locationG, 1, GL_FALSE, projectionMatrix.m);
	auto camera_locationG = glGetUniformLocation(shaderProgram, "u_CameraMatrix");
	glUniformMatrix4fv(camera_locationG, 1, GL_FALSE, cameraMatrix.m);

	glUseProgram(program);

	//glVertexAttrib3f(color_location, 0.0f, 1.0f, 0.0f);

	// Le fait de specifier la ligne suivante va modifier le fonctionnement interne de glVertexAttribPointer
	// lorsque GL_ARRAY_BUFFER != 0 cela indique que les donnees sont stockees sur le GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//le VBO contient des vertex au format suivant :
	//X, Y, Z, ?, ?, ?, U, V

	struct Vertex
	{
		float x, y, z;		// offset = 0
		float nx, ny, nz;	// offset = 3
		float u, v;			// offset = 6
	};

	// 1er cas on à l'adresse du tableau
	//Vertex* v = (Vertex*)DragonVertices;

	//size_t rel = offsetof(adresse - DragonVertices)

	glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<const void *>(0 * sizeof(float)));
	
	glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<const void *>(3 * sizeof(float)));
	// on interprete les 3 valeurs inconnues comme RGB alors que ce sont les normales
	glVertexAttribPointer(texcoords_location, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<const void *>(6 * sizeof(float)));

	//glEnableVertexAttribArray(texcoords_location);
	glEnableVertexAttribArray(position_location);
	glEnableVertexAttribArray(normal_location);
	glEnableVertexAttribArray(texcoords_location);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glDrawElements(GL_TRIANGLES, vertexIndices.size(), GL_UNSIGNED_INT, nullptr);

	glDisableVertexAttribArray(position_location);
	glDisableVertexAttribArray(normal_location);
	glDisableVertexAttribArray(texcoords_location);
	glUseProgram(0);

	//Repositionnement du curseur 
	//glutWarpPointer(width*0.5f, height*0.5f);

	if (showGrid)
	{
		DisplayGrid();
	}

	glutSwapBuffers();
}

float COS(float angle)
{
	return cos(angle * 2 * 0.00872665);
}
float SIN(float angle)
{
	return sin(angle * 2 * 0.00872665);
}


void keyboard(unsigned char key, int x, int y)
{
	if (key == 'z')
	{
		if (CamType == 0)
		{
			posZ += moveSpeed *  DeltaTime * COS(rotY);
			posX += moveSpeed *  DeltaTime * -SIN(rotY);
			posY += moveSpeed *  DeltaTime * SIN(rotX);
		}
		else
		{
			distance += moveSpeed *  DeltaTime;
		}
	}
	if (key == 'q')
	{
		posZ += moveSpeed *  DeltaTime * COS(rotY - 90);
		posX += moveSpeed *  DeltaTime * -SIN(rotY - 90);
		
	}
	if (key == 's')
	{
		if (CamType == 0)
		{
			posZ -= moveSpeed *  DeltaTime * COS(rotY);
			posX -= moveSpeed *  DeltaTime * -SIN(rotY);
			posY -= moveSpeed *  DeltaTime * SIN(rotX);
		}
		else
		{
			distance -= moveSpeed *  DeltaTime;
		}
	}
	if (key == 'd')
	{
		posZ -= moveSpeed *  DeltaTime * COS(rotY - 90);
		posX -= moveSpeed *  DeltaTime * -SIN(rotY - 90);
	}
	if (key == 27)	// Echap
	{
		exit(0);
	}
	if (key == 32)	// Space Bar
	{
		posY -= moveSpeed *  DeltaTime;
	}
	if (key == 'f')
	{
		posY += moveSpeed *  DeltaTime;
	}
	if (key == '&')	 // 1 Mode FPS
	{
		ChangeCam(0);
	}
	if (key == 233)	 // 2 Mode Orbit
	{
		ChangeCam(1);
	}
	if (key == 'g')
	{
		showGrid = !showGrid;
	}
}

void mouse(int x, int y) 
{
	
	//rotX = (float)(y-height*0.5f) * rotSpeed;
	//rotY = (float)(x-width*0.5f) * rotSpeed;
	rotX += (y - lastposY)* rotSpeed;
	rotY += (x - lastposX)* rotSpeed;
	lastposX = x;
	lastposY = y;
}


int main(int argc, const char* argv[])
{
	// passe les parametres de la ligne de commande a glut
	glutInit(&argc, (char**)argv);
	// defini deux color buffers (un visible, un cache) RGBA
	// GLUT_DEPTH alloue egalement une zone mémoire pour le depth buffer
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// positionne et dimensionne la fenetre
	glutInitWindowPosition(200, 100);
	glutInitWindowSize(width, height);
	// creation de la fenetre ainsi que du contexte de rendu
	glutCreateWindow("Transformation");

#ifdef FREEGLUT
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif

#ifdef NO_GLEW
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)
		wglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)
		wglGetProcAddress("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)
		wglGetProcAddress("glDisableVertexAttribArray");
#else
	glewInit();
#endif

	if (Initialize() == false)
		return -1;

	glutIdleFunc(update);
	glutDisplayFunc(animate);
	glutPassiveMotionFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	Terminate();

	return 1;
}