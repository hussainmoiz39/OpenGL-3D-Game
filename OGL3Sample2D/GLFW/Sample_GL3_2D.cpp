#include<bits/stdc++.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include<ao/ao.h>
#include<mpg123.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define pb push_back
#define mp make_pair
#define b_s binary_search
#define BITS 8


using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
	double x;
	double y;
	double z;
	double length;
	double breadth;
	double height;
	int flag;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

mpg123_handle *mh;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;

int driver;
ao_device *dev;

ao_sample_format format;
int channels, encoding;
long rate;

void audio_init() {
	/* initializations */
	ao_initialize();
	driver = ao_default_driver_id();
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size= 3000;
	buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "./moiz.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);
}

void audio_play() {
	/* decode and play */
	if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
		ao_play(dev, (char*) buffer, done);
	else mpg123_seek(mh, 0, SEEK_SET);
}

void audio_close() {
	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
	ao_shutdown();
}







/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->x = -55;
	vao->y = -55;
	vao->z = -55;
	vao->length = 0;
	vao->breadth = 0;
	vao->height = 0;
	vao->flag=-1;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int arry[] = {1,2,3,6,7,11,12,13,16,17,21,22,23,26,27,31,32,33,34,35,36,37,40,43,47,50,52,53,54,57,60,62,63,64,67,70,72,73,74,77,78,79,80,84,85,86,87,88,89,90,97,98,99,100};
vector<int> checklist(arry,arry+56);//54

int arry2[] = {6};
vector<int> swtch(arry2,arry2+1);

int arry3[] = {19,20};
vector<int> bridge(arry3,arry3+2);

int arry4[] = {4,23,25,30};
vector<int> fragile(arry4,arry4+4);

int arry5[] = {48};
vector<int> target(arry5,arry5+1);

int bridgeflag=0;

int standonfragile=0;
int targetreached=0;
int gameflag=0;

int view=0;
int cubecnt=1;
int orientation=0; //0->standing,1->sitting
int flg1=0,flg2=0,flg3=0,flg4=0;
int gflag=0;
double movex,movey,movez;
int mychk=0;
int mytarget=0;
int lr=0,ud=0;;

int gameend=0;

int ismousepressed=0;




/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_V:
				view++;
				view=view%5;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;


			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		if(glfwGetKey(window,GLFW_KEY_ESCAPE))
		{
			quit(window);
		}
		if( glfwGetKey(window,GLFW_KEY_DOWN) )
		{
			flg1=1;
			if(orientation==0)
			{
				movex+=0;
				movey-=1.5;
				movez-=0.5;
				//cout<<"1:0"<<endl;
			}
			else if(orientation==1)
			{
				movex+=0;
				movey-=1.5;
				movez+=0.5;
				//mychk=0;
				//cout<<"1:1"<<endl;
			}
			else if(orientation==2)
			{
				movex+=0;
				movey-=1;
				movez+=0;
				//cout<<"1:2"<<endl;
			}
		}
		if( glfwGetKey(window,GLFW_KEY_UP) )
		{
			flg2=1;
			if(orientation==0)
			{
				movex+=0;
				movey+=1.5;
				movez-=0.5;
				//cout<<"2:0"<<endl;
			}
			else if(orientation==1)
			{
				movex+=0;
				movey+=1.5;
				movez+=0.5;
				//mychk=0;
				//cout<<"2:1"<<endl;
			}
			else if(orientation==2)
			{
				movex+=0;
				movey+=1;
				movez+=0;
				//cout<<"2:2"<<endl;
			}
		}
		if( glfwGetKey(window,GLFW_KEY_LEFT) )
		{
			flg3=1;
			if(orientation==0)
			{
				movex-=1.5;
				movey+=0;
				movez-=0.5;
				//cout<<"3:0"<<endl;
			}
			else if(orientation==1)
			{
				movex-=1;
				movey+=0;
				movez+=0;
				//mychk=1;
				//cout<<"3:1"<<endl;
			}
			else if(orientation==2)
			{
				movex-=1.5;
				movey+=0;
				movez+=0.5;
				//cout<<"3:2"<<endl;
			}
		}
		if( glfwGetKey(window,GLFW_KEY_RIGHT) )
		{
			flg4=1;
			if(orientation==0)
			{
				movex+=1.5;
				movey+=0;
				movez-=0.5;
				//cout<<"4:0"<<endl;
			}
			else if(orientation==1)
			{
				movex+=1;
				movey+=0;
				movez+=0;
				//mychk=1;
				//cout<<"4:1"<<endl;
			}
			else if(orientation==2)
			{
				movex+=1.5;
				movey+=0;
				movez+=0.5;
			}
		}




	}
}






/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;

		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS)
				ismousepressed=1;
			if(action == GLFW_RELEASE)
				ismousepressed=0;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	//Matrices.projection = glm::perspective (30.0f, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 5000.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *square;
VAO *cube[101],*border[101];
VAO *player;
int border_cnt=0;
void createCube(double l,double w,double h,double x,double y,double z,int i,int choice,int flag)
{
	static const GLfloat vertex_buffer_data [] = {
		-l/2,w/2,-h/2,
		-l/2,-w/2,-h/2,
		l/2,-w/2,-h/2,
		-l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		l/2,-w/2,-h/2,//bottom

		l/2,-w/2,h/2,
		l/2,-w/2,-h/2,
		l/2,w/2,-h/2,
		l/2,-w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,-h/2,//right

		-l/2,w/2,h/2,
		-l/2,-w/2,h/2,
		l/2,-w/2,h/2,
		-l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,-w/2,h/2,//top

		-l/2,-w/2,h/2,
		-l/2,-w/2,-h/2,
		-l/2,w/2,-h/2,
		-l/2,-w/2,h/2,
		-l/2,w/2,h/2,
		-l/2,w/2,-h/2,//left

		-l/2,-w/2,h/2,
		-l/2,-w/2,-h/2,
		l/2,-w/2,-h/2,
		-l/2,-w/2,h/2,
		l/2,-w/2,h/2,
		l/2,-w/2,-h/2,//front

		-l/2,w/2,h/2,
		-l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		-l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,-h/2, //end
	};

	if(choice==3)//target
	{

		static const GLfloat color_buffer_data [] = {

			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,//bottom

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,//right

			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,
			0.2,0.2,0.2,//top

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1, //left

			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0, //front

			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0, //end
		};


		cube[cubecnt] = create3DObject(GL_TRIANGLES,36,vertex_buffer_data,color_buffer_data,GL_FILL);
		cube[cubecnt]->x = x;
		cube[cubecnt]->y = y;
		cube[cubecnt]->z = z;
		cube[cubecnt]->length = l;
		cube[cubecnt]->breadth = w;
		cube[cubecnt]->height = h;
		cube[cubecnt]->flag = flag;
		cubecnt++;

	}


	if(choice==2)//fragile
	{

		static const GLfloat color_buffer_data [] = {

			1,0,0,
			1,0,0,
			1,0,0,
			1,0.5,0,
			1,0.5,0,
			1,0.5,0,//bottom

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,//right

			1,0,0,
			1,0,0,
			1,0,0,
			1,0.5,0,
			1,0.5,0,
			1,0.5,0,//top

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1, //left

			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0, //front

			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0, //end
		};


		cube[cubecnt] = create3DObject(GL_TRIANGLES,36,vertex_buffer_data,color_buffer_data,GL_FILL);
		cube[cubecnt]->x = x;
		cube[cubecnt]->y = y;
		cube[cubecnt]->z = z;
		cube[cubecnt]->length = l;
		cube[cubecnt]->breadth = w;
		cube[cubecnt]->height = h;
		cube[cubecnt]->flag = flag;
		cubecnt++;

	}

	if(choice==1) //switch
	{

		static const GLfloat color_buffer_data [] = {

			1,0,0,
			1,0,0,
			1,0,0,
			0.85,0.49,0.85,
			0.85,0.49,0.85,
			0.85,0.49,0.85,//bottom

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,//right

			1,0,0,
			1,0,0,
			1,0,0,
			0.85,0.49,0.85,
			0.85,0.49,0.85,
			0.85,0.49,0.85,//top

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1, //left

			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0, //front

			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0, //end
		};


		cube[cubecnt] = create3DObject(GL_TRIANGLES,36,vertex_buffer_data,color_buffer_data,GL_FILL);
		cube[cubecnt]->x = x;
		cube[cubecnt]->y = y;
		cube[cubecnt]->z = z;
		cube[cubecnt]->length = l;
		cube[cubecnt]->breadth = w;
		cube[cubecnt]->height = h;
		cube[cubecnt]->flag = flag;
		cubecnt++;

	}


	if(choice==0) //normal
	{

		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			0.62,0.62,0.37,
			0.62,0.62,0.37,
			0.62,0.62,0.37,//bottom

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,//right

			1,0,0,
			1,0,0,
			1,0,0,
			0.62,0.62,0.37,
			0.62,0.62,0.37,
			0.62,0.62,0.37,//top

			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,
			0,0,1,//left

			0,0.5,0, //0.5,1,0.5
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,//front

			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,
			0,0.5,0,//end
		};


		cube[cubecnt] = create3DObject(GL_TRIANGLES,36,vertex_buffer_data,color_buffer_data,GL_FILL);
		cube[cubecnt]->x = x;
		cube[cubecnt]->y = y;
		cube[cubecnt]->z = z;
		cube[cubecnt]->length = l;
		cube[cubecnt]->breadth = w;
		cube[cubecnt]->height = h;
		cube[cubecnt]->flag = flag;
		cubecnt++;

	}

}


void createPlayer(double l,double w,double h,double x,double y,double z)
{
	static const GLfloat vertex_buffer_data [] = {
		-l/2,w/2,-h/2,
		-l/2,-w/2,-h/2,
		l/2,-w/2,-h/2,
		-l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		l/2,-w/2,-h/2,//bottom

		l/2,-w/2,h/2,
		l/2,-w/2,-h/2,
		l/2,w/2,-h/2,
		l/2,-w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,-h/2,//right

		-l/2,w/2,h/2,
		-l/2,-w/2,h/2,
		l/2,-w/2,h/2,
		-l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,-w/2,h/2,//top

		-l/2,-w/2,h/2,
		-l/2,-w/2,-h/2,
		-l/2,w/2,-h/2,
		-l/2,-w/2,h/2,
		-l/2,w/2,h/2,
		-l/2,w/2,-h/2,//left

		-l/2,-w/2,h/2,
		-l/2,-w/2,-h/2,
		l/2,-w/2,-h/2,
		-l/2,-w/2,h/2,
		l/2,-w/2,h/2,
		l/2,-w/2,-h/2,//front

		-l/2,w/2,h/2,
		-l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		-l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,-h/2, //end
	};	








	static const GLfloat color_buffer_data [] = {
		0.30,0.18,0.30, //0.8,0.49,0.19
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,//bottom

		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,//right

		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,//top

		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,
		0.13,0.13,0.55,//left

		3,3,3,
		3,3,3,
		3,3,3,
		3,3,3,
		3,3,3,
		3,3,3,//front

		3,3,3,
		3,3,3,
		3,3,3,
		3,3,3,
		3,3,3,
		3,3,3,//end
	};



	player = create3DObject(GL_TRIANGLES,36,vertex_buffer_data,color_buffer_data,GL_FILL);
	player->x = x;
	player->y = y;
	player->z = z;
	player->length = l;
	player->breadth = w;
	player->height = h;

}



void createBorder (double l,double w,double h,double x,double y,double z)
{


	static const GLfloat vertex_buffer_data [] = {
		-l/2,w/2,h/2,
		-l/2,-w/2,h/2,
		-l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,-w/2,h/2,
		l/2,-w/2,h/2,
		-l/2,-w/2,h/2, //top

		-l/2,w/2,-h/2,
		-l/2,-w/2,-h/2,
		-l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		l/2,-w/2,-h/2,
		l/2,-w/2,-h/2,
		-l/2,-w/2,-h/2, //bottom

		-l/2,-w/2,h/2,
		-l/2,-w/2,-h/2,
		-l/2,-w/2,h/2,
		l/2,-w/2,h/2,
		l/2,-w/2,h/2,
		l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		-l/2,-w/2,-h/2, //front

		-l/2,w/2,h/2,
		-l/2,w/2,-h/2,
		-l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		-l/2,w/2,-h/2, //end

		-l/2,-w/2,h/2,
		-l/2,-w/2,-h/2,
		-l/2,-w/2,h/2,
		-l/2,w/2,h/2,
		-l/2,w/2,h/2,
		-l/2,w/2,-h/2,
		-l/2,w/2,-h/2,
		-l/2,-w/2,-h/2, //left

		l/2,-w/2,h/2,
		l/2,-w/2,-h/2,
		l/2,-w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,h/2,
		l/2,w/2,-h/2,
		l/2,w/2,-h/2,
		l/2,-w/2,-h/2, //right



	};







	static const GLfloat color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
		0,0,0,
	};
	border[border_cnt] = create3DObject(GL_LINES,48,vertex_buffer_data,color_buffer_data,GL_LINES);
	border[border_cnt]->length = l;
	border[border_cnt]->breadth = w;
	border[border_cnt]->height = h;
	border[border_cnt]->x = x;
	border[border_cnt]->y = y;
	border[border_cnt]->z = z;
	border_cnt++;
}








// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1.2,-1,0, // vertex 1
		1.2,-1,0, // vertex 2
		1.2, 1,0, // vertex 3

		1.2, 1,0, // vertex 3
		-1.2, 1,0, // vertex 4
		-1.2,-1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSquare ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
		1, 1,0, // vertex 3

		1, 1,0, // vertex 3
		-1, 1,0, // vertex 4
		-1,-1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0,0,0, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	square = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);


	glm::vec3 eye;
	glm::vec3 target;
	glm::vec3 up;

	if(view==0)//Tower view
	{
		eye = glm::vec3(3,-4,6);//-4,1,9
		target = glm::vec3(0,0,0);
		up = glm::vec3(0,1,0);
		Matrices.projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.1f,500.0f);
	}
	if(view==1)//Top View
	{
		eye = glm::vec3(5*cos(camera_rotation_angle*M_PI/180.0f),0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
		target = glm::vec3(0,0,0);
		up = glm::vec3(0,1,0);
		Matrices.projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.1f,500.0f);
	}
	if(view==2)//Follow-Cam view
	{
		eye = glm::vec3(-5+movex+1.5,5+movey+2,2+movez+0.5);//+2,+2.5,+1
		target = glm::vec3(-5+movex+1,5+movey-1,0.6); //+2,0,+0.6
		up = glm::vec3(1,-1,3);//1,-1,3
		Matrices.projection = glm::ortho(-2.5f,2.5f,-2.5f,2.5f,0.1f,25.0f);
	}
	if(view==3) //Block view
	{
		
		eye = glm::vec3(-5+movex-1,5+movey-3,5);
		target = glm::vec3(-5+movex+3,5+movey-2,0.6);
		up = glm::vec3(1,1,-1);//1,-1,3
		Matrices.projection = glm::ortho(-2.5f,2.5f,-2.5f,2.5f,0.1f,100.0f);
	}

	if(view==4)
	{
		camera_rotation_angle = 600;
		double xpos,ypos;
		if(ismousepressed==1)
		{
			glfwGetCursorPos(window,&xpos,&ypos);
			//glfwGetFramebufferSize(window,&width,&height);
			xpos = (xpos/30)+(-10);
			ypos = (ypos/30)+(-10);
			//ypos*=-1;
			eye = glm::vec3(-ypos,-5,-xpos); //xpos,2,ypos
		}
		if(ismousepressed==0)
		{
			eye=glm::vec3(2*sin(camera_rotation_angle*M_PI/180.0f),2*cos(camera_rotation_angle*M_PI/180.0f),5) ;
		}
		target=glm::vec3(0,0,0);
		up=glm::vec3(0,0,1);
		Matrices.projection=glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.1f,500.0f);
		
	}








	/*	
	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f),0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	//glm::vec3 eye (0,1,1);
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0,0, 0); //0,0,0
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);//0,0,0
	 */


	// Compute Camera matrix (view)
	Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	//Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);








	/* Render your scene */

	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	Matrices.model *= triangleTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	// draw3DObject(triangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(rectangle);


	glm::mat4 translateSquare = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef

	Matrices.model *= (translateSquare);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(square);

	for(int i=1;i<cubecnt;i++)
	{


		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateCube = glm::translate (glm::vec3(cube[i]->x, cube[i]->y, cube[i]->z));        // glTranslatef
		Matrices.model *= (translateCube);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		if( b_s(bridge.begin(),bridge.end(),i) && bridgeflag==0)//switch not pressed yet
		{
			continue;
		}

		else if(orientation==0 && movex==1 && movey==0 && movez==0)
		{
			bridgeflag=1;
			draw3DObject(cube[i]);
		}
		else
		{
			draw3DObject(cube[i]);
		}

	}

	for(int i=0;i<border_cnt;i++)
	{
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateBorder = glm::translate (glm::vec3(player->x,player->y,player->z));        // glTranslate
		Matrices.model *= (translateBorder);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(border[i]);
	}


	if(gameend==1)   //game has ended
	{
		quit(window);
	}

	else if(targetreached==1 || standonfragile==1)  //target has been reached
	{
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 tP1 = glm::translate(glm::vec3(0,0,0));

		glm::mat4 tP2 = glm::translate(glm::vec3(-5.0f+movex,5+movey,-55));
		Matrices.model *=(tP2*tP1);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(player);
		if(targetreached==1)
		{
			cout<<"YOU WIN "<<endl;
		}
		else if(standonfragile==1)
		{
			cout<<"YOU LOSE "<<endl;
		}
		gameend=1;

	}



	else if( (flg1==1||flg2==1||flg3==1||flg4==1) && targetreached==0 ) //direction signal
	{
		//cout<<"Moiz"<<endl;
		//cout<<"flg1: "<<flg1<<" "<<"flg2: "<<flg2<<" "<<"flg3: "<<flg3<<" flg4: "<<flg4<<endl;
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 tP1 = glm::translate(glm::vec3(0,0,0));

		glm::mat4 rotateP1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0));
		glm::mat4 rotateP2 = glm::rotate((float)(-90*M_PI/180.0f),glm::vec3(1,0,0));
		glm::mat4 rotateP3 = glm::rotate((float)(90*M_PI/180.0f),glm::vec3(0,1,0));
		glm::mat4 rotateP4 = glm::rotate((float)(-90*M_PI/180.0f),glm::vec3(0,1,0));
		glm::mat4 rotateP5 = glm::rotate((float)(90*M_PI/180.0f),glm::vec3(0,0,1));

		glm::mat4 tP2 = glm::translate(glm::vec3(-5.0f+movex,5+movey,2.0f+movez));

		double a = -5+movex;
		double b =  5+movey;

		if(flg1==1||flg2==1)//up-down
		{
			ud=1;lr=0;
			if(orientation==0)//standing
			{
				mychk=0;//st-sit
				orientation=1;
				if(mytarget==0)
				{
					Matrices.model *=(tP2*rotateP1*tP1);
					mytarget=0;
					//cout<<"O:0 mt:0"<<endl;
				}
				else if(mytarget==1)
				{
					Matrices.model *=(tP2*rotateP5*rotateP3*tP1);
					mytarget=1;
					//cout<<"O:0 mt:1"<<endl;
				}
			}
			else if(orientation==1)
			{
				orientation=0;
				if(mytarget==0)
				{
					Matrices.model *=(tP2*rotateP1*rotateP5*tP1);
					mytarget=0;
					//cout<<"O:1 mt:0"<<endl;
				}
				else if(mytarget==1)
				{
					Matrices.model *=(tP2*rotateP5*tP1);
					mytarget=1;
					//cout<<"O:1 mt:1"<<endl;
				}
			}
			else if(orientation==2)
			{
				mychk=1;
				if(mytarget==0)
				{
					Matrices.model *=(tP2*rotateP3*tP1);
					mytarget=1;
					//cout<<"O:2 mt:0"<<endl;
				}
				else if(mytarget==1)
				{
					Matrices.model *=(tP2*rotateP5*rotateP2*tP1);//2,4
					mytarget=0;
					//cout<<"O:2 mt:1"<<endl;
				}
			}

		}

		if(flg3==1||flg4==1)//left-right
		{
			lr=1;ud=0;
			if(orientation==0)
			{
				mychk=1;
				orientation=2;
				if(mytarget==0)
				{
					mytarget=1;
					Matrices.model *=(tP2*rotateP3*tP1);
					//cout<<"O:0 mt:0"<<endl;
				}
				else if(mytarget==1)
				{
					mytarget=0;
					Matrices.model *=(tP2*rotateP5*rotateP2*tP1);//r5,r2
					//cout<<"O:0 mt:1"<<endl;
				}
				//Matrices.model *=(tP2*rotateP5*rotateP4*tP1);
			}

			else if(orientation==1)
			{
				orientation=1;

				if(mytarget==0)
				{
					mytarget=1;
					Matrices.model *=(tP2*rotateP5*rotateP3*tP1);//5,3
					//cout<<"O:1 mt:0"<<endl;
				}
				else if(mytarget==1)
				{
					mytarget=0;
					Matrices.model *=(tP2*rotateP2*tP1);
					//cout<<"O:1 mt:1"<<endl;
				}
			}
			else if(orientation==2)
			{
				orientation=0;
				if(mytarget==0)
				{
					mytarget=1;
					Matrices.model *=(tP2*rotateP5*tP1);
					//cout<<"O:2 mt:0"<<endl;
				}
				else if(mytarget==1)
				{
					mytarget=0;
					Matrices.model *=(tP2*tP1);
					//cout<<"O:2 mt:1"<<endl;
				}
			}
			


		}




		flg1=0;flg2=0;flg3=0;flg4=0;

		//cout<<"Orientation is "<<orientation<<"mychk is "<<mychk<<"mytarget is "<<mytarget<<endl;


		MVP = VP*Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(player);

	}

	else if( targetreached==0 && (flg1==0&&flg2==0&&flg3==0&&flg4==0) )
	{
		//cout<<"Check"<<endl;
	
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 tP1 = glm::translate(glm::vec3(0,0,0));

		glm::mat4 rotateP2 = glm::rotate((float)(-90*M_PI/180.0f),glm::vec3(1,0,0));
		glm::mat4 rotateP1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(1,0,0));
		glm::mat4 rotateP3 = glm::rotate((float)(90*M_PI/180.0f),glm::vec3(0,1,0));
		glm::mat4 rotateP5 = glm::rotate((float)(90*M_PI/180.0f),glm::vec3(0,0,1));
		glm::mat4 rotateP4 = glm::rotate((float)(-90*M_PI/180.0f),glm::vec3(0,1,0));

		glm::mat4 tP2 = glm::translate (glm::vec3(-5.0f+movex,5+movey,2.0f+movez));        // glTranslate




		double a =-5+movex;
		double b = 5+movey;


		int testflag=0;//not found

		if(orientation==0)
		{
			
			for(int i=1;i<cubecnt;i++)
			{
				if(cube[i]->x==a && cube[i]->y==b)
				{
					if( b_s(fragile.begin(),fragile.end(),i) )
					{
						testflag=0;
						standonfragile=1;
					}
					else
					{
						testflag=1;
						break;
					}
					
				}
			}


		}
		else if(orientation==1)
		{
			double y1,y2;
			int flag1=0,flag2=0;
			y1=b+0.5;y2=b-0.5;
			for(int i=1;i<cubecnt;i++)
			{
				if(cube[i]->x==a && cube[i]->y==y1)
				{
					flag1=1;
				}
				else if(cube[i]->x==a && cube[i]->y==y2)
				{
					flag2=1;
				}
			}
			//cout<<"flg1 "<<flg1<<" flg2 "<<flg2<<endl;
			if(flag1==1 && flag2==1)
			{
				testflag=1;
			}

			//cout<<"orientation "<<orientation<<" flg1 "<<flg1<<" flg2 "<<flg2<<" testflag "<<testflag<<endl;
		}
		else if(orientation==2)
		{
			double x1,x2;
			int flag1=0,flag2=0;
			x1=a-0.5;x2=a+0.5;
			for(int i=1;i<cubecnt;i++)
			{
				if(cube[i]->x==x1 && cube[i]->y==b)
				{
					flag1=1;
				}
				else if(cube[i]->x==x2 && cube[i]->y==b)
				{
					flag2=1;
				}
			}

			//cout<<"flg1 "<<flg1<<" flg2 "<<flg2<<endl;

			if(flag1==1 && flag2==1)
			{
				testflag=1;
			}

			//cout<<"orientation "<<orientation<<" flg1 "<<flg1<<" flg2 "<<flg2<<" testflag "<<testflag<<endl;

		}



		//cout<<"orientation "<<orientation<<endl;
		if(testflag==0)
		{
			gameend=1;
			cout<<"YOU LOSE"<<endl;
		}

		else
		{


			//cout<<"fffffff"<<endl;
			//cout<<"orientation: "<<orientation<<" mytarget "<<mytarget<<endl;


			if(movex==8 && movey==-7 && movez==0 && orientation==0)//check target
			{
				targetreached=1;
				gameflag=1;
				Matrices.model *=(tP2*tP1);
			}

			else if(orientation==0 && ud==0 && lr==0)		{
				Matrices.model *=(tP2*tP1);
			}


			else if(orientation==0 && ((ud!=0)||(lr!=0)))
			{
				//cout<<"hello1"<<endl;
				if(mytarget==0 && ud==1)
				{
					Matrices.model *=(tP2*tP1);
				}
				else if(mytarget==1 && ud==1)
				{
					Matrices.model *=(tP2*rotateP5*tP1);
				}
				else if(mytarget==0 && lr==1)
				{
					Matrices.model *=(tP2*tP1);
				}
				else if(mytarget==1 && lr==1)
				{
					Matrices.model *=(tP2*rotateP5*tP1);
				}
				//Matrices.model *= (tP2*tP1);
			}
			else if(orientation==1 && mychk==0)//up/down
			{
				//cout<<"hello2"<<endl;
				if(mytarget==0 && ud==1)
				{
					Matrices.model *=(tP2*rotateP1*tP1);
				}
				else if(mytarget==1 && ud==1)
				{
					Matrices.model *=(tP2*rotateP5*rotateP3*tP1);
				}
				else if(mytarget==0 && lr==1)
				{
					Matrices.model *=(tP2*rotateP2*tP1);
				}
				else if(mytarget==1 && lr==1)
				{
					Matrices.model *=(tP2*rotateP5*rotateP3*tP1);
				}
				//Matrices.model *=(tP2*rotateP1*tP1);
			}
			else if(orientation==2 && mychk==1)
			{
				//cout<<"hello3"<<endl;
				if(mytarget==0 && ud==1)
				{
					Matrices.model *=(tP2*rotateP5*rotateP2*tP1);
				}
				else if(mytarget==1 && ud==1)
				{
					Matrices.model *=(tP2*rotateP3*tP1);//r5,r2
				}
				else if(mytarget==0 && lr==1)
				{
					Matrices.model *=(tP2*rotateP5*rotateP2*tP1);//r5
				}
				else if(mytarget==1 && lr==1)
				{
					Matrices.model *=(tP2*rotateP3*tP1);
				}


			}

		}

		//cout<<orientation<<" "<<mychk<<" "<<mytarget<<endl;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(player);
		//cout<<-5+movex<<" "<<5+movey<<" "<<2+movez<<endl;
	}

	//cout<<"Player "<<player->x<<" "<<player->y<<" "<<player->z<<endl;
	//cout<<"Cube "<<cube[1]->x<<" "<<cube[1]->y<<" "<<cube[1]->z<<endl;



	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
	int i,j,t;
	double x=-6;

	for(i=0;i<10;i++)
	{
		x++;
		double y = 6;

		for(j=0;j<10;j++)
		{
			y--;
			t=(i*10)+j+1;
			if( b_s(checklist.begin(),checklist.end(),t) )
			{

				if( t==11)
				{
					createCube(1,1,1,x,y,0.5,t,1,1);//switch
				}
				else if(t==6||t==40||t==47||t==57)
				{
					createCube(1,1,1,x,y,0.5,t,2,1);//fragile
				}
				else if(t==88)
				{
					createCube(1,1,1,x,y,0.5,t,3,1);//target
				}
				else
				{
					createCube(1,1,1,x,y,0.5,t,0,1);//normal
				}
			}
		


		}
	}
	createPlayer(1,1,2,-5,5,2);//-2.95,2.7,0.3,1.2
	




	/* A Vertex Buffer Object (VBO) is a memory buffer in the high speed memory of your video card designed to hold information about vertices.
	   A Vertex Array Object (VAO) is an object which contains one or more Vertex Buffer Objects and is designed to store the information for a complete rendered object. 
	   In our example this is a diamond consisting of four vertices as well as a color for each vertex.
	 */

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;


	GLFWwindow* window = initGLFW(width, height);

	audio_init();

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw(window);

		audio_play();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	audio_close();
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
