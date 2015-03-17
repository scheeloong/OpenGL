/***********************************************************
             CSC418/2504, Fall 2009
  
                 penguin.cpp
                 
       Simple demo program using OpenGL and the glut/glui 
       libraries

  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif



#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include "gl.h" 
#include "component.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
    Sleep(nanosec / 1000);
}
#endif


// Minimum and maximum rotation of most joints.
#define MAX_ROTATION  30
#define MIN_ROTATION -30

#define MAX_ROTATION_BODY  20
#define MIN_ROTATION_BODY -20

#define MAX_ROTATION_HEAD  10
#define MIN_ROTATION_HEAD -10

#define MAX_ROTATION_HAND  30
#define MIN_ROTATION_HAND -30

#define MAX_ROTATION_LEG  10
#define MIN_ROTATION_LEG -10

#define MAX_ROTATION_FEET  30
#define MIN_ROTATION_FEET -20


// Minimum and maximum scaling factor for the penguin.
#define MAX_SCALE 5.0
#define MIN_SCALE 0.1

// Minimum and maximum translation 
#define MAX_TRANSLATION_BODY_Y  25
#define MIN_TRANSLATION_BODY_Y -25

#define MAX_TRANSLATION_BODY_X  300
#define MIN_TRANSLATION_BODY_X -200

#define MAX_TRANSLATION_MOUTH  2
#define MIN_TRANSLATION_MOUTH -0.6

//---------------------------
// Global Variables 
//---------------------------
const float PI = 3.14159;

//---------------------------
// USER INTERFACE VARIABLES 
//---------------------------

// Window settings
int windowID = 0;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)

int Win[2];                 // window (x,y) = (width, height) size, the window is where everything is drawn 
//---------------------------
// ANIMATION VARIABLES
//---------------------------

// Animation settings
int animate_mode = 0;       // 0 = no animation, 1 = animation
int animation_frame = 0;      // Specify current frame of animation

// Joint parameters
const float JOINT_MIN = -45.0f;
const float JOINT_MAX =  45.0f;
float joint_rot = 0.0f;


// ---------------- ANIMATION STATE VARIABLES ---------------------
// The entire penguin object
Entity penguin;


// Controls how much to zoom into robot 
Zoomer* zoomer;  // Zoomer that controls the size of the robot

// Controls translation amount
Translator* translator; // translate the penguin by a certain amount 

// Hidden Joints
Joint* bodyJoint;  // A joint connected from penguin to main Body

// The 6 circular joint pointers
Joint* headJoint;  // Joint connecting the body to the neck.	  
Joint* handJoint; // Joint connecting body to hand
Joint* leftLegJoint;  // connects body to leftLeg
Joint* rightLegJoint;  // connects body to rightLeg
Joint* leftFeetJoint;  // connects leftLeg to leftFeet
Joint* rightFeetJoint;  // connects rightLeg to rightFeet

// The 7th Mouth joint
Joint* mouthJoint;  // Joint connecting the head to mouth	  

// Note:
// These objects are created in main() 
// Body
// Head (Note: Head includes an eye and upper mouth that moves along with it)
// Mouth
// Hand
// LeftLeg
// RightLeg
// leftFeet
// rightFeet

// ----------- Penguin components defined ---------------------------------------------------------------------------------------------
// Final Kinematics, from root to leaves 
// Notation: a->b means  'a' has a joint to connect to 'b' 
// 	     a-->b means 'a' directly contains 'b' (using addComponent()) 
// Current Progress: penguin-->zoomer--> bodyJoint -> Body --> headJoint -> Head --> mouthJoint => Mouth
//			    -->translator		   --> handJoint -> Hand
//		     	  	   			   --> leftLegJoint  -> LeftLeg --> leftFeetJoint   -> leftFeet
//		     	  	    			   --> rightLegJoint -> RightLeg --> rightFeetJoint -> rightFeet
// ------------------------------------------------------------------------------------------------------------------------------------

// For Animation 
int frame        = 0;  // Current frame number.
int is_animating = 0;  // 0 -> Stop Animating 
		       // 1 -> Start Animating from Reset 

// For Hard Coded Animation
 float BODY_WIDTH    = 100.0f;
 float BODY_LENGTH   = 50.0f;
 float HAND_HEIGHT    = 40.0f;
 float HAND_WIDTH     = 15.0f;
 float HEAD_WIDTH    = 50.0f;
 float HEAD_HEIGHT   = 25.0f;
 float MOUTH_WIDTH    = 50.0f;
 float MOUTH_HEIGHT   = 5.0f;
 float LEG_WIDTH  = 15.0f;
 float LEG_HEIGHT = 40.0f;
 float FEET_WIDTH  = 40.0f;
 float FEET_HEIGHT = 10.0f;

//-------------------------------------
// Function Prototypes 
//-------------------------------------

// Initialization functions
void initGlut(char* winName); // helper function defined below for initialization
void initGlui();
void initGl();

// Callbacks for handling events in glut
void myReshape(int w, int h);  // used to resize Window 
void animate();
void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);

// Functions to help draw the object
void drawSquare(float size);

// Return the current system clock (in seconds)
double getTime();

//----------------------------------------------------------------
// Main Function
//----------------------------------------------------------------
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{
    // Process program arguments
    if(argc != 3) 
    {
        printf("Usage: demo [width] [height]\n");
        printf("Using 600x800 window by default...\n");
        Win[0] = 600;
        Win[1] = 800;
        // ./penguin 400 800 
        // means 400 width and 800 height window 
    }
    else
    {
        Win[0] = atoi(argv[1]); // change string argument to integers
        Win[1] = atoi(argv[2]); // change string argument to integers
    }

//---------------------------------------------------------------------------------------------
// Build Kinematic Tree 
//---------------------------------------------------------------------------------------------
// ----------- Penguin components defined --------------------------------------------------------------------------------------------
// Final Kinematics, from root to leaves 
// Notation: a->b means  'a' has a joint to connect to 'b' 
// 	     a-->b means 'a' directly contains 'b' (using addComponent()) 
// Current Progress: penguin-->zoomer--> bodyJoint -> Body --> headJoint -> Head --> mouthJoint => Mouth
//					  		   --> handJoint -> Hand
//		     	  	   			   --> leftLegJoint  -> LeftLeg --> leftFeetJoint   -> leftFeet
//		     	  	    			   --> rightLegJoint -> RightLeg --> rightFeetJoint -> rightFeet
// ------------------------------------------------------------------------------------------------------------------------------------

// Note: It draws based on the order of priority you give it in this kinematic tree
// Thus, you need draw main body last so everything else is over it 
// Also, if you want object A to be blue color, you need to call the color function before you call object A 
// It is also drawn based on the order you point the joints 

 // Create a new head for the penguin. 
//--------------------------------------------
    // Head of the penguin.
    Entity Head;
//--------------------------------------------
    // Rectangle for the head.
    Head.AddComponent(new Color(0, 1, 1)); // Blue Head
    Head.AddComponent(new Rectangle(-HEAD_WIDTH/2, 0, HEAD_WIDTH/2, HEAD_HEIGHT)); 
    // Whiteseyes.
    Head.AddComponent(new Color(0.8, 0.8, 0.8)); // Grey Eye
    Head.AddComponent(new Circle(-HEAD_WIDTH/4, 0.60*HEAD_HEIGHT, 0.3 * HEAD_HEIGHT));
    // Black pupils.
    Head.AddComponent(new Color(0.1, 0.1, 0.1)); // Black pupil 
    Head.AddComponent(new Circle(-HEAD_WIDTH/4, 0.60*HEAD_HEIGHT, 0.15 * HEAD_HEIGHT));
    // Top of Mouth 
    Head.AddComponent(new Color(1, 1, 0)); // Yellow Mouth
    Head.AddComponent(new Rectangle(-HEAD_WIDTH/2 -MOUTH_WIDTH, 2*MOUTH_HEIGHT, -HEAD_WIDTH/2, 3*MOUTH_HEIGHT));
//--------------------------------------------
    // Mouth 
    Entity Mouth;
//--------------------------------------------
    Mouth.AddComponent(new Color(1, 1, 0)); // Black pupil 
    Mouth.AddComponent(new Rectangle(-HEAD_WIDTH/2 -MOUTH_WIDTH , 0, -HEAD_WIDTH/2, MOUTH_HEIGHT));

    Mouth.AddComponent(new Color(0, 1, 1)); // Black Circle Joint

    mouthJoint = new Joint(0, 0, 0, &Mouth, true); // Attach Mouth to mouthJoint

    Head.AddComponent(mouthJoint); // Attach mouthJoint to head

//--------------------------------------------
    // Hand
    Entity Hand; 
//--------------------------------------------
    Hand.AddComponent(new Color(0, 1, 1)); // Blue Hand 
    Hand.AddComponent(new Rectangle(-HAND_WIDTH/2, -HAND_HEIGHT , HAND_WIDTH/2,0 + HAND_HEIGHT/6)); 
    Hand.AddComponent(new Color(0, 1, 1)); // Black Circle Joint for Hand
// Current Progress: Neck -> Head  ( Note: a->b means a joint from 'a' to 'b')

    // Global pointer defined above, make it point to neck just created 
    headJoint = new Joint(0, BODY_LENGTH/2 - MOUTH_WIDTH/2, 0, &Head, true);
    handJoint = new Joint(0, -BODY_WIDTH*(1.0/4.0), 0, &Hand, true);


//--------------------------------------------
    // Left Feet
    Entity LeftFeet; 
//--------------------------------------------

    LeftFeet.AddComponent(new Color(1, 1, 0)); // Yellow Left Feet
    LeftFeet.AddComponent(new Rectangle(-LEG_WIDTH/4 -FEET_WIDTH/2, -FEET_HEIGHT , FEET_WIDTH/4, FEET_HEIGHT/6)); 
    LeftFeet.AddComponent(new Color(0, 1, 1)); // Black Circle Joint for LeftLeg

    leftFeetJoint = new Joint(0 ,-LEG_HEIGHT*0.8 , 30, &LeftFeet, true);

//--------------------------------------------
    // Left Leg
    Entity LeftLeg; 
//--------------------------------------------

    LeftLeg.AddComponent(new Color(0, 1, 1)); // Blue Left Leg 
    LeftLeg.AddComponent(new Rectangle(-LEG_WIDTH/2, -LEG_HEIGHT , LEG_WIDTH/2,0 + LEG_HEIGHT/6)); 
    LeftLeg.AddComponent(new Color(0, 1, 1)); // Black Circle Joint for LeftLeg
    LeftLeg.AddComponent(leftFeetJoint); 

    leftLegJoint = new Joint(-BODY_WIDTH/4, -BODY_LENGTH*3/2, 0, &LeftLeg, true);

//--------------------------------------------
    // Right Feet
    Entity RightFeet; 
//--------------------------------------------

    RightFeet.AddComponent(new Color(1, 1, 0)); // Yellow Left Feet
    RightFeet.AddComponent(new Rectangle(-LEG_WIDTH/4 -FEET_WIDTH/2, -FEET_HEIGHT , FEET_WIDTH/4, FEET_HEIGHT/6)); 
    RightFeet.AddComponent(new Color(0, 1, 1)); // Black Circle Joint for LeftLeg

    rightFeetJoint = new Joint(0 ,-LEG_HEIGHT*0.8 , 30, &RightFeet, true);

//--------------------------------------------
    // Right Leg
    Entity RightLeg; 
//--------------------------------------------

    RightLeg.AddComponent(new Color(0, 1, 1)); // Blue Right Leg 
    RightLeg.AddComponent(new Rectangle(-LEG_WIDTH/2, -LEG_HEIGHT , LEG_WIDTH/2,0 + LEG_HEIGHT/6)); 
    RightLeg.AddComponent(new Color(0, 1, 1)); // Black Circle Joint for RightLeg
    RightLeg.AddComponent(rightFeetJoint); 

    rightLegJoint = new Joint(BODY_WIDTH/4, -BODY_LENGTH*3/2, 0, &RightLeg, true);

//--------------------------------------------
    // Body 
    Entity Body; 
//--------------------------------------------
    Body.AddComponent(new Color(0.5, 0.5, 0)); // Black pupil 
    Body.AddComponent(new Circle(0, -BODY_WIDTH/2, BODY_WIDTH/2)); 
    Body.AddComponent(headJoint); 
    Body.AddComponent(handJoint); 
    Body.AddComponent(leftLegJoint); 
    Body.AddComponent(rightLegJoint); 

    bodyJoint = new Joint(0, BODY_LENGTH/2 - MOUTH_WIDTH/2, 0, &Body, true);
	
    // Note: Need translate by (-) values of X to move forward but not passed too much from screen .
    translator = new Translator(0,0); // begin by translating by nothing
    // Top level object controlling the robot in the scene.
    zoomer = new Zoomer(1);
    penguin.AddComponent(zoomer); // To allow zooming 
    penguin.AddComponent(translator); // To allow zooming 
    penguin.AddComponent(bodyJoint); 
//-------------------------------------------------------------------------------------------------------------------------------------
// Done building kinematic tree, need set up window and run main event loop 

    // Note: Need initialize all classes and connection before calling functions below 
    // Initialize glut, glui, and opengl
    glutInit(&argc, argv);
    initGlut(argv[0]); // helper function defined below for initialization of OpenGL so code looks cleaner
    initGlui(); // helper function defined below to add all buttons 
    initGl(); // helper function defined below to set color 

    // Invoke the standard GLUT main event loop
    glutMainLoop(); // Draw Stuff 
    return 0;         // never reached
}

//---------------------------------------------------------------------------------------------
// Helper Functions
//---------------------------------------------------------------------------------------------


// This function initialize glut and create a window with the specified caption 
void initGlut(char* winName)
{
    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(winName);

    // Setup callback functions to handle events
    glutReshapeFunc(myReshape); // Call myReshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed 
}


//---------------------------------------------------------------------------------------------

// This function quits the button handler.  
// It is called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

//---------------------------------------------------------------------------------------------

// This function animates button handler.  
// It is called when the "animate" checkbox is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui->sync_live();

  animation_frame = 0;
  if(animate_mode == 1) {
    // start animation
    GLUI_Master.set_glutIdleFunc(animate);
  } else {
    // stop animation
    GLUI_Master.set_glutIdleFunc(NULL);
  }
}

//---------------------------------------------------------------------------------------------


// This function initializes all the possibe buttons. 
// This function initialize GLUI and the user interface
void initGlui()
{
    GLUI_Master.set_glutIdleFunc(NULL);

    // Create GLUI window
    glui = GLUI_Master.create_glui("Glui Window", 0, Win[0]+10, 0);

// Buttons
    // Create a spinner to get input variables 
    GLUI_Spinner *joint_spinner; 

// SCALING 
    // 1. Zoom  =  Scaling size of entire penguin 
    joint_spinner = glui->add_spinner("Zoom", GLUI_SPINNER_FLOAT, zoomer->f_ptr());
    joint_spinner->set_speed(0.5);
    joint_spinner->set_float_limits(MIN_SCALE, MAX_SCALE, GLUI_LIMIT_CLAMP); // limit of zooming is 5

// ROTATION 
    // 2. Body = Rotating penguin left and right 
    joint_spinner = glui->add_spinner("Body Rotate", GLUI_SPINNER_FLOAT, bodyJoint->a_ptr());
    joint_spinner->set_speed(1); // each step increment per click
    joint_spinner->set_float_limits(MIN_ROTATION_BODY , MAX_ROTATION_BODY , GLUI_LIMIT_CLAMP);

    // 3. Head
    joint_spinner = glui->add_spinner("Head Rotate", GLUI_SPINNER_FLOAT, headJoint->a_ptr());
    joint_spinner->set_speed(1);
    joint_spinner->set_float_limits( MIN_ROTATION_HEAD,  MAX_ROTATION_HEAD, GLUI_LIMIT_CLAMP);

    // 4. Hand
    joint_spinner = glui->add_spinner("Hand Rotate", GLUI_SPINNER_FLOAT, handJoint->a_ptr());
    joint_spinner->set_speed(2);
    joint_spinner->set_float_limits( MIN_ROTATION_HAND,  MAX_ROTATION_HAND, GLUI_LIMIT_CLAMP);

    // 5. Left Leg
    joint_spinner = glui->add_spinner("Left Leg Rotate", GLUI_SPINNER_FLOAT, leftLegJoint->a_ptr());
    joint_spinner->set_speed(1);
    joint_spinner->set_float_limits( MIN_ROTATION_LEG,  MAX_ROTATION_LEG, GLUI_LIMIT_CLAMP);

    // 6. Right Leg
    joint_spinner = glui->add_spinner("Right Leg Rotate", GLUI_SPINNER_FLOAT, rightLegJoint->a_ptr());
    joint_spinner->set_speed(1);
    joint_spinner->set_float_limits( MIN_ROTATION_LEG,  MAX_ROTATION_LEG, GLUI_LIMIT_CLAMP);


    // 7. Left Feet
    joint_spinner = glui->add_spinner("Left Feet Rotate", GLUI_SPINNER_FLOAT, leftFeetJoint->a_ptr());
    joint_spinner->set_speed(3);
    joint_spinner->set_float_limits( MIN_ROTATION_FEET,  MAX_ROTATION_FEET, GLUI_LIMIT_CLAMP);

    // 8. Right Feet
    joint_spinner = glui->add_spinner("Right Feet Rotate", GLUI_SPINNER_FLOAT, rightFeetJoint->a_ptr());
    joint_spinner->set_speed(3);
    joint_spinner->set_float_limits( MIN_ROTATION_FEET,  MAX_ROTATION_FEET, GLUI_LIMIT_CLAMP);

// TRANSLATION 
    // 9. Mouth 
    joint_spinner = glui->add_spinner("Mouth Translate", GLUI_SPINNER_FLOAT, mouthJoint->y_ptr());
    joint_spinner->set_speed(2);
    joint_spinner->set_float_limits( MIN_TRANSLATION_MOUTH, MAX_TRANSLATION_MOUTH, GLUI_LIMIT_CLAMP);

    // 10. Body Y
    joint_spinner = glui->add_spinner("Body_Y Translate", GLUI_SPINNER_FLOAT, translator->y_ptr());
    joint_spinner->set_speed(1);
    joint_spinner->set_float_limits( MIN_TRANSLATION_BODY_Y, MAX_TRANSLATION_BODY_Y, GLUI_LIMIT_CLAMP);

    // 11. Body X
    joint_spinner = glui->add_spinner("Body_X Translate", GLUI_SPINNER_FLOAT, translator->x_ptr());
    joint_spinner->set_speed(10);
    joint_spinner->set_float_limits( MIN_TRANSLATION_BODY_X, MAX_TRANSLATION_BODY_X, GLUI_LIMIT_CLAMP);

    // Add button to specify animation mode 
    glui->add_separator();
    glui->add_checkbox("Animate", &animate_mode, 0, animateButton);// call function animateButton (uses function pointer) when button Animate is pressed

    // Add "Quit" button
    glui->add_separator();
    glui->add_button("Quit", 0, quitButton); // call function quitButton (uses function pointer) when button Quit is pressed

    // Set the main window to be the "active" window
    glui->set_main_gfx_window(windowID);
}

//---------------------------------------------------------------------------------------------

// This function performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}

//---------------------------------------------------------------------------------------------


// This function callback idle function for animating the scene
// It animates the character's joints
void animate()
{

/*
// Controls translation amount
Translator* translator; // translate the penguin by a certain amount 

// Hidden Joints
Joint* bodyJoint;  // A joint connected from penguin to main Body

// The 6 circular joint pointers
Joint* headJoint;  // Joint connecting the body to the neck.	  
Joint* handJoint; // Joint connecting body to hand
Joint* leftLegJoint;  // connects body to leftLeg
Joint* rightLegJoint;  // connects body to rightLeg
Joint* leftFeetJoint;  // connects leftLeg to leftFeet
Joint* rightFeetJoint;  // connects rightLeg to rightFeet

// The 7th Mouth joint
Joint* mouthJoint;  // Joint connecting the head to mouth


// Minimum and maximum rotation of most joints.
#define MAX_ROTATION  30
#define MIN_ROTATION -30

#define MAX_ROTATION_BODY  20
#define MIN_ROTATION_BODY -20

#define MAX_ROTATION_HEAD  10
#define MIN_ROTATION_HEAD -10

#define MAX_ROTATION_HAND  30
#define MIN_ROTATION_HAND -30

#define MAX_ROTATION_LEG  10
#define MIN_ROTATION_LEG -10

#define MAX_ROTATION_FEET  30
#define MIN_ROTATION_FEET -20


// Minimum and maximum scaling factor for the penguin.
#define MAX_SCALE 5.0
#define MIN_SCALE 0.1

// Minimum and maximum translation 
#define MAX_TRANSLATION_BODY_Y  25
#define MIN_TRANSLATION_BODY_Y -25

#define MAX_TRANSLATION_BODY_X  300
#define MIN_TRANSLATION_BODY_X -200

#define MAX_TRANSLATION_MOUTH  2
#define MIN_TRANSLATION_MOUTH -0.6
*/

    // Update geometry
    double oscillate = (sin(animation_frame * 0.1) + 1.0) / 2.0;

    // Another convenience macro that will change the value of @obj->attr@
    // based on the current frame.
    translator->set_x(oscillate * (MIN_TRANSLATION_BODY_X) + (1 - oscillate) *  MAX_TRANSLATION_BODY_X);
    translator->set_y(oscillate * (MIN_TRANSLATION_BODY_Y) + (1 - oscillate) *  MAX_TRANSLATION_BODY_Y);
    handJoint->set_a(oscillate * (MIN_ROTATION_HAND) + (1 - oscillate) *  MAX_ROTATION_HAND);
    leftLegJoint->set_a(oscillate * (MIN_ROTATION_LEG) + (1 - oscillate) *  MAX_ROTATION_LEG);
    rightLegJoint->set_a(oscillate * (MIN_ROTATION_LEG) + (1 - oscillate) *  MAX_ROTATION_LEG);
    leftFeetJoint->set_a(oscillate * (MIN_ROTATION_FEET) + (1 - oscillate) *  MAX_ROTATION_FEET);
    rightFeetJoint->set_a(oscillate * (MIN_ROTATION_FEET) + (1 - oscillate) *  MAX_ROTATION_FEET);
    mouthJoint->set_y(oscillate * (MIN_TRANSLATION_MOUTH) + (1 - oscillate) *  MAX_TRANSLATION_MOUTH);

    // Update user interface
    glui->sync_live();

    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();

    // Increment the frame number.

    // Wait 50 ms between frames (20 frames per second)
    // Run at 20 FPS by sleeping before this animate function returns and allow main function to call animate() again. 
    animation_frame++;
    usleep(50000);


}

//---------------------------------------------------------------------------------------------

// This function handles the window being resized by updating the viewport and projection matrices
void myReshape(int w, int h)
{
    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-w/2, w/2, -h/2, h/2);

    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);
    Win[0] = w;
    Win[1] = h;
}

//---------------------------------------------------------------------------------------------


// This is the display callback. It gets called by the event handler to draw the scene, 
// so this is where you need to build your scene -- make your changes and additions here.
// All rendering happens in this function.  
// For Assignment 1, updates to geometry should happen in the "animate" function.
void display(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);

    // OK, now clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ///////////////////////////////////////////////////////////
    //   This function draw the scene
    //   This should include function calls to pieces that
    //   apply the appropriate transformation matrice and
    //   render the individual body parts.
    ///////////////////////////////////////////////////////////
    // Draw everything 
    penguin.Update(); // It calls down the kinematic tree recursive and every member calls the update function 

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}

//---------------------------------------------------------------------------------------------


// Draw a square of the specified size, centered at the current location
void drawSquare(float width)
{
    // Draw the square
    glBegin(GL_POLYGON);
    glVertex2d(-width/2, -width/2);
    glVertex2d(width/2, -width/2);
    glVertex2d(width/2, width/2);
    glVertex2d(-width/2, width/2);
    glEnd();
}
//---------------------------------------------------------------------------------------------


