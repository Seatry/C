#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <fstream>
#include <SDL2/SDL.h>  
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h> 
#include <vector>
#include </home/sasha/glm/glm/glm.hpp>
#include </home/sasha/glm/glm/gtc/matrix_transform.hpp>
#include </home/sasha/glm/glm/gtx/transform2.hpp>

//g++ -o g8l1 g8l1.cpp -lglfw3 -lGL  -lGLU  -lm -lXrandr -lXi -lX11 -lXxf86vm -lpthread -std=c++11 -g  -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer  -lGLEW

int WIDTH = 800, HEIGHT = 800, spin_x = 0, spin_y = 0, spin_z = 0, SPLIT = 16; //16 texture->3
float x = 0, y = 0, z = 0, scale = 0.8, vx = 0.8, vy = 0.5, vz = 1, speed = 0.1;
bool fill = true, move = false, sphere = false, cube = false;
float lx = 0, ly = 0, lz = 0;
bool t = false;
float start = 0; 
int d = 32;
GLuint texture[1], programHandle, vaoHandle, vertShader, fragShader, cubeHandle, vaoCubeHandle, cubeShader;

std::vector<std::vector<float>> conusVertex;
std::vector<std::vector<float>> sphereVertex;

GLfloat staticCube[] = {-0.3, -0.3, -0.3, -0.3, 0.3, -0.3,  0.3, -0.3, -0.3,
			-0.3, 0.3, -0.3, 0.3, 0.3, -0.3, 0.3, -0.3, -0.3,
			-0.3, -0.3, 0.3, -0.3, 0.3, 0.3, 0.3, -0.3, 0.3,
			-0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, -0.3, 0.3,
			0.3, -0.3, -0.3, 0.3, 0.3, -0.3, 0.3, -0.3, 0.3,
			0.3, 0.3, -0.3, 0.3, 0.3, 0.3, 0.3, -0.3, 0.3,
			-0.3, -0.3, -0.3, -0.3, 0.3, -0.3, -0.3, -0.3, 0.3,
			-0.3, 0.3, -0.3, -0.3, 0.3, 0.3, -0.3, -0.3, 0.3,
			-0.3, 0.3, -0.3, 0.3, 0.3, 0.3, 0.3, 0.3, -0.3,
			-0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, -0.3,
			-0.3, -0.3, -0.3, -0.3, -0.3, 0.3, 0.3, -0.3, -0.3,
			-0.3, -0.3, 0.3, 0.3, -0.3, 0.3, 0.3, -0.3, -0.3 };
			
void error(int code, const char *desc) { fputs(desc, stderr); }

void loadTexture() {
	SDL_Surface *surface;
	surface = IMG_Load((char*)"/home/sasha/texture5.bmp");
	if (!surface || !surface->pixels) {
		std::cout << IMG_GetError() << std::endl;
	} else {
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &texture[0]);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,surface->w, surface->h,0,GL_RGB, GL_UNSIGNED_BYTE,surface->pixels);
	}
}

void sphereCount(float &x, float &y, float &lz, float z, float R, float phi, float &psi, float delta, int o) {
	if(o%2 == 0) {
		x = R*cosf(phi+delta)*cosf(psi);
		y = R*cosf(phi+delta)*sinf(psi);
		lz = R*sinf(phi+delta);
		psi-=delta;
	} else {
		x = R*cosf(phi)*cosf(psi);
		y = R*cosf(phi)*sinf(psi);
		lz = z;
	}
}

void sphereAdd(int i, float x, float y, float lz) {
	sphereVertex[i].push_back(x);
	sphereVertex[i].push_back(y);
	sphereVertex[i].push_back(lz); 
	sphereVertex[i].push_back(2*x);
	sphereVertex[i].push_back(2*y);
	sphereVertex[i].push_back(2*lz);
	sphereVertex[i].push_back((x+1)/2); 
	sphereVertex[i].push_back((y+1)/2);
}

void makeSphere() { //h=r=0.3
	float R = 0.3; // (h^2 + r^2)/2h
	int i = 0;
	sphereVertex.clear(); 
	for(float phi = -M_PI/2; phi <= 0; phi+=M_PI/d,i++) {   //32 and texture->16
		float z = R*sinf(phi), x, y, lz;
		sphereVertex.push_back(std::vector<float>());
		for(float psi = 0, o = 1; psi <= 2*M_PI+1; psi+=M_PI/d, o++) {
			sphereCount(x,y,lz,z,R,phi,psi,M_PI/d,o);
			sphereAdd(i,x,y,lz);	
		}
	}
	for(float phi = M_PI/2; phi >= 0; phi-=M_PI/d,i++) {
		float z = R*sinf(phi), x, y, lz;
		sphereVertex.push_back(std::vector<float>());
		for(float psi = 2*M_PI, o = 1; psi >= -1; psi-=M_PI/d, o++) {
			sphereCount(x,y,lz,z,R,phi,psi,-M_PI/d,o);
			sphereAdd(i,x,y,lz);
				
		}
	}
	
}

float myabs(float a) {
	return (a > 0 ? a : -a);
}

void makeConus() {
		conusVertex.clear(); 
		int i = 0;
		for(float r = 0, n = SPLIT; n > 0; n--, r+=0.3/SPLIT,i++) {
			conusVertex.push_back(std::vector<float>());
			for(float angle = 0, x, y, o = 1; angle <= 2*M_PI+1; angle+= M_PI/32, o++) {
				if((int)o%2 == 0) {
					x = (r+0.3/SPLIT)*cosf(angle), y = (r+0.3/SPLIT)*sinf(angle);
					angle-=M_PI/32;
				}
				else {
					x = r*cosf(angle), y = r*sinf(angle);
				}
				conusVertex[i].push_back(x);
				conusVertex[i].push_back(y);
				conusVertex[i].push_back(0);
				conusVertex[i].push_back(2*x);
				conusVertex[i].push_back(2*y);
				conusVertex[i].push_back(-1);
				conusVertex[i].push_back((x+1)/2); 
				conusVertex[i].push_back((y+1)/2);
			}
		}
		for(float z = 0.3/SPLIT, n = SPLIT, r = 0.3; n > 0 ; z+=0.3/SPLIT, r -=0.3/SPLIT, n--,i++) { //16 texture->3
			conusVertex.push_back(std::vector<float>());
			for(float angle = 0, x, y, lz, o = 1; angle <= 2*M_PI+1; angle+= M_PI/d, o++) { //32 texture->16
				if((int)o%2 == 0) {
					x = (r-0.3/SPLIT)*cosf(angle), y = (r-0.3/SPLIT)*sinf(angle), lz = z;
					angle-=M_PI/d;
				}
				else {
					x = r*cosf(angle), y = r*sinf(angle), lz = z-0.3/SPLIT;
				}
				conusVertex[i].push_back(x);
				conusVertex[i].push_back(y);
				conusVertex[i].push_back(lz);
				conusVertex[i].push_back(2*x);
				conusVertex[i].push_back(2*y);
				conusVertex[i].push_back(2*lz);
				conusVertex[i].push_back((x+1)/2); 
				conusVertex[i].push_back((y+1)/2);
			}
		}
}

void save() {
	std::ofstream file;
	file.open("data.txt");
	file << WIDTH << " " << HEIGHT << " " << spin_x << " " << spin_y << " " << spin_z << " " << SPLIT << " " << 
		x << " " << y << " " << z << " " << scale << " " << vx << " " << vy << " " << vz << " " << speed << " " <<
			fill << " " << move << " " << sphere << " " << cube << " " << lx << " " << ly << " " << lz  <<
				" " << t << " "  << d;
	file.close();
}

void load() {
	std::ifstream file;
	file.open("data.txt");
	if (file.peek() != EOF) {
		file >> WIDTH >> HEIGHT >> spin_x >> spin_y >> spin_z >> SPLIT >> 
		x >> y >> z >> scale >> vx >> vy >> vz >> speed >>
			fill >> move >> sphere >> cube >> lx >> ly >> lz  >> t >> d;
		start = glfwGetTime();
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
		glfwSetWindowShouldClose(window,GL_TRUE);
	else if(key == GLFW_KEY_Z && action != GLFW_RELEASE)
		spin_x+=5;
	else if(key == GLFW_KEY_X && action != GLFW_RELEASE)
		spin_y+=5;
	else if(key ==  GLFW_KEY_C && action != GLFW_RELEASE)
		spin_z+=5;

	else if(key == GLFW_KEY_A && action != GLFW_RELEASE)
		spin_x-=5;
	else if(key == GLFW_KEY_S && action != GLFW_RELEASE)
		spin_y-=5;
	else if(key ==  GLFW_KEY_D && action != GLFW_RELEASE)
		spin_z-=5;

	else if(key ==  GLFW_KEY_UP && action == GLFW_PRESS)
		scale+=0.1;
	else if(key ==  GLFW_KEY_DOWN && action == GLFW_PRESS)
		scale-=0.1;

	else if(key == GLFW_KEY_TAB && action == GLFW_PRESS)
		fill = !fill;
	else if(key == GLFW_KEY_EQUAL && action == GLFW_PRESS) 
		speed+=0.1;
	else if(key == GLFW_KEY_MINUS && action == GLFW_PRESS)	
		speed-=0.1;
	else if(key == GLFW_KEY_O && action != GLFW_RELEASE && SPLIT > 16) {
		SPLIT-=1;
		makeConus();
	} 
	else if(key == GLFW_KEY_P && action != GLFW_RELEASE && SPLIT != 3) {
		SPLIT+=1;
		makeConus();
	} else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		move = !move;
		start = glfwGetTime();
	} else if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		sphere = !sphere;
	} else if(key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS) 
		cube = !cube;
	else if(key == GLFW_KEY_1 && action == GLFW_PRESS) 
		save();
	else if(key == GLFW_KEY_2 && action == GLFW_PRESS)
		load();
	else if(key == GLFW_KEY_Q && action != GLFW_RELEASE)
		x-=0.1;
	else if(key ==  GLFW_KEY_W && action != GLFW_RELEASE)
		x+=0.1;
	else if(key == GLFW_KEY_E && action != GLFW_RELEASE)
		y-=0.1;
	else if(key ==  GLFW_KEY_R && action != GLFW_RELEASE)
		y+=0.1;
	else if(key == GLFW_KEY_3 && action != GLFW_RELEASE)
		lx-=0.1;
	else if(key ==  GLFW_KEY_4 && action != GLFW_RELEASE)
		lx+=0.1;
	else if(key == GLFW_KEY_5 && action != GLFW_RELEASE)
		ly-=0.1;
	else if(key ==  GLFW_KEY_6 && action != GLFW_RELEASE)
		ly+=0.1;
	else if(key == GLFW_KEY_7 && action != GLFW_RELEASE)
		lz-=0.1;
	else if(key ==  GLFW_KEY_8 && action != GLFW_RELEASE)
		lz+=0.1;
	else if(key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		t = !t;
		if(t) { 
			d = 16;
			makeSphere();
			SPLIT = 3;
			makeConus();
		} else {
			d = 32;
			makeSphere();
			SPLIT = 16;
			makeConus();
		}
	}
}

void hit() { //bot = y+0.3 = 0 //top = y-0.3 = 0 //left = x+0.3 = 0 //right = x-0.3 = 0 // c = (x,y,z) // R = 0.3*scale // //a=(1,0) b=(vx,vy)
	float botDist = myabs(y+0.3);
	float topDist = myabs(y-0.3);
	float leftDist = myabs(x+0.3);
	float rightDist = myabs(x-0.3);
	float farDist = myabs(z-0.3);
	float nearDist = myabs(z+0.3);
	float R = 0.3*scale;
	vx*=(leftDist>R && rightDist>R ? 1 : -1);
	vy*=(topDist>R && botDist>R ? 1 : -1);
	vz*=(farDist>R && nearDist>R ? 1 : -1);
}

void logOut(GLuint Shader) {
	GLint logLen;
	glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &logLen);
	if(logLen > 0) {
		char* log = (char*)malloc(logLen);
		GLsizei written;
		glGetShaderInfoLog(Shader, logLen, &written, log);
		std::cout << log << std::endl;
		free(log);
	}
}
void initShader() {
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	cubeShader = glCreateShader(GL_VERTEX_SHADER);
	if(!vertShader || !fragShader || !cubeShader) { 
		std::cout << "Error creating shaders" << std::endl;
		exit(1);
	}
	const GLchar* vertCode = ("#version 330\n" 
				"layout (location = 0) in vec3 VertexPosition;\n" 
				"layout (location = 1) in vec3 VertexNormal;\n"
				"layout (location = 2) in vec2 VertexTexCoord;\n"
				"out vec3 Position;\n"
				"out vec3 Normal;\n"
				"out vec2 TexCoord;\n"
				"uniform mat4 ProjectionMatrix, ModelMatrix;\n"
				"void main() {\n" 
				"	TexCoord = VertexTexCoord;\n"
				"	Normal = normalize(mat3(transpose(inverse(ModelMatrix)))*VertexNormal);\n"
				"	Position = vec3(ModelMatrix*vec4(VertexPosition,1));\n"
				"	gl_Position = ProjectionMatrix*ModelMatrix*vec4(VertexPosition, 1);\n" 
				"}\n") ;
	const GLchar* vertArray[] = {vertCode};
	glShaderSource(vertShader,1,vertArray,NULL);
	glCompileShader(vertShader);
	const GLchar* fragCode = ("#version 330\n" //vec3 viewPos (0,0,0); vec3 viewDir = viewPos - Position; vec3 r = reflect(lightDir, Normal)  pow(max(dot(r,viewDir), 0), 32)*Ks
				"in vec3 Position;\n"
				"in vec3 Normal;\n"
				"in vec2 TexCoord;\n"
				"uniform sampler2D Tex1;\n"
				"uniform bool t;\n"
				"struct LightInfo {\n"
				"	vec3 Position;\n"
				"	vec3 Intensity;\n"
				"};\n"
				"uniform LightInfo Light;\n"
				"struct MaterialInfo {\n"
				"	vec3 Ka;\n"
				"	vec3 Kd;\n"
				" 	float Ks;\n"
				"};\n"
				"uniform MaterialInfo Material;\n"
				"out vec4 FragColor;\n" 
				"void phongModel(vec3 pos, vec3 norm, out vec3 ambAndDiffspec) {\n"
				"	vec3 ambient = Light.Intensity*Material.Ka;\n"
				"	vec3 lightDir = normalize(Light.Position - Position);\n"
				"	float diff = max(dot(Normal, lightDir), 0);\n"
				"	vec3 diffuse = Light.Intensity*(diff * Material.Kd);\n"
				"	vec3 viewPos = vec3(0,0,0);\n"
				"	vec3 viewDir = normalize(viewPos - pos);\n"
				"	vec3 r = reflect(-lightDir, norm);\n"
				"	vec3 specular = vec3(pow(max(dot(r,viewDir), 0), 32)*Material.Ks*diff);\n"
				"	ambAndDiffspec = ambient  + diffuse + specular;\n"
				"}\n"
				"void main() {\n"
				"	vec3 ambAndDiffspec;\n"
				"	vec4 texColor = texture(Tex1, TexCoord);\n"
				"	phongModel(Position, Normal, ambAndDiffspec);\n"
				"	if(t) FragColor = vec4(ambAndDiffspec,1)*texColor;\n"
				"	else FragColor = vec4(ambAndDiffspec,1);\n"
				"}\n") ;
	const GLchar* fragArray[] = {fragCode};
	glShaderSource(fragShader,1,fragArray,NULL);
	glCompileShader(fragShader);
	const GLchar* cubeCode = ("#version 330\n"
				"layout (location = 0) in vec3 VertexPosition;\n"
				"uniform mat4 ProjectionMatrix;\n"
				"void main() {\n" 
				"	gl_Position = ProjectionMatrix*vec4(VertexPosition, 1);\n" 
				"}\n") ;
	const GLchar* cubeArray[] = {cubeCode};
	glShaderSource(cubeShader,1,cubeArray,NULL);
	glCompileShader(cubeShader);
	GLint vertResult, fragResult, cubeResult;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &vertResult);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fragResult);
	glGetShaderiv(cubeShader, GL_COMPILE_STATUS, &cubeResult);
	if(GL_FALSE == vertResult || GL_FALSE == fragResult || GL_FALSE == cubeResult) {
		std::cout << "Failed to compile shaders" << std::endl;
		logOut(vertShader); logOut(fragShader); logOut(cubeShader);
		exit(1);
	}
	programHandle = glCreateProgram();
	if(!programHandle) {
		std::cout << "Failed to create program" << std::endl;
		exit(1);
	}
	glAttachShader(programHandle,vertShader);
	glAttachShader(programHandle,fragShader);
	glLinkProgram(programHandle);
	GLint status;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
	if(GL_FALSE == status) {
		std::cout << "Failed to link program" << std::endl;
		exit(1);
	} else glUseProgram(programHandle);

	glBindAttribLocation(programHandle, 0, "VertexPosition");
	glBindAttribLocation(programHandle, 1, "VertexNormal");
	glBindAttribLocation(programHandle, 2, "VertexTexCoord");
	
	glm::mat4 m;
	m[0][0] = sin(45); m[0][1] = 0; m[0][2] = -sin(45); m[0][3] = sin(45)/2;
	m[1][0] =  0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
	m[2][0] = sin(45); m[2][1] = 0; m[2][2] = sin(45); m[2][3] = -sin(45)/2;
	m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
	GLuint location = glGetUniformLocation(programHandle, "ProjectionMatrix");
	if(location >= 0) 
		glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
	GLint lightIntensity = glGetUniformLocation(programHandle, "Light.Intensity");
	if(lightIntensity >= 0) { 
		glUniform3f(lightIntensity, 1, 1, 1);
	}
	GLint mAmb = glGetUniformLocation(programHandle, "Material.Ka");
	if(mAmb >= 0)
		glUniform3f(mAmb, 0.1, 0.1, 0.1);
	GLint mDif = glGetUniformLocation(programHandle, "Material.Kd");
	if(mDif >= 0)
		glUniform3f(mDif, 0.7, 0.7, 0.7);
	GLint mSpec = glGetUniformLocation(programHandle, "Material.Ks");
	if(mSpec >= 0)
		glUniform1f(mSpec, 0.5);
	GLint loc = glGetUniformLocation(programHandle, "Tex1");
	if(loc >= 0) 
		glUniform1i(loc, 0);
	else std::cout << "not detected" << std::endl;
	cubeHandle = glCreateProgram();
	if(!cubeHandle) {
		std::cout << "Failed to create cubeProgram" << std::endl;
		exit(1);
	}
	glAttachShader(cubeHandle,cubeShader);
	glLinkProgram(cubeHandle);
	GLint statusCube;
	glGetProgramiv(cubeHandle, GL_LINK_STATUS, &statusCube);
	if(GL_FALSE == statusCube) {
		std::cout << "Failed to link cubeProgram" << std::endl;
		exit(1);
	} else glUseProgram(cubeHandle);
	glBindAttribLocation(cubeHandle, 0, "VertexPosition");
	GLuint locate = glGetUniformLocation(cubeHandle, "ProjectionMatrix");
	if(locate >= 0) 
		glUniformMatrix4fv(locate, 1, GL_FALSE, &m[0][0]);
}

GLuint positionBufferHandle;
void shInit() {
	GLuint vboHandles[2];
	glGenBuffers(2, vboHandles);
	positionBufferHandle = vboHandles[0];
	GLuint colorBufferHandle = vboHandles[1];
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6*sizeof(float)));
	glGenVertexArrays(2, &vaoCubeHandle);
	glBindVertexArray(vaoCubeHandle);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)NULL);
}
	
void shDisplay() {
	GLfloat lightCube[] = 
{lx-0.5-0.04, ly+0.5-0.04, lz+0.2-0.04, lx-0.5-0.04, ly+0.5+0.04, lz+0.2-0.04,  lx-0.5+0.04, ly+0.5-0.04, lz+0.2-0.04,
lx-0.5-0.04, ly+0.5+0.04, lz+0.2-0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2-0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2-0.04,
lx-0.5-0.04, ly+0.5-0.04, lz+0.2+0.04, lx-0.5-0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2+0.04,
lx-0.5-0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2+0.04,
lx-0.5+0.04, ly+0.5-0.04, lz+0.2-0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2-0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2+0.04,
lx-0.5+0.04, ly+0.5+0.04, lz+0.2-0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2+0.04,
lx-0.5-0.04, ly+0.5-0.04, lz+0.2-0.04, lx-0.5-0.04, ly+0.5+0.04, lz+0.2-0.04, lx-0.5-0.04, ly+0.5-0.04, lz+0.2+0.04,
lx-0.5-0.04, ly+0.5+0.04, lz+0.2-0.04, lx-0.5-0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5-0.04, ly+0.5-0.04, lz+0.2+0.04,
lx-0.5-0.04, ly+0.5+0.04, lz+0.2-0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2-0.04,
lx-0.5-0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5+0.04, lz+0.2-0.04,
lx-0.5-0.04, ly+0.5-0.04, lz+0.2-0.04, lx-0.5-0.04, ly+0.5-0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2-0.04,
lx-0.5-0.04, ly+0.5-0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2+0.04, lx-0.5+0.04, ly+0.5-0.04, lz+0.2-0.04 };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programHandle);
	glBindVertexArray(vaoHandle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	GLint tx = glGetUniformLocation(programHandle, "t");
	if(tx >= 0) {
		if(t) glUniform1i(tx, 1);
		else glUniform1i(tx, 0);
	}
	glm::mat4 tm = glm::translate(glm::mat4(1), glm::vec3(x,y,z));
	glm::mat4 rmx = glm::rotate(tm, glm::radians((float)spin_x), glm::vec3(1,0,0));
	glm::mat4 rmy = glm::rotate(rmx, glm::radians((float)spin_y), glm::vec3(0,1,0));
	glm::mat4 rmz = glm::rotate(rmy, glm::radians((float)spin_z), glm::vec3(0,0,1));
	glm::mat4 sm = glm::scale(rmz, glm::vec3(scale,scale,scale));
	GLuint modelLocation = glGetUniformLocation(programHandle, "ModelMatrix");
	if(modelLocation >= 0)
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &sm[0][0]);
	GLint lightPos = glGetUniformLocation(programHandle, "Light.Position");
	if(lightPos >= 0)
		glUniform3f(lightPos, lx-0.5, ly+0.5, lz+0.2);
	if(fill) glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	else glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	if(sphere) {
		for(int i = 0; i < sphereVertex.size(); i++) {
			glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
			glBufferData(GL_ARRAY_BUFFER, sphereVertex[i].size()*sizeof(float), sphereVertex[i].data(), GL_STATIC_DRAW);
			glDrawArrays(GL_TRIANGLE_STRIP, 0 , sphereVertex[i].size()/8);
		}
	} else {
		for(int i = 0; i < conusVertex.size(); i++) {
			glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
			glBufferData(GL_ARRAY_BUFFER, conusVertex[i].size()*sizeof(float), conusVertex[i].data(), GL_STATIC_DRAW);
			glDrawArrays(GL_TRIANGLE_STRIP, 0 , conusVertex[i].size()/8);
		}
	}
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glUseProgram(cubeHandle);
	glBindVertexArray(vaoCubeHandle);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, 108*sizeof(float), lightCube, GL_STATIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0 , 36);
	if(cube) {
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
		glBufferData(GL_ARRAY_BUFFER, 108*sizeof(float), staticCube, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLES, 0 , 36);
	}
	if(move) {
		float t = glfwGetTime();
		float dt = t - start;
		start = t;
		x+=speed*vx*dt;
		y+=speed*vy*dt;
		z+=speed*vz*dt;
		hit();		
	}
}

void shFree() {
	glDeleteProgram(programHandle);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	glDeleteVertexArrays(1, &vaoHandle);
}

int main() {
	glfwSetErrorCallback(error);
	if(!glfwInit()) {
		std::cout << "Failed to init glfw" << std::endl;
		exit(1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "EighthLab", NULL, NULL);
	if(!window) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		exit(1);
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE; 
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		std::cout << "Error initializing GLEW" << std::endl;
		exit(1);
	}
	glViewport(0, 0, WIDTH, HEIGHT);
	initShader();
	glfwSetKeyCallback(window, key_callback);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_COLOR_MATERIAL);
	loadTexture();
	glClearColor(0,0,0,0);
	makeSphere();
	makeConus();
	shInit();
	while(!glfwWindowShouldClose(window)) {
		shDisplay();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	shFree();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
	
