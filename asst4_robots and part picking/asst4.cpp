////////////////////////////////////////////////////////////////////////
//
//   Harvard University
//   CS175 : Computer Graphics
//   Professor Steven Gortler
//
////////////////////////////////////////////////////////////////////////
//	These skeleton codes are later altered by Ming Jin,
//	for "CS6533: Interactive Computer Graphics", 
//	taught by Prof. Andy Nealen at NYU
////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
// #if __GNUG__
// #   include <tr1/memory>
// #endif

#include <GL/glew.h>
#ifdef __MAC__
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "cvec.h"
#include "matrix4.h"
#include "geometrymaker.h"
#include "ppm.h"
#include "glsupport.h"
#include "arcball.h"    // -----------change
#include "quat.h"        // -----------change
#include "rigtform.h"  // -----------change

#include "asstcommon.h"
#include "scenegraph.h"
#include "drawer.h"
#include "picker.h"

using namespace std;      // for string, vector, iostream, and other standard C++ stuff
// using namespace tr1; // for shared_ptr

// G L O B A L S ///////////////////////////////////////////////////

// --------- IMPORTANT --------------------------------------------------------
// Before you start working on this assignment, set the following variable
// properly to indicate whether you want to use OpenGL 2.x with GLSL 1.1 or
// OpenGL 3.x+ with GLSL 1.3.
//
// Set g_Gl2Compatible = true to use GLSL 1.0 and g_Gl2Compatible = false to
// use GLSL 1.3. Make sure that your machine supports the version of GLSL you
// are using. In particular, on Mac OS X currently there is no way of using
// OpenGL 3.x with GLSL 1.3 when GLUT is used.
//
// If g_Gl2Compatible=true, shaders with -gl2 suffix will be loaded.
// If g_Gl2Compatible=false, shaders with -gl3 suffix will be loaded.
// To complete the assignment you only need to edit the shader files that get
// loaded
// ----------------------------------------------------------------------------
const bool g_Gl2Compatible = true;


static const float g_frustMinFov = 60.0;  // A minimal of 60 degree field of view
static float g_frustFovY = g_frustMinFov; // FOV in y direction (updated by updateFrustFovY)

static const float g_frustNear = -0.1;    // near plane
static const float g_frustFar = -50.0;    // far plane
static const float g_groundY = -2.0;      // y coordinate of the ground
static const float g_groundSize = 10.0;   // half the ground length

static int g_windowWidth = 512;
static int g_windowHeight = 512;
static bool g_mouseClickDown = false;    // is the mouse button pressed
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static int g_mouseClickX, g_mouseClickY; // coordinates for mouse click event
static int g_activeShader = 0;
// ========================================
// TODO: you can add global variables here
// ========================================
static int g_frame_number = 0;  //  indicate the current frame 0: sky frame, 1: cube 1 frame, 2: cube 2 frame  
static int g_object_number = 0;  //  indicate the current object 0: sky, 1: cube 1, 2: cube 2			                  
static int g_sky_world = 0;  //  0: sky-world frame, 1: sky-sky frame		

static double g_arcballscale = 1; // basic arcball radius scale
static double g_acrballScreenRadius = 1; // basic arcball screen radius	    
static int arcball_visible = 1;		

static bool g_pick = false;    // manipulate the pick mode	            //---------------change                                     

static const int PICKING_SHADER = 2; // index of the picking shader is g_shader 
static const int g_numShaders = 3; // 3 shaders instead of 2
static const char * const g_shaderFiles[g_numShaders][2] = 
{
	{"./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader"},
	{"./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader"},
	{"./shaders/basic-gl3.vshader", "./shaders/pick-gl3.fshader"}
};
static const char * const g_shaderFilesGl2[g_numShaders][2] = 
{
	{"./shaders/basic-gl2.vshader", "./shaders/diffuse-gl2.fshader"},
	{"./shaders/basic-gl2.vshader", "./shaders/solid-gl2.fshader"},
	{"./shaders/basic-gl2.vshader", "./shaders/pick-gl2.fshader"}		//---------------change
};
static vector<shared_ptr<ShaderState> > g_shaderStates; // our global shader states

// --------- Geometry

// Macro used to obtain relative offset of a field within a struct
#define FIELD_OFFSET(StructType, field) &(((StructType *)0)->field)

// A vertex with floating point position and normal
struct VertexPN 
{
	Cvec3f p, n;

	VertexPN() {}
	VertexPN(float x, float y, float z,
					 float nx, float ny, float nz)
		: p(x,y,z), n(nx, ny, nz)
	{}

	// Define copy constructor and assignment operator from GenericVertex so we can
	// use make* functions from geometrymaker.h
	VertexPN(const GenericVertex& v) 
	{
		*this = v;
	}

	VertexPN& operator = (const GenericVertex& v) 
	{
		p = v.pos;
		n = v.normal;
		return *this;
	}
};

struct Geometry 
{
	GlBufferObject vbo, ibo;
	int vboLen, iboLen;

	Geometry(VertexPN *vtx, unsigned short *idx, int vboLen, int iboLen) 
	{
		this->vboLen = vboLen;
		this->iboLen = iboLen;

		// Now create the VBO and IBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vboLen, vtx, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * iboLen, idx, GL_STATIC_DRAW);
	}

	void draw(const ShaderState& curSS) 
	{
		// Enable the attributes used by our shader
		safe_glEnableVertexAttribArray(curSS.h_aPosition);
		safe_glEnableVertexAttribArray(curSS.h_aNormal);

		// bind vbo
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		safe_glVertexAttribPointer(curSS.h_aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, p));
		safe_glVertexAttribPointer(curSS.h_aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, n));

		// bind ibo
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		// draw!
		glDrawElements(GL_TRIANGLES, iboLen, GL_UNSIGNED_SHORT, 0);

		// Disable the attributes used by our shader
		safe_glDisableVertexAttribArray(curSS.h_aPosition);
		safe_glDisableVertexAttribArray(curSS.h_aNormal);
	}
};

typedef SgGeometryShapeNode<Geometry> MyShapeNode;


// Vertex buffer and index buffer associated with the ground and cube geometry
static shared_ptr<Geometry> g_ground, g_cube, g_sphere;          // -----------change   

// --------- Scene

static shared_ptr<SgRootNode> g_world;
static shared_ptr<SgRbtNode> g_skyNode, g_groundNode, g_robot1Node, g_robot2Node;
static shared_ptr<SgRbtNode> g_currentPickedRbtNode, g_frameNode; // used later when you do picking 		//---------------change

static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);  // define two lights positions in world space

// ============================================
// TODO: add a second cube's 
// 1. transformation
// 2. color
// ============================================

// The skyRbt should be constant, so use g_temp_frame to store the 								
// add a cube 2 and its transformation
static RigTForm arcballRbt = RigTForm(Cvec3(0,0,0)); 	//--------------change
// add a new color to cube 2
static Cvec3f arcball_color = Cvec3f(0.2, 0.5, 0.8);   		//--------------change	

///////////////// END OF G L O B A L S //////////////////////////////////////////////////


static void initGround() 
{
	// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
	VertexPN vtx[4] = {
		VertexPN(-g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
		VertexPN(-g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
		VertexPN( g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
		VertexPN( g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
	};
	unsigned short idx[] = {0, 1, 2, 0, 2, 3};
	g_ground.reset(new Geometry(&vtx[0], &idx[0], 4, 6));
}

static void initCubes() 
{
	int ibLen, vbLen;
	getCubeVbIbLen(vbLen, ibLen);

	// Temporary storage for cube geometry
	vector<VertexPN> vtx(vbLen);
	vector<unsigned short> idx(ibLen);

	makeCube(1, vtx.begin(), idx.begin());
	g_cube.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

//initial the arcball sphere
static void initSphere() 
{
	int ibLen, vbLen;
	int slices = 30;      // the number of slices
	int stacks = 50;     // the number of stacks
	getSphereVbIbLen(slices, stacks, vbLen, ibLen);  // void getSphereVbIbLen(int slices, int stacks, int& vbLen, int& ibLen)

	// Temporary storage for cube geometry
	vector<VertexPN> vtx(vbLen);
	vector<unsigned short> idx(ibLen);

	makeSphere(g_acrballScreenRadius, slices, stacks, vtx.begin(), idx.begin());  //void makeSphere(float radius, int slices, int stacks, VtxOutIter vtxIter, IdxOutIter idxIter)
	g_sphere.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

// takes a projection matrix and send to the the shaders
static void sendProjectionMatrix(const ShaderState& curSS, const Matrix4& projMatrix) 
{
	GLfloat glmatrix[16];
	projMatrix.writeToColumnMajorMatrix(glmatrix); // send projection matrix
	safe_glUniformMatrix4fv(curSS.h_uProjMatrix, glmatrix);
}

// update g_frustFovY from g_frustMinFov, g_windowWidth, and g_windowHeight
static void updateFrustFovY() 
{
	if (g_windowWidth >= g_windowHeight)
		g_frustFovY = g_frustMinFov;
	else {
		const double RAD_PER_DEG = 0.5 * CS175_PI/180;
		g_frustFovY = atan2(sin(g_frustMinFov * RAD_PER_DEG) * g_windowHeight / g_windowWidth, cos(g_frustMinFov * RAD_PER_DEG)) / RAD_PER_DEG;
	}
}

static void maintainArcballScale()
{
	Matrix4 arcball = rigTFormToMatrix(arcballRbt);
	Cvec3 arc_center = Cvec3(arcball(0,3), arcball(1,3), arcball(2,3));
	Matrix4 eye = rigTFormToMatrix(g_frameNode->getRbt());
	Cvec3 eye_center = Cvec3(eye(0,3), eye(1,3), eye(2,3));
	Cvec3 temp = eye_center - arc_center;
	double length =  norm(temp);
	g_arcballscale = getScreenToEyeScale(-length, g_frustFovY, g_windowHeight) * 0.25 * min(g_windowWidth, g_windowHeight );
}

static Matrix4 makeProjectionMatrix() 
{
	return Matrix4::makeProjection(g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight), g_frustNear, g_frustFar);
}

static void drawStuff(const ShaderState& curSS, bool picking) 
{
	// short hand for current shader state
	//const ShaderState& curSS = *g_shaderStates[g_activeShader];

	// build & send proj. matrix to vshader
	const Matrix4 projmat = makeProjectionMatrix();
	sendProjectionMatrix(curSS, projmat);

	// use the skyRbt as the eyeRbt ! NOT HERE !!!!!!!!!

	// when draw the stuff, the eyeRbt should be the newest frame
	const RigTForm eyeRbt = g_frameNode->getRbt();      									                                       // -----------------change
	const RigTForm invEyeRbt = inv(eyeRbt);                                                                                       // -----------------change

	const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1)); // g_light1 position in eye coordinates
	const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1)); // g_light2 position in eye coordinates
	safe_glUniform3f(curSS.h_uLight, eyeLight1[0], eyeLight1[1], eyeLight1[2]);
	safe_glUniform3f(curSS.h_uLight2, eyeLight2[0], eyeLight2[1], eyeLight2[2]);

	if (!picking) 
	{									//---------------change
		Drawer drawer(invEyeRbt, curSS);
		g_world->accept(drawer);

	 	// draw arcball as part of asst3
	 	// draw sphere
	 	maintainArcballScale();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw wireframe
		const Cvec3 scale = Cvec3(g_arcballscale);
		Matrix4 scale_matrix = Matrix4::makeScale(scale);   // scale matrix
		Matrix4 MVM = rigTFormToMatrix(invEyeRbt * arcballRbt);             // compute MVM, taking into account the dynamic radius.
		//if(g_mouseClickDown != (g_mouseLClickButton && g_mouseRClickButton) || g_mouseMClickButton)
		MVM = MVM * scale_matrix;
		Matrix4 NMVM = normalMatrix(MVM);            // send in MVM and NMVM
		sendModelViewNormalMatrix(curSS, MVM, NMVM);
		safe_glUniform3f(curSS.h_uColor, arcball_color[0], arcball_color[1], arcball_color[2]);          // send in uColor
		if(arcball_visible == 1)  // arcball is visible
		{
			if(g_sky_world%2 == 0 )  // when sky-world frame and pick a part then draw the sphere
		 	{
		 		if(g_frameNode == g_skyNode )
		 		{
		 			g_sphere->draw(curSS);
		 		}
		 		else
		 		{
		 			 if(g_currentPickedRbtNode != g_skyNode && g_currentPickedRbtNode != g_groundNode && g_currentPickedRbtNode != nullptr)
		 				g_sphere->draw(curSS);
		 		}
		 	}
		 	if(g_sky_world%2 == 1) // when sky-sky frame
		 	{
		 		if(g_currentPickedRbtNode != nullptr && g_currentPickedRbtNode != g_skyNode && g_currentPickedRbtNode != g_robot1Node && g_currentPickedRbtNode != g_robot2Node)
		 		{
		 			g_sphere->draw(curSS);    //manipulating a cube, and it is not with respect to itself
		 		}
		 	}
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // draw filled again						//---------------change
	}
	else 
	{
		Picker picker(invEyeRbt, curSS);
		g_world->accept(picker);
		glFlush();
		//cout<<"g_pick is "<<g_pick<<endl;   //test
		g_currentPickedRbtNode = picker.getRbtNodeAtXY(g_mouseClickX, g_mouseClickY);

		if (g_currentPickedRbtNode == g_groundNode)
			g_currentPickedRbtNode = shared_ptr<SgRbtNode>();   // set to NULL
		//cout<<"g_currentPickedRbtNode is "<<g_currentPickedRbtNode<<endl;
		if(g_currentPickedRbtNode == nullptr)
		{
			cout<<"Pick anything? NO! "<<endl;     //test
			arcballRbt = g_world->getRbt();  //when pick nothing , the arcball will at the center of the world
		}
		else
		{
			cout<<"Pick anything? YES! "<<endl;     //test
			arcballRbt = getPathAccumRbt(g_world, g_currentPickedRbtNode);  //when pick any part , the arcball will at this part instantly
		}
	}         																						//---------------change
}

static void pick() 																				//---------------change
{
	// We need to set the clear color to black, for pick rendering.
	// so let's save the clear color
	GLdouble clearColor[4];
	glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);

	glClearColor(0, 0, 0, 0);

	// using PICKING_SHADER as the shader
	glUseProgram(g_shaderStates[PICKING_SHADER]->program);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawStuff(*g_shaderStates[PICKING_SHADER], true);

	// Uncomment below and comment out the glutPostRedisplay in mouse(...) call back
	// to see result of the pick rendering pass
	// glutSwapBuffers();

	//Now set back the clear color
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

	checkGlErrors();
}

static void display() 
{
	glUseProgram(g_shaderStates[g_activeShader]->program);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // clear framebuffer color&depth

	drawStuff(*g_shaderStates[g_activeShader], false);							//---------------change

	glutSwapBuffers();                                    // show the back buffer (where we rendered stuff)

	checkGlErrors();
}

static void reshape(const int w, const int h) 
{
	g_windowWidth = w;
	g_windowHeight = h;
	glViewport(0, 0, w, h);
	cerr << "Size of window is now " << w << "x" << h << endl;
	updateFrustFovY();
	maintainArcballScale();
	glutPostRedisplay();
}

static Cvec3 GetVector(int l, int b, const Cvec2& p )
{
	Cvec3 v = Cvec3(0);
	if(l > b) // point 1 is out of the ball
	{
		v = normalize(Cvec3(p,0.0));   
	}
	else // point 1 is on the ball
	{
		double temp = sqrt(b * b - l * l);
		v = normalize(Cvec3(p, temp));
	}
	return v;
}

static void motion(const int x, const int y) 
{
	//const double x_new = x;
	//const double y_new = g_windowHeight - y - 1;
	const double dx = x - g_mouseClickX;
	const double dy = g_windowHeight - y - 1 - g_mouseClickY;

	int object_number = g_object_number % 3;                                           
	int frame_number = g_frame_number % 3;                                                                   
	int sky_world = g_sky_world % 2;  

	RigTForm m;
	if (g_mouseLClickButton && !g_mouseRClickButton) 
	{ // left button down?
		if(sky_world == 1 || (sky_world == 0 && frame_number != 0)) 
		{
			 m= RigTForm(Quat::makeXRotation(-dy) * Quat::makeYRotation(dx));
		}
		if(sky_world == 0 && frame_number == 0)
		{
			Matrix4 MVM = rigTFormToMatrix(inv(g_frameNode->getRbt()) * arcballRbt);         // eye-coordinates of the arcball		//---------------change
			 Cvec3 center =  Cvec3(MVM(0,3), MVM(1,3), MVM(2,3));   // eye-coordinates of the center

			 // screen-coordinate of center
			 Cvec2 cen_scr_cor = getScreenSpaceCoord(center, makeProjectionMatrix(), g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight); 
			 Cvec3 bt1 = Cvec3(MVM(0,3) + g_acrballScreenRadius * g_arcballscale , MVM(1,3), MVM(2,3));   // eye-coordinates of the center
			 Cvec2 bt2 = getScreenSpaceCoord(bt1, makeProjectionMatrix(), g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight); 
			 double b = bt2[0] - cen_scr_cor[0]; // the screen-coordinate of radiu of sphere 
			Cvec2 point_1 = Cvec2(g_mouseClickX, g_mouseClickY) - cen_scr_cor;
			Cvec2 point_2 = Cvec2(x, g_windowHeight - y - 1) - cen_scr_cor;
			double l = norm(point_1);
			Cvec3 vector0, vector1;
			vector0 = GetVector(l,b,point_1);
			l = norm(point_2);
			vector1 = GetVector(l,b,point_2);
			m = RigTForm(Quat(0, vector1) * Quat(0, -vector0)); 
		}
		//g_arcballscale = 1.3;  //test
		}
	else if (g_mouseRClickButton && !g_mouseLClickButton) // right button down?
	{ 
		m = RigTForm(Cvec3(dx, dy, 0) * 0.01);
	}
	else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) // middle or (left and right) button down?
	{  
		m = RigTForm(Cvec3(0, 0, -dy) * 0.01);
	}

	if (g_mouseClickDown) 
	{
		//g_objectRbt[g_object_number%3] *= m; // Simply right-multiply is WRONG                                                                                
			if(frame_number == 0) // frame is sky                                                                            
			{
					if(g_currentPickedRbtNode == g_skyNode || g_currentPickedRbtNode == nullptr) // object is sky                                                                 
					{
							//cout<<"test 1"<<endl;
							if(sky_world == 1) // frame is sky-sky                                                       		//---------------change
							{
									RigTForm pure = g_skyNode->getRbt();
									RigTForm A = transFact(pure) * linFact(pure);
									g_frameNode->setRbt(A * inv(m) * inv(A) * g_frameNode->getRbt()) ;
							}
							else  // frame is sky-world                                                                          		//---------------change
							{
									RigTForm pure = g_world->getRbt();
									RigTForm A = transFact(pure) * linFact(pure);
									g_frameNode->setRbt(A * inv(m) * inv(A) * g_frameNode->getRbt());
									arcballRbt = pure;
							}
					}
					else 		//object is a robot 															//---------------change
					{
							RigTForm mix1 = getPathAccumRbt(g_world, g_currentPickedRbtNode);
							RigTForm mix2 = g_frameNode->getRbt();

							//calculate A
							RigTForm A = transFact(mix1) * linFact(mix2);
							//calculate As
							RigTForm As = inv(getPathAccumRbt(g_world, g_currentPickedRbtNode, 1)) * A;

							g_currentPickedRbtNode->setRbt(As * m * inv(As) * g_currentPickedRbtNode->getRbt());
							arcballRbt = getPathAccumRbt(g_world, g_currentPickedRbtNode); 
							//cout<<"test 2"<<endl;
					}                                                                                  
			}
			else // frame is a robot                                                                                                   
       		{
       			if(g_currentPickedRbtNode == g_frameNode) // manipulate the torso of robot
       			{
       				RigTForm mix1 = getPathAccumRbt(g_world, g_currentPickedRbtNode);
					RigTForm mix2 = g_frameNode->getRbt();
					//calculate A
					RigTForm A = transFact(mix1) * linFact(mix2);
					g_currentPickedRbtNode->setRbt(A * inv(m) * inv(A) * g_currentPickedRbtNode->getRbt());
					//cout<<"test 2"<<endl; 
       			}
       			else  if(g_currentPickedRbtNode!=g_skyNode && g_currentPickedRbtNode != nullptr)// manipulate other part of the other robot
       			{
       				RigTForm mix1 = getPathAccumRbt(g_world, g_currentPickedRbtNode);
					RigTForm mix2 = g_frameNode->getRbt();
					//calculate A
					RigTForm A = transFact(mix1) * linFact(mix2);
					//calculate As
					RigTForm As = inv(getPathAccumRbt(g_world, g_currentPickedRbtNode, 1)) * A;

					g_currentPickedRbtNode->setRbt(As * m * inv(As) * g_currentPickedRbtNode->getRbt());
					arcballRbt = getPathAccumRbt(g_world, g_currentPickedRbtNode);	
       			}                                                   
       		} 
			glutPostRedisplay(); // we always redraw if we changed the scene
	}

	g_mouseClickX = x;
	g_mouseClickY = g_windowHeight - y - 1;
}

static void mouse(const int button, const int state, const int x, const int y) 
{
	g_mouseClickX = x;
	g_mouseClickY = g_windowHeight - y - 1;  // conversion from GLUT window-coordinate-system to OpenGL window-coordinate-system

	g_mouseLClickButton |= (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
	g_mouseRClickButton |= (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
	g_mouseMClickButton |= (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN);

	g_mouseLClickButton &= !(button == GLUT_LEFT_BUTTON && state == GLUT_UP);
	g_mouseRClickButton &= !(button == GLUT_RIGHT_BUTTON && state == GLUT_UP);
	g_mouseMClickButton &= !(button == GLUT_MIDDLE_BUTTON && state == GLUT_UP);

	g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;
	if(g_pick == true && g_mouseLClickButton && !g_mouseRClickButton) 		//---------------change
	{
		pick();
		g_pick = false;
		cout<<"PICKING: OFF"<<endl;
	}
	glutPostRedisplay();
}
static void reset();

static void keyboard(const unsigned char key, const int x, const int y) 
{
	switch (key) 
	{
	case 27:
		exit(0);                                  // ESC
	case 'h':
		cout << " ============== H E L P ==============\n\n"
		<< "h\t\thelp menu\n"
		<< "s\t\tsave screenshot\n"
		<< "f\t\tToggle flat shading on/off.\n"
		<< "o\t\tCycle object to edit\n"
		<< "v\t\tCycle view\n"
		<<"m\t\tswitch between world-sky frame and sky-sky frame\n"	
		<< "drag left mouse to rotate\n" << endl;
		break;
	case 's':
		glFlush();
		writePpmScreenshot(g_windowWidth, g_windowHeight, "out.ppm");
		break;
	case 'f':
		g_activeShader ^= 1;
		break;
	// ============================================================
	// TODO: add the following functionality for 
	//       keybaord inputs
	// - 'v': cycle through the 3 views
	// - 'o': cycle through the 3 objects being manipulated
	// - 'm': switch between "world-sky" frame and "sky-sky" frame
	// - 'r': reset the scene
	// ============================================================
		case 'v':
			g_frame_number ++;
			switch(g_frame_number%3)
			{
				case 0: g_frameNode = g_skyNode;
					 cout<<"Current frame is sky camera frame."<<endl;		
					 break;
				case 1: g_frameNode = g_robot1Node;
					 cout<<"Current frame is frame of robot 1."<<endl;		
					 break;
				case 2: g_frameNode = g_robot2Node;
					 cout<<"Current frame is frame of robot 2."<<endl;		
					 break;
			}
		break;
		// change---delete 'o' case
		case 'm':
			g_sky_world++;
			switch(g_sky_world%2)
			{
				case 0: cout<<"Current is world-sky  frame."<<endl;			
					 break;
				case 1: cout<<"Current is sky-sky frame."<<endl;			
					 break;
			}
		break;
		case 'r':
			reset();										                                       
		break;
		case 'a':
			 arcball_visible = (arcball_visible+1) % 2;
		break;
		case 'p': 		//---------------change
			 g_pick = true;
			 cout<<"PICKING:ON"<<endl;
		break;
	}
	glutPostRedisplay();
}

static void initGlutState(int argc, char * argv[]) 
{
	glutInit(&argc, argv);                                  // initialize Glut based on cmd-line args
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);  //  RGBA pixel channels and double buffering
	glutInitWindowSize(g_windowWidth, g_windowHeight);      // create a window
	glutCreateWindow("yl3651_Assignment 4");                       // title the window

	glutDisplayFunc(display);                               // display rendering callback
	glutReshapeFunc(reshape);                               // window reshape callback
	glutMotionFunc(motion);                                 // mouse movement callback
	glutMouseFunc(mouse);                                   // mouse click callback
	glutKeyboardFunc(keyboard);
}

static void initGLState() 
{
	glClearColor(128./255., 200./255., 255./255., 0.); // clear not only the color in the image but also the "Z-buffer"
	glClearDepth(0.);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);      // enable back face
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glReadBuffer(GL_BACK);
	if (!g_Gl2Compatible)
		glEnable(GL_FRAMEBUFFER_SRGB);
}

static void initShaders() 
{
	g_shaderStates.resize(g_numShaders);
	for (int i = 0; i < g_numShaders; ++i) {
		if (g_Gl2Compatible)
			g_shaderStates[i].reset(new ShaderState(g_shaderFilesGl2[i][0], g_shaderFilesGl2[i][1]));
		else
			g_shaderStates[i].reset(new ShaderState(g_shaderFiles[i][0], g_shaderFiles[i][1]));
	}
}

static void initGeometry() 
{
	initGround();
	initCubes();
	initSphere();                              //-------------change
}

static void constructRobot(shared_ptr<SgTransformNode> base, const Cvec3& color)  		//---------------change
{

	const double ARM_LEN = 0.7,
							 ARM_THICK = 0.25,
							 TORSO_LEN = 1.5,
							 TORSO_THICK = 0.25,
							 TORSO_WIDTH = 1,
							 HEAD_RADIUS = 0.5;
	const int 
		NUM_JOINTS = 14,
		NUM_SHAPES = 14;

	struct JointDesc 
	{
		int parent;
		float x, y, z;
	};

	JointDesc jointDesc[NUM_JOINTS] = 
	{
		// TORSO
		{-1}, 											// torso jointDesc[0]

		// RIGHT ARM
		{0, TORSO_WIDTH/2, TORSO_LEN/2-ARM_THICK/2, 0},   // upper right arm jointDesc[1]
		{1, ARM_THICK, 0, 0},							// lower right arm jointDesc[2]
		{2, ARM_LEN, 0, 0},							// lower hand jointDesc[3]

		//  LEFT ARM
		{0, -TORSO_WIDTH/2, TORSO_LEN/2-ARM_THICK/2, 0},  // upper left arm jointDesc[4]
		{4, -ARM_LEN, 0, 0},							// lower left arm jointDesc[5]
		{5, -ARM_LEN, 0, 0},							// lower hand jointDesc[6]

		// RIGHT LEG
		{0, (TORSO_WIDTH/8)*3, -TORSO_LEN/2, 0}, // upper right leg jointDesc[7]
		{7, 0, -ARM_LEN, 0},								// lower right leg jointDesc[8]
		{8, 0, -ARM_LEN, 0},							// lower foot jointDesc[9]

		//  LEFT LEG
		{0, -(TORSO_WIDTH/8)*3, -TORSO_LEN/2, 0}, // upper left leg jointDesc[10]
		{10, 0, -ARM_LEN, 0},								// lower left leg jointDesc[11]
		{11, 0, -ARM_LEN, 0},							// lower foot jointDesc[12]

		// HEAD
		{0, 0, TORSO_LEN/2, 0},						//  smart head jointDesc[13]
	};

	struct ShapeDesc 
	{
		int parentJointId;
		float x, y, z, sx, sy, sz;
		shared_ptr<Geometry> geometry;
	};

	ShapeDesc shapeDesc[NUM_SHAPES] = 
	{
		// TORSO
		{0, 0,         0, 0, TORSO_WIDTH, TORSO_LEN, TORSO_THICK, g_cube }, // torso

		//RIGHT ARM
		{1, ARM_THICK/2, 0, 0, ARM_THICK, ARM_THICK, ARM_THICK, g_cube}, // upper right arm
		{2, ARM_LEN/2, 0, 0, ARM_LEN/2, ARM_THICK/2, ARM_THICK/2, g_sphere},// lower right arm
		{3, ARM_LEN/8, 0, 0, ARM_THICK/2, ARM_THICK/2, ARM_THICK/2, g_sphere},

		// LEFT ARM
		{4, -ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube},
		{5, -ARM_LEN/2, 0, 0, ARM_LEN/2, ARM_THICK/2, ARM_THICK/2, g_sphere},
		{6, -ARM_LEN/8, 0, 0, ARM_THICK/2, ARM_THICK/2, ARM_THICK/2, g_sphere},

		// RIGHT LEG
		{7, 0, -ARM_LEN/2, 0, ARM_THICK, ARM_LEN, ARM_THICK, g_cube},
		{8, 0, -ARM_LEN/2, 0, ARM_THICK /2, ARM_LEN /2, ARM_THICK /2, g_sphere},
		{9, 0, -ARM_THICK/2, 0, ARM_THICK/2, ARM_THICK/2, ARM_THICK/2, g_sphere},

		//LEFT LEG
		{10, 0, -ARM_LEN/2, 0, ARM_THICK, ARM_LEN, ARM_THICK, g_cube},
		{11, 0, -ARM_LEN/2, 0, ARM_THICK /2, ARM_LEN /2, ARM_THICK /2, g_sphere},
		{12, 0, -ARM_THICK/2, 0, ARM_THICK /2, ARM_THICK /2, ARM_THICK /2, g_sphere},

		//HEAD
		{13, 0, HEAD_RADIUS, 0, HEAD_RADIUS, HEAD_RADIUS, HEAD_RADIUS, g_sphere},
	};

	shared_ptr<SgTransformNode> jointNodes[NUM_JOINTS]; // jointNodes is a SgTransformNode array

	for (int i = 0; i < NUM_JOINTS; ++i) 
	{
		if (jointDesc[i].parent == -1)
			jointNodes[i] = base;
		else 
		{
			jointNodes[i].reset(new SgRbtNode(RigTForm(Cvec3(jointDesc[i].x, jointDesc[i].y, jointDesc[i].z)))); //use to move frame
			jointNodes[jointDesc[i].parent]->addChild(jointNodes[i]);
		}
	}
	for (int i = 0; i < NUM_SHAPES; ++i) 
	{
		shared_ptr<MyShapeNode> shape(
			new MyShapeNode(shapeDesc[i].geometry,
											color,
											Cvec3(shapeDesc[i].x, shapeDesc[i].y, shapeDesc[i].z),
											Cvec3(0, 0, 0),
											Cvec3(shapeDesc[i].sx, shapeDesc[i].sy, shapeDesc[i].sz)));
		jointNodes[shapeDesc[i].parentJointId]->addChild(shape);
	}
}

static void initScene() 
{
	g_world.reset(new SgRootNode());
	g_skyNode.reset(new SgRbtNode(RigTForm(Cvec3(0.0, 0.25, 4.0))));
	g_currentPickedRbtNode=g_frameNode = g_skyNode;     //-------------------change


	g_groundNode.reset(new SgRbtNode());
	g_groundNode->addChild(shared_ptr<MyShapeNode>(new MyShapeNode(g_ground, Cvec3(0.1, 0.95, 0.1))));

	g_robot1Node.reset(new SgRbtNode(RigTForm(Cvec3(-2, 1, 0))));
	g_robot2Node.reset(new SgRbtNode(RigTForm(Cvec3(2, 1, 0))));

	constructRobot(g_robot1Node, Cvec3(1, 0, 0)); // a Red robot
	constructRobot(g_robot2Node, Cvec3(0, 0, 1)); // a Blue robot

	g_world->addChild(g_skyNode);
	g_world->addChild(g_groundNode);
	g_world->addChild(g_robot1Node);
	g_world->addChild(g_robot2Node);
}

static void reset() 		//---------------change
{
	// =========================================================
	// TODO:
	// - reset g_skyRbt and g_objectRbt to their default values
	// - reset the views and manipulation mode to default
	// - reset sky camera mode to use the "world-sky" frame
	// =========================================================    
	initScene();
	g_sky_world = 0;  			
	g_arcballscale = 1;
	g_acrballScreenRadius =1;	
      arcball_visible = 1;  
	arcballRbt = g_world->getRbt();                         

	cout << "reset objects and modes to defaults" << endl;
}

int main(int argc, char * argv[]) 
{
	try {
		initGlutState(argc,argv);

		glewInit(); // load the OpenGL extensions

		cout << (g_Gl2Compatible ? "Will use OpenGL 2.x / GLSL 1.0" : "Will use OpenGL 3.x / GLSL 1.3") << endl;
		if ((!g_Gl2Compatible) && !GLEW_VERSION_3_0)
			throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.3");
		else if (g_Gl2Compatible && !GLEW_VERSION_2_0)
			throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.0");

		initGLState();
		initShaders();
		initGeometry();
		initScene();

		glutMainLoop();
		return 0;
	}
	catch (const runtime_error& e) {
		cout << "Exception caught: " << e.what() << endl;
		return -1;
	}
}
