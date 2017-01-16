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
#include <list>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <iostream>
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
#include "sgutils.h"
#include "geometry.h"
#include "mesh.h"

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

static bool g_pick = false;    // manipulate the pick mode	                                                 

static const int PICKING_SHADER = 2; // index of the picking shader is g_shader 
static bool isSmoothShading = true;

// --------- Materials
// This should replace all the contents in the Shaders section, e.g., g_numShaders, g_shaderFiles, and so on
static shared_ptr<Material> g_redDiffuseMat,
                            g_blueDiffuseMat,
                            g_bumpFloorMat,
                            g_arcballMat,
                            g_pickingMat,
                            g_lightMat,
                            g_meshcubeMat;

shared_ptr<Material> g_overridingMaterial;

static list<std::vector<RigTForm> > key_frame; 								//----------change
static list<std::vector<RigTForm> >::iterator current_key_frame = key_frame.begin(); 				// remember to initiallize the interator     //----------change
static int key_frame_index = 0; // current number od the current key frame //----------change
static int g_msBetweenKeyFrames = 2000; // 2 seconds between keyframes  //----------change
static int g_animateFramesPerSecond = 60; // frames to render per second during animation playback  //----------change
static int bubbling_animate_speed = 5;
static int flag = 0;
static int bubbling_animation_flag = 0;
static float ti = 0;
static float t_1;

// --------- Geometry
typedef SgGeometryShapeNode MyShapeNode;

// Vertex buffer and index buffer associated with the ground and cube geometry
static shared_ptr<Geometry> g_ground, g_cube, g_sphere;
static shared_ptr<SimpleGeometryPN>  g_meshCube;

// --------- Scene

static shared_ptr<SgRootNode> g_world;
static shared_ptr<SgRbtNode> g_skyNode, g_groundNode, g_robot1Node, g_robot2Node, g_light1Node, g_light2Node, g_meshCubeNode;
static shared_ptr<SgRbtNode> g_currentPickedRbtNode, g_frameNode; // used later when you do picking 	
Mesh mesh;
Mesh mesh_temp;
static int subdivide_time = 0;
static bool subdevide_decrease_flag = false;

static const Cvec3 g_light1(2.0, 5.0, 10.0), g_light2(-2, 5.0, -10.0);  // define two lights positions in world space

// ============================================
// TODO: add a second cube's 
// 1. transformation
// 2. color
// ============================================

// The skyRbt should be constant, so use g_temp_frame to store the 								
// add a cube 2 and its transformation
static RigTForm arcballRbt = RigTForm(Cvec3(0,0,0)); 	
// add a new color to cube 2
static Cvec3f arcball_color = Cvec3f(0.2, 0.5, 0.8);   			

///////////////// END OF G L O B A L S //////////////////////////////////////////////////


static void initGround() {
  int ibLen, vbLen;
  getPlaneVbIbLen(vbLen, ibLen);

  // Temporary storage for cube Geometry
  vector<VertexPNTBX> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makePlane(g_groundSize*2, vtx.begin(), idx.begin());
  g_ground.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initCubes() {
  int ibLen, vbLen;
  getCubeVbIbLen(vbLen, ibLen);

  // Temporary storage for cube Geometry
  vector<VertexPNTBX> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makeCube(1, vtx.begin(), idx.begin());
  g_cube.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initMeshCube()
{	
	mesh.load("cube.mesh");
	g_meshCube.reset(new SimpleGeometryPN());
	g_meshCube->new_upload(mesh, isSmoothShading);
}

//initial the arcball sphere
static void initSphere() {
  int ibLen, vbLen;
  getSphereVbIbLen(20, 10, vbLen, ibLen);

  // Temporary storage for sphere Geometry
  vector<VertexPNTBX> vtx(vbLen);
  vector<unsigned short> idx(ibLen);
  makeSphere(1, 20, 10, vtx.begin(), idx.begin());
  g_sphere.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vtx.size(), idx.size()));
}

// takes a projection matrix and send to the the shaders
inline void sendProjectionMatrix(Uniforms& uniforms, const Matrix4& projMatrix) {
  uniforms.put("uProjMatrix", projMatrix);
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
	Cvec3 arc_center = arcballRbt.getTranslation();
	Cvec3 eye_center = (g_frameNode->getRbt()).getTranslation();
	Cvec3 temp = eye_center - arc_center;
	double length =  norm(temp);
	g_arcballscale = getScreenToEyeScale(-length, g_frustFovY, g_windowHeight) * 0.25 * min(g_windowWidth, g_windowHeight );
}

static Matrix4 makeProjectionMatrix() 
{
	return Matrix4::makeProjection(g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight), g_frustNear, g_frustFar);
}

static void drawStuff(bool picking)
{

	// Declare an empty uniforms
  	Uniforms uniforms;

	// build & send proj. matrix to vshader
	const Matrix4 projmat = makeProjectionMatrix();
	sendProjectionMatrix(uniforms, projmat);

	// use the skyRbt as the eyeRbt ! NOT HERE !!!!!!!!!

	// when draw the stuff, the eyeRbt should be the newest frame
	const RigTForm eyeRbt = g_frameNode->getRbt();      									                                   
	const RigTForm invEyeRbt = inv(eyeRbt);                                                                                   

	const Cvec3 light1 = getPathAccumRbt(g_world, g_light1Node).getTranslation();
	const Cvec3 light2 = getPathAccumRbt(g_world, g_light2Node).getTranslation();

	uniforms.put("uLight", Cvec3(invEyeRbt * Cvec4(light1, 1)));
	uniforms.put("uLight2", Cvec3(invEyeRbt * Cvec4(light2, 1)));

	if (!picking) 
	{								
		Drawer drawer(invEyeRbt, uniforms);
		g_world->accept(drawer);

	 	// draw arcball as part of asst3
	 	// draw sphere
	 	maintainArcballScale();
		const Cvec3 scale = Cvec3(g_arcballscale);
		Matrix4 scale_matrix = Matrix4::makeScale(scale);   // scale matrix
		Matrix4 MVM = rigTFormToMatrix(invEyeRbt * arcballRbt);             // compute MVM, taking into account the dynamic radius.
		//if(g_mouseClickDown != (g_mouseLClickButton && g_mouseRClickButton) || g_mouseMClickButton)
		MVM = MVM * scale_matrix;
		Matrix4 NMVM = normalMatrix(MVM);            // send in MVM and NMVM
		sendModelViewNormalMatrix(uniforms, MVM, normalMatrix(MVM));
		//safe_glUniform3f(uniforms.h_uColor, arcball_color[0], arcball_color[1], arcball_color[2]);          // send in uColor
		if(arcball_visible == 1)  // arcball is visible
		{
			if(g_sky_world%2 == 0 )  // when sky-world frame and pick a part then draw the sphere
		 	{
		 		if(g_frameNode == g_skyNode )
		 		{
		 			g_arcballMat->draw(*g_sphere, uniforms);
		 		}
		 		else
		 		{
		 			 if(g_currentPickedRbtNode != g_skyNode && g_currentPickedRbtNode != g_groundNode && g_currentPickedRbtNode != nullptr)
		 				g_arcballMat->draw(*g_sphere, uniforms);
		 		}
		 	}
		 	if(g_sky_world%2 == 1) // when sky-sky frame
		 	{
		 		if(g_currentPickedRbtNode != nullptr && g_currentPickedRbtNode != g_skyNode && g_currentPickedRbtNode != g_robot1Node && g_currentPickedRbtNode != g_robot2Node)
		 		{
		 			g_arcballMat->draw(*g_sphere, uniforms);    //manipulating a cube, and it is not with respect to itself
		 		}
		 	}
		}					
	}
	else 
	{
		Picker picker(invEyeRbt, uniforms);

		// set overiding material to our picking material
    		g_overridingMaterial = g_pickingMat;
    		
		g_world->accept(picker);

		// unset the overriding material
    		g_overridingMaterial.reset();

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
	}         																						
}

static void pick() {
  // We need to set the clear color to black, for pick rendering.
  // so let's save the clear color
  GLdouble clearColor[4];
  glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // No more glUseProgram
  drawStuff(true); // no more curSS

  // Uncomment below and comment out the glutPostRedisplay in mouse(...) call back
  // to see result of the pick rendering pass
  // glutSwapBuffers();

  //Now set back the clear color
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

  checkGlErrors();
}

static void display() {
  // No more glUseProgram
    
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawStuff(false); // no more curSS

  glutSwapBuffers();

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

static Cvec3 GetVector(double x, double y, int r, const Cvec2& p )
{
	Cvec3 v = Cvec3(0);
	double l = norm(Cvec2(x - p(0), y - p(1)));
	if(l > r) // point 1 is out of the ball
	{
		v = normalize(Cvec3(x - p(0), y - p(1),0.0));
	}
	else // point 1 is on the ball
	{
		double temp = sqrt(r * r - l * l);
		v = normalize(Cvec3(x - p(0), y - p(1), temp));
	}
	return v;
}

static RigTForm get_arcball_rtf(const int x, const int y)
{
	RigTForm M = inv(g_frameNode->getRbt()) * arcballRbt;         // eye-coordinates of the arcball
	Cvec3 center =  M.getTranslation();   // eye-coordinates of the center
	Cvec2 cen_scr_cor = getScreenSpaceCoord(center, makeProjectionMatrix(), g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight);

	Cvec3 bt1 = Cvec3(center(0) + g_acrballScreenRadius * g_arcballscale , center(1), center(2));   // eye-coordinates of the center
	Cvec2 bt2 = getScreenSpaceCoord(bt1, makeProjectionMatrix(), g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight); 
	double radiu = bt2[0] - cen_scr_cor[0]; // the screen-coordinate of radiu of sphere 

	Cvec3 vector0 = GetVector(g_mouseClickX, g_mouseClickY, radiu, cen_scr_cor);
	Cvec3 vector1 = GetVector(x, g_windowHeight - y - 1, radiu, cen_scr_cor);

	RigTForm m = RigTForm(Quat(0, vector1) * Quat(0, -vector0)); 
	return m;
}

static void motion(const int x, const int y) 
{
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
			m= get_arcball_rtf(x,y);
		}
		//g_arcballscale = 1.3;  //test
		}
	else if (g_mouseRClickButton && !g_mouseLClickButton) // right button down?
	{ 
		if(g_currentPickedRbtNode == g_light1Node || g_currentPickedRbtNode == g_light2Node)
		{
			m = RigTForm(Cvec3(dx, dy, 0) * 0.035);
		}
		else
		{
			m = RigTForm(Cvec3(dx, dy, 0) * 0.01);
		}
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
							if(sky_world == 1) // frame is sky-sky                                                       		
							{
									RigTForm pure = g_skyNode->getRbt();
									RigTForm A = transFact(pure) * linFact(pure);
									g_frameNode->setRbt(A * inv(m) * inv(A) * g_frameNode->getRbt()) ;
							}
							else  // frame is sky-world                                                                          		
							{
									RigTForm pure = g_world->getRbt();
									RigTForm A = transFact(pure) * linFact(pure);
									g_frameNode->setRbt(A * inv(m) * inv(A) * g_frameNode->getRbt());
									arcballRbt = pure;
							}
					}
					else 		//object is a robot 															
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
			glutPostRedisplay();
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
	if(g_pick == true && g_mouseLClickButton && !g_mouseRClickButton) 		
	{
		pick();
		g_pick = false;
		cout<<"PICKING: OFF"<<endl;
	}
	glutPostRedisplay();
}
static void reset();

static void Copy_current_key_frame_to_scene_graph()
{
	if(flag == 0)
	{
		if(!key_frame.empty())   // current key frame is defined
		{
			vector<shared_ptr<SgRbtNode> > node_point_vector;   // temp vector to store scene rbt pointers
			vector<shared_ptr<SgRbtNode> >::iterator pointer = node_point_vector.begin(); // interator of node_point_vector
			vector<RigTForm>::iterator rbt = current_key_frame->begin();  // interator of update_current_key_frame

			dumpSgRbtNodes(g_world, node_point_vector); // get scene rbt pointer
			for( pointer =node_point_vector.begin();  pointer != node_point_vector.end(); ++pointer, ++rbt)
			{
				(*pointer)->setRbt(*rbt); // copy current_key_frame to scene graph
			}
			cout<<"Copy key frame "<< key_frame_index-1 <<" to scene graph"<<endl;
			glutPostRedisplay();
		}
		else // current key frame is not defined
		{
			cout<<"Current key frame is not defined yet!"<<endl;
		}
	}
	else
	{
		cout<<"Can not copy the key frames while animating"<<endl;
	}
}
 
static void new_a_key_frame();

static void update_current_key_frame()
{
	if(flag == 0)
	{
		if(!key_frame.empty())    // current key frame is defined
		{
			vector<shared_ptr<SgRbtNode> > node_point_vector;   // temp vector to store scene rbt pointers
			vector<shared_ptr<SgRbtNode> >::iterator pointer = node_point_vector.begin(); // interator of node_point_vector
			vector<RigTForm>::iterator rbt = current_key_frame->begin();  // interator of current_key_frame

			dumpSgRbtNodes(g_world, node_point_vector); // update current key frame
			for( pointer =node_point_vector.begin();  pointer != node_point_vector.end(); ++pointer, ++rbt)
			{
				(*rbt) = (*pointer)->getRbt(); // copy scene graph to current_key_frame
			}
			cout<<"update key frame "<< key_frame_index-1 <<" to scene graph"<<endl;
		}
		else // current key frame is not defined
		{
			new_a_key_frame();
			cout<<"Current key frame is not defined yet! Create a new key frame!"<<endl;
		}
	}
	else
	{
		cout<<"Can not update the key frames while animating"<<endl;
	}
}

static void advance_to_next_key_frame()
{
	if(flag == 0)
	{
		if(!key_frame.empty())
		{
			list<std::vector<RigTForm> >::iterator temp = current_key_frame;
			++temp;
			if(temp != key_frame.end())
			{
				++current_key_frame;
				++key_frame_index;
				Copy_current_key_frame_to_scene_graph();
			}
			else
			{
				cout<<"Current key frame is the last key frame"<<endl;
				//cout<<"a key_frame_index is "<<key_frame_index<<endl;    //test
			}
		}
		else
		{
			cout<<"Current key frame is not defined yet!"<<endl;
		}
	}
	else
	{
		cout<<"Can not advance to the next key frames while animating"<<endl;
	}
}

static void retreat_to_previous_key_frame()
{
	if(flag == 0)
	{
		if(!key_frame.empty())
		{
			if( key_frame_index-1 >= 0 )
			{
				--current_key_frame;
				--key_frame_index;
				Copy_current_key_frame_to_scene_graph();
			}
			else
			{
				cout<<"There is no previous key frame"<<endl;
			}
		}
		else
		{
			cout<<"Current key frame is not defined yet!"<<endl;
		}
	}
	else
	{
		cout<<"Can not retreat to the previous key frames while animating"<<endl;
	}
}

void delete_current_key_frame()
{
	if(flag == 0)
	{
		if(!key_frame.empty())  // current key frame is defined
		{
			if(key_frame.size()==1) // if just 1 key frame
			{
				//cout<<"test 1"<<endl;
				key_frame.erase(current_key_frame);
				current_key_frame=key_frame.begin();
			}
			else // more than 1 key frame
			{
				list<std::vector<RigTForm> >::iterator temp;
				if(current_key_frame == key_frame.begin())  // current key frame is the first frame
				{
					temp = current_key_frame;
					++temp;
					key_frame.erase(current_key_frame);
					current_key_frame = temp;
				}
				else     // current frame is not the first frame
				{
					temp = current_key_frame;
					//cout<<"test 3"<<endl;
					--temp;
					--key_frame_index;
					key_frame.erase(current_key_frame);
					current_key_frame = temp;
				}
			}
			cout << "Key frame " << key_frame_index-1 << " is deleted!" << endl;
		}
		else
		{
			cout<<"Current key frame is not defined yet!"<<endl;
		}
		Copy_current_key_frame_to_scene_graph();
	}
	else
	{
		cout<<"Can not delete the key frames while animating"<<endl;
	}
}

void new_a_key_frame()
{
	if(flag == 0)
	{
		vector<shared_ptr<SgRbtNode> > node_point_vector;   // temp vector to store scene rbt pointers
		vector<shared_ptr<SgRbtNode> >::iterator pointer = node_point_vector.begin(); // interator of node_point_vector
		dumpSgRbtNodes(g_world, node_point_vector);
		vector<RigTForm> temp_node;
		for(size_t i = 0; i< node_point_vector.size(); ++i)
			temp_node.push_back(node_point_vector[i]->getRbt());

		if(key_frame.empty()) // if current_key_frame is  undefined
		{
			key_frame.push_back(temp_node);
			current_key_frame = key_frame.begin();
			key_frame_index = 0;
			cout<<"Newly added keyframe "<<key_frame_index-1<<endl;

		}
		else  // current_key_frame is define
		{
			list<std::vector<RigTForm> >::iterator temp = current_key_frame;
			++temp;
			if(temp == key_frame.end()) //current is the last item
			{
				key_frame.push_back(temp_node);
				++current_key_frame;
				++key_frame_index;
				cout<<"Newly added keyframe "<<key_frame_index-1<<endl;
			}
			else
			{
				key_frame.insert(temp, temp_node);
				++current_key_frame;
				++key_frame_index;
				cout<<"Newly added keyframe "<<key_frame_index-1<<endl;
			}
		}
	}
	else
	{
		cout<<"Can not new a key frames while animating"<<endl;
	}
}

static vector<string> split(string s)
{
	vector<string> buf;
	int start=0, end=0;
	for(int i=0;i<s.size();i++)
	{
		if(s[i] ==' ')
		{
			end = i;
			buf.push_back(s.substr(start,end-start));
			start = i+1;
		}
	}
	return buf;
}

static void input_key_frame()
{
	if(flag == 0)
	{
		key_frame.clear();
		vector<RigTForm> rtf;
		char data[100];
		vector<string> v;
		fstream in("yl.txt");
		in.getline(data,100);
		//cout<<"data is "<<data<<endl;  //test
		v= split(data);
		int frame_size = stoi(v[0]);
		int vector_size = stoi(v[1]);
		for(int i=0; i<frame_size; i++)
		{
			for(int j=0; j<vector_size; j++)
			{
				in.getline(data,100);
				v= split(data);
				Cvec3 t(stof(v[0]), stof(v[1]), stof(v[2]));
				Quat q(stof(v[3]), stof(v[4]), stof(v[5]), stof(v[6]));
				RigTForm tf(t,q);
				rtf.push_back(tf);
			}
			key_frame.push_back(rtf);
			rtf.clear();
		}
		in.close();
		current_key_frame = key_frame.begin();
		key_frame_index = 0;
		Copy_current_key_frame_to_scene_graph();
		cout<<"Input key frames from file successfully"<<endl;
	}
	else
	{
		cout<<"Can not input key frames file while animating"<<endl;
	}
}

static void output_key_frame()
{
	if(flag == 0)
	{
		ofstream out("yl.txt");
		if(!key_frame.empty())
		{
			int frame_size = key_frame.size();
			int vector_size = (*current_key_frame).size();
			Cvec3 t = Cvec3();
			Quat q = Quat();
			out<<frame_size<<" "<<vector_size<<" "<<endl;
			for(list<std::vector<RigTForm> >::iterator temp = key_frame.begin(); temp != key_frame.end(); ++temp)
			{
				for(vector<RigTForm>::iterator i = temp->begin(); i != temp->end(); ++i)
				{
					t = (*i).getTranslation();
					q = (*i).getRotation();
					out<<t(0)<<" "<<t(1)<<" "<<t(2)<<" "<<q(0)<<" "<<q(1)<<" "<<q(2)<<" "<<q(3)<<" "<<endl;
				}
			}
			cout<<"output key frames into file successfully"<<endl;
		}
		else
		{
			cout<<"Current key frame is not defined yet!"<<endl;
			cout<<"No data to output!"<<endl;
		}
		out.close();
	}
	else
	{
		cout<<"Can not output key frames into file while animating"<<endl;
	}
}

static bool interpolateAndDisplay(float t)
{
	int frame_number = floor(t)+1; //number of current key frame
	float alpha = t-floor(t);
	if(frame_number != key_frame.size()-2)
	{
		current_key_frame = key_frame.begin();
		advance(current_key_frame, frame_number);
		list<std::vector<RigTForm> >::iterator temp= current_key_frame;
		--temp;
		list<std::vector<RigTForm> >::iterator frame0 = temp;
		++temp;
		list<std::vector<RigTForm> >::iterator frame1 = current_key_frame;
		++temp;
		list<std::vector<RigTForm> >::iterator frame2 = temp;
		++temp;
		list<std::vector<RigTForm> >::iterator frame3 = temp;

		vector<shared_ptr<SgRbtNode> > node_point_vector;   // temp vector to store scene rbt pointers
		dumpSgRbtNodes(g_world, node_point_vector);

		vector<RigTForm>::iterator i_0 = frame0->begin();
		vector<RigTForm>::iterator i_1 = frame1->begin();
		vector<RigTForm>::iterator i_2 = frame2->begin();
		vector<RigTForm>::iterator i_3 = frame3->begin();

		vector<shared_ptr<SgRbtNode> >::iterator pointer = node_point_vector.begin(); // interator of node_point_vector

		RigTForm te;
		while( i_0!=frame0->end() && i_1!=frame1->end() && i_2!=frame2->end() && i_3!=frame3->end()&& pointer !=node_point_vector.end())
		{
			//te = lerp((*i),(*j), alpha);
			te = interpolateCatmullRom((*i_0), (*i_1), (*i_2), (*i_3), alpha);
			(*pointer)->setRbt(te);
			++pointer;
			//++i;
			//++j;
			++i_0; ++i_1; ++i_2; ++i_3; 
		}
		glutPostRedisplay();
		return false;
	}
	else
	{
		return true;
	}
}
//int flag = 0;
static void animateTimerCallback(int ms)
{
	if(flag == 1)
	{
		float t = (float)ms/(float)g_msBetweenKeyFrames;
		bool endReached = interpolateAndDisplay(t);
		if (!endReached)
		{
			glutTimerFunc(1000/g_animateFramesPerSecond, animateTimerCallback, ms + 1000/g_animateFramesPerSecond);
		}
		else 
		{
			cout<<"Animation playback finished"<<endl;
			current_key_frame = key_frame.end();
			--current_key_frame;
			--current_key_frame;
			key_frame_index = key_frame.size()-2;
			flag = 0;
			Copy_current_key_frame_to_scene_graph();
		}
	}
	else
	{
		current_key_frame = key_frame.end();
		--current_key_frame;
		--current_key_frame;
		key_frame_index = key_frame.size()-2;
		Copy_current_key_frame_to_scene_graph();
	}
}

static void play_animation()
{
	flag++;
	flag = flag%2;
	if(flag == 1)
	{
		if(key_frame.size()<4)
		{
			cout<<"Need at least 4 keyframes for animation"<<endl;
			flag = 0;
		}
		else
		{
			animateTimerCallback(0);
		}
	}
	else
	{
		current_key_frame = key_frame.end();
		--current_key_frame;
		--current_key_frame;
		key_frame_index = key_frame.size()-2;
		Copy_current_key_frame_to_scene_graph();
	}
}

static void make_animation_faster()
{
	if(g_msBetweenKeyFrames>100)
	{
		g_msBetweenKeyFrames = g_msBetweenKeyFrames -100;
	}
	cout<<g_msBetweenKeyFrames <<" ms between Key frames"<<endl;
}

static void make_animatino_slower()
{
	g_msBetweenKeyFrames = g_msBetweenKeyFrames +100;
	cout<<g_msBetweenKeyFrames <<" ms between Key frames"<<endl;
}

static void subdivideMeshCatmullClark(Mesh& m);
static void subdivideMesh();

static bool bubbling_interpolateAndDisplay(float t)//------change
{
	mesh_temp = mesh;
	for(int i = 0; i<mesh_temp.getNumVertices(); i++)
	{
		Cvec3 p = mesh_temp.getVertex(i).getPosition();
		p = p*(0.1+abs(sin(t+i/5)));
		//cout<<"p is "<<p(0)<<" "<<p(1)<<" "<<p(2)<<endl;
		mesh_temp.getVertex(i).setPosition(p);
	}
	subdivideMesh();
	return false;
}

static void bubbling_animateTimerCallback(int ms)
{
	if(bubbling_animation_flag == 1)//------change
	{
		t_1 = (float)ms/(float)g_msBetweenKeyFrames;
		bool endReached = bubbling_interpolateAndDisplay(t_1);
		if (!endReached)
		{
			ti+=1000/g_animateFramesPerSecond;
			glutTimerFunc(bubbling_animate_speed, bubbling_animateTimerCallback, ti);
		}
	}
}

static void bubbling_animation()//------change
{
	bubbling_animation_flag++;
	bubbling_animation_flag = bubbling_animation_flag%2;
	if(bubbling_animation_flag == 1)
	{
		cout<<"Bubbling activated"<<endl;
		bubbling_animateTimerCallback(ti);
	}
	else
	{
		cout<<"No bubbling activated"<<endl;
	}
}

static void bubbling_animation_faster()	//------change
{
	if (bubbling_animate_speed > 1)
	{
		bubbling_animate_speed = bubbling_animate_speed / 2;
	}
	cout << "Bubbling animate speed is " << bubbling_animate_speed << endl;
}

static void bubbling_animatino_slower()		//------change
{
		bubbling_animate_speed = bubbling_animate_speed *2;
		cout << "Bubbling animate speed is " << bubbling_animate_speed << endl;
}

static void subdivideMeshCatmullClark(Mesh& m)//------change
{
	for(int i = 0; i<m.getNumFaces(); i++) //get new face vertex
	{
		Cvec3 vf = Cvec3(0,0,0);
		for(int j = 0; j<m.getFace(i).getNumVertices(); j++)
		{
			vf = vf + m.getFace(i).getVertex(j).getPosition();
		}
		vf = vf / m.getFace(i).getNumVertices();
		m.setNewFaceVertex(m.getFace(i), vf);
	} 


	for(int i = 0; i<m.getNumEdges(); i++) // get new edge vertex
	{
		Cvec3 ve = Cvec3(0,0,0);
		ve += m.getEdge(i).getVertex(0).getPosition();
		ve += m.getEdge(i).getVertex(1).getPosition();
		ve += m.getNewFaceVertex(m.getEdge(i).getFace(0));
		ve += m.getNewFaceVertex(m.getEdge(i).getFace(1));
		ve = ve / 4.0;
		m.setNewEdgeVertex(m.getEdge(i), ve);
	}

	for(int i = 0; i < m.getNumVertices(); i++) // get new vertex vertex
	{
		std::vector<Cvec3> vv_temp;
		Cvec3 vv = Cvec3(0,0,0);
		int nv = 0;
		const Mesh::Vertex v = m.getVertex(i);
		Mesh::VertexIterator it(v.getIterator()), it0(it);
		do
		{
			//[...]                                               // can use here it.getVertex(), it.getFace()
			vv_temp.push_back(it.getVertex().getPosition());
			vv_temp.push_back(m.getNewFaceVertex(it.getFace()));
		}
		while (++it != it0);                                  // go around once the 1ring
		for(int j = 0; j+1<vv_temp.size(); j++)
		{
			vv = vv + vv_temp[j] + vv_temp[j+1];
			j++;
		}
		nv = vv_temp.size()/2;
		Cvec3 vvv = v.getPosition() * ((nv-2.0)/nv) + vv*(1.0/(nv*nv));
		m.setNewVertexVertex(v,vvv);
	}
	m.subdivide();
	mesh_temp = m;
}

static void subdivideMesh()//------change
{
	if(subdivide_time>=0 && subdivide_time <6)
	{
		if(subdevide_decrease_flag == false) // if not decrease subdivision
		{
			for(int i = 0; i<subdivide_time;i++)// just subdivide the temp mseh
			{
				subdivideMeshCatmullClark(mesh_temp);
			}
		}
		else //if decrease subdivision
		{
			mesh_temp = mesh;
			if(ti!=0) // get the new temp mesh
			{
				for(int i = 0; i<mesh_temp.getNumVertices(); i++)
				{
					Cvec3 p = mesh_temp.getVertex(i).getPosition();
					p = p*(0.1+abs(sin(t_1+i/5)));
					//cout<<"p is "<<p(0)<<" "<<p(1)<<" "<<p(2)<<endl; //test
					mesh_temp.getVertex(i).setPosition(p);
				}
			}
			for(int i = 0; i<subdivide_time;i++)// then doo the subdivision
			{
				subdivideMeshCatmullClark(mesh_temp);
			}
			subdevide_decrease_flag = false;
		}
		g_meshCube->new_upload(mesh_temp, isSmoothShading);
		glutPostRedisplay();
	}
}

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
		case 'p': 		
			 g_pick = true;
			 cout<<"PICKING:ON"<<endl;
		break;
		case ' ':
			 Copy_current_key_frame_to_scene_graph();
		break;
		case 'u':
			 update_current_key_frame();
		break;
		case '>':
			 advance_to_next_key_frame();
		break;
		case '<':
			 retreat_to_previous_key_frame();
		break;
		case 'd':
			 delete_current_key_frame();
		break;
		case 'n':
			 new_a_key_frame();
		break;
		case 'i':
			 input_key_frame();
		break;
		case 'w':
			 output_key_frame();
		break;
		case 'y':
			 play_animation();
		break;
		case '+':
			 make_animation_faster();
		break;
		case '-':
			 make_animatino_slower();
		break;
		case 'f'://------change
			if(isSmoothShading == false) isSmoothShading = true;
			else isSmoothShading = false; 
			//if(mesh_temp.getNumVertices()!=0)
			//{
				g_meshCube->new_upload(mesh_temp, isSmoothShading);
			//}
			//else
			//{
			//	g_meshCube->new_upload(mesh, isSmoothShading);
			//}
		break;
		case 'b'://------change
			 bubbling_animation();
		break;
		case '7'://------change
			 bubbling_animatino_slower();
		break;
		case '8'://------change
			 bubbling_animation_faster();
		break;
		case '0'://------change
			if(subdivide_time>=0 && subdivide_time <5)
			{
				subdivide_time++;
				subdivideMesh();
			}
		break;
		case '9'://------change
			if(subdivide_time>0 && subdivide_time <6)
			{
				subdivide_time--;
				subdevide_decrease_flag = true;
				subdivideMesh();
			}
		break;
	}
	glutPostRedisplay();
}

static void initGlutState(int argc, char * argv[]) 
{
	glutInit(&argc, argv);                                  // initialize Glut based on cmd-line args
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);  //  RGBA pixel channels and double buffering
	glutInitWindowSize(g_windowWidth, g_windowHeight);      // create a window
	glutCreateWindow("yl3651_Assignment 7");                       // title the window

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

static void initMaterials()
{
  // Create some prototype materials
  Material diffuse("./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader");
  Material solid("./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader");


  // copy diffuse prototype and set red color
  g_redDiffuseMat.reset(new Material(diffuse));
  g_redDiffuseMat->getUniforms().put("uColor", Cvec3f(1, 0, 0));

  // copy diffuse prototype and set blue color
  g_blueDiffuseMat.reset(new Material(diffuse));
  g_blueDiffuseMat->getUniforms().put("uColor", Cvec3f(0, 0, 1));

  // normal mapping material
  g_bumpFloorMat.reset(new Material("./shaders/normal-gl3.vshader", "./shaders/normal-gl3.fshader"));
  g_bumpFloorMat->getUniforms().put("uTexColor", shared_ptr<Texture>(new ImageTexture("Fieldstone.ppm", true)));
  g_bumpFloorMat->getUniforms().put("uTexNormal", shared_ptr<Texture>(new ImageTexture("FieldstoneNormal.ppm", false)));

  // copy solid prototype, and set to wireframed rendering
  g_arcballMat.reset(new Material(solid));
  g_arcballMat->getUniforms().put("uColor", Cvec3f(0.27f, 0.82f, 0.35f));
  g_arcballMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // copy solid prototype, and set to color white
  g_lightMat.reset(new Material(solid));
  g_lightMat->getUniforms().put("uColor", Cvec3f(1, 1, 1));

  // pick shader
  g_pickingMat.reset(new Material("./shaders/basic-gl3.vshader", "./shaders/pick-gl3.fshader"));

  // meshcube shader
  g_meshcubeMat.reset(new Material("./shaders/basic-gl3.vshader", "./shaders/specular-gl3.fshader"));
  g_meshcubeMat->getUniforms().put("uColor", Cvec3f(1, 0.5, 1));
};

static void initGeometry() 
{
	initGround();
	initCubes();
	initSphere();                              //-------------change
	initMeshCube();
	mesh_temp = mesh;
}

static void constructRobot(shared_ptr<SgTransformNode> base, shared_ptr<Material> material)
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
    		shared_ptr<SgGeometryShapeNode> shape(
     				new MyShapeNode(shapeDesc[i].geometry,
                      	material, // USE MATERIAL as opposed to color
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
	g_groundNode->addChild(shared_ptr<MyShapeNode>(
                           new MyShapeNode(g_ground, g_bumpFloorMat, Cvec3(0, g_groundY, 0))));

	g_light1Node.reset(new SgRbtNode(RigTForm(g_light1)));
	g_light1Node->addChild(shared_ptr<MyShapeNode>(
				new MyShapeNode(g_sphere, g_lightMat)));

	g_light2Node.reset(new SgRbtNode(RigTForm(g_light2)));
	g_light2Node->addChild(shared_ptr<MyShapeNode>(
				new MyShapeNode(g_sphere, g_lightMat)));

	g_robot1Node.reset(new SgRbtNode(RigTForm(Cvec3(-2, 1, 0))));
	g_robot2Node.reset(new SgRbtNode(RigTForm(Cvec3(2, 1, 0))));

	constructRobot(g_robot1Node, g_redDiffuseMat); // a Red robot
  	constructRobot(g_robot2Node, g_blueDiffuseMat); // a Blue robot

  	g_meshCubeNode.reset(new SgRbtNode(RigTForm(Cvec3(0,1.5,-1))));
  	g_meshCubeNode->addChild(shared_ptr<MyShapeNode>(
		new MyShapeNode(g_meshCube, g_meshcubeMat)));

	g_world->addChild(g_skyNode);
	g_world->addChild(g_groundNode);
	g_world->addChild(g_robot1Node);
	g_world->addChild(g_robot2Node);
	g_world->addChild(g_light1Node);
	g_world->addChild(g_light2Node);
	g_world->addChild(g_meshCubeNode);
}

static void reset() 		
{
	// =========================================================
	// TODO:
	// - reset g_skyRbt and g_objectRbt to their default values
	// - reset the views and manipulation mode to default
	// - reset sky camera mode to use the "world-sky" frame
	// =========================================================    
	if(flag == 0)
	{
		initGLState();
  		initMaterials();
		initGeometry();
		initScene();
		g_sky_world = 0;  			
		g_arcballscale = 1;
		g_acrballScreenRadius =1;	
		arcball_visible = 1;  
		arcballRbt = g_world->getRbt(); 
		key_frame.clear();
		current_key_frame=key_frame.begin();
		isSmoothShading = true;
		key_frame_index = 0; // current number od the current key frame //----------change
		g_msBetweenKeyFrames = 2000; // 2 seconds between keyframes  //----------change
		g_animateFramesPerSecond = 60; // frames to render per second during animation playback  //----------change
		bubbling_animate_speed = 5;
		flag = 0;
		bubbling_animation_flag = 0;
		ti = 0;
		mesh.load("cube.mesh");
		mesh_temp = mesh;
		static int subdivide_time = 0;

		cout << "reset objects and modes to defaults" << endl;
	}
	else
	{
		cout<<"Can not reset while animating!"<<endl;
	}
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

		// remove initShaders() and add initMaterials();
  		// initShaders();
  		initMaterials();

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
