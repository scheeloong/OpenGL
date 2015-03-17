/***********************************************************
             CSC418, FALL 2009
 
                 penguin.cpp
                 author: Mike Pratscher
                 based on code by: Eron Steger, J. Radulovich

        Main source file for assignment 2
        Uses OpenGL, GLUT and GLUI libraries
  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design. In particular, see lines marked 'README'.
        
        Be sure to also look over keyframe.h and vector.h.
        While no changes are necessary to these files, looking
        them over will allow you to better understand their
        functionality and capabilites.

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
#include "gl.h"
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Added myself 
#include "component.h" // to make the program OOP and much cleaner to code by using classes 
#include "LoadShaders.h"
#include "keyframe.h"
#include "timer.h"
#include "vector.h"



// *************** GLOBAL VARIABLES *************************
const float PI = 3.14159;
const float SPINNER_SPEED = 3; // How fast you can turn a spinner on the window menu 

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
int Win[2];                 // window (x,y) size, width and height 

GLUI* glui_light;       // Glui window to control light and shading
GLUI* glui_joints;          // Glui window with joint controls
GLUI* glui_keyframe;        // Glui window with keyframe controls
GLUI* glui_render;          // Glui window for render style

char msg[256];              // String used for status message
GLUI_StaticText* status;    // Status message ("Status: <msg>")


// ---------------- ANIMATION VARIABLES ---------------------

//------------------------------------------
// Camera settings
//------------------------------------------
bool updateCamZPos = false; // initialize update camera position to false  
int  lastX = 0;
int  lastY = 0;
const float ZOOM_SCALE = 0.01; // Zoom scale of camera

// Camera positions
GLdouble camXPos =  0.0;
GLdouble camYPos =  0.5;
GLdouble camZPos = -7.5;

// The values of the near clip plane and the far clip plane limits 
const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0; // set clip to be far away 

//------------------------------------------
// Light settings
//------------------------------------------
float light_angle = 90;
const float LIGHT_CIRCLE_RADIUS = 100;

// Shader modes 
enum { SHADE_FLAT, SHADE_SMOOTH };
int shadeModel = SHADE_FLAT;

//------------------------------------------
// Render settings
//------------------------------------------
// The 5 different rendering modes required for Assignment 2 
enum 
{
    WIREFRAME,
    SOLID,
    OUTLINED,
    METAL,
    MATTE
};

// Initialize the render style to WireFrame 
int renderStyle = WIREFRAME; // README: the selected render style

// Animation settings
int animate_mode = 0; // 0 = no animation, 1 = animate

// Keyframe settings
const char filenameKF[] = "keyframes.txt";  // file for loading / saving keyframes

// TODO
Keyframe* keyframes;            // list of keyframes

int maxValidKeyframe   = 0;     // index of max VALID keyframe (in keyframe list)
const int KEYFRAME_MIN = 0;     // Minimum value fo key frame 
const int KEYFRAME_MAX = 32;    // README: specifies the max number of keyframes

// Frame settings
char filenameF[128];            // storage for frame filename

int frameNumber = 0;            // current frame being dumped
int frameToFile = 0;            // flag for dumping frames to file

const float DUMP_FRAME_PER_SEC = 24.0;   // frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 10.0;    // README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings

// README: This is the key data structure for
// updating keyframes in the keyframe list and
// for driving the animation.
//   i) When updating a keyframe, use the values
//      in this data structure to update the
//      appropriate keyframe in the keyframe list.
//  ii) When calculating the interpolated pose,
//      the resulting pose vector is placed into
//      this data structure. (This code is already
//      in place - see the animate() function)
// iii) When drawing the scene, use the values in
//      this data structure (which are set in the
//      animate() function as described above) to
//      specify the appropriate transformations.
Keyframe* joint_ui_data;

// The main PENGUIN entity 
Entity PENGUIN;
// Initialize to color the penguin 
bool colorPenguin = true;
int coloredMaterials = true;
Entity wireFrameMode;
Entity solidMode;

// Create a function pointer to enabling and disabling colors for the penguin
Component *ENABLE_COLOR_PENGUIN = Component::function([]{ colorPenguin = true; });
Component *DISABLE_COLOR_PENGUIN = Component::function([]{ colorPenguin = false; });

// README: To change the range of a particular DOF,
// simply change the appropriate min/max values below
// Root is the global position of the penguin 
const float ROOT_TRANSLATE_X_MIN = -5.0;
const float ROOT_TRANSLATE_X_MAX =  5.0;
const float ROOT_TRANSLATE_Y_MIN = -5.0;
const float ROOT_TRANSLATE_Y_MAX =  5.0;
const float ROOT_TRANSLATE_Z_MIN = -5.0;
const float ROOT_TRANSLATE_Z_MAX =  5.0;
const float ROOT_ROTATE_X_MIN    = -180.0;
const float ROOT_ROTATE_X_MAX    =  180.0;
const float ROOT_ROTATE_Y_MIN    = -180.0;
const float ROOT_ROTATE_Y_MAX    =  180.0;
const float ROOT_ROTATE_Z_MIN    = -180.0;
const float ROOT_ROTATE_Z_MAX    =  180.0;

// Minimum and maximum of the head translation
const float HEAD_MIN             = -30.0;
const float HEAD_MAX             =  45.0;

// Min and max of the shoulder pitch, yaw, and roll 
const float SHOULDER_PITCH_MIN   = -45.0;
const float SHOULDER_PITCH_MAX   =  45.0;
const float SHOULDER_YAW_MIN     = -45.0;
const float SHOULDER_YAW_MAX     =  45.0;
const float SHOULDER_ROLL_MIN    = -45.0;
const float SHOULDER_ROLL_MAX    =  45.0;

// Min and max of the hip pitch, yaw, and roll 
const float HIP_PITCH_MIN        = -45.0;
const float HIP_PITCH_MAX        =  45.0;
const float HIP_YAW_MIN          = -30.0;
const float HIP_YAW_MAX          =  30.0;
const float HIP_ROLL_MIN         = -20.0;
const float HIP_ROLL_MAX         =  20.0;

// Min and max of the beak 
const float BEAK_MIN             =  0.0;
const float BEAK_MAX             = 0.15;
// Min and max of the elbow
const float ELBOW_MIN            =  -30.0;
const float ELBOW_MAX            = 30.0;
// Min and max of the knee 
const float KNEE_MIN             =  -50.0;
const float KNEE_MAX             = 50.0;

// Min and max for light
const float LIGHT_POS_MIN        = -180.0;
const float LIGHT_POS_MAX        =  180.0;

//--------------------------------------------
// OTHER GLOBAL VARIABLES
//--------------------------------------------

// Global Variables for drawing shapes 

// Head
const float PAD = 0.05; 
const float HEAD_WIDTH = 2.00; 
const float HEAD_HEIGHT = 1.00; 
const float HEAD_DEPTH = 0.75; 

// Beak  
const float BEAK_WIDTH = 0.25; 
const float BEAK_HEIGHT = 0.50; 
const float BEAK_DEPTH = 0.25; 


// Body 
const float BODY_WIDTH = 1.75; 
const float BODY_HEIGHT = 2.50; 
const float BODY_DEPTH = 1.25; 

// Shoulder

// Elbow


// Hip (Leg) 

// Knee




// ***********  FUNCTION HEADER DECLARATIONS ****************

// To easily translate degree to radians 
inline float deg2rad(float deg) { return deg * M_PI / 180; }

// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();

// Callbacks for handling events in glut
void reshape(int w, int h); // Reshape the window size 
void animate(); // For animating the penguin automatically 
void display(void); // The display function that displays the penguin 
void mouse(int button, int state, int x, int y); // The mouse event handler
void motion(int x, int y);

// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);

void drawCube();// temporary helper function for starting this assignment 

// Image functions
void writeFrame(char* filename, bool pgm, bool frontBuffer);

//--------------------------------------------------------
// Functions 
//--------------------------------------------------------
// A Supplier that returns DOF values from the current state.
class DOFSupplier : public Supplier<float> 
{    
    public:
        DOFSupplier(int dof) { dof_ = dof; }
        virtual ~DOFSupplier() {}
        virtual float Get() { return (*joint_ui_data).getDOF(dof_); }
    private:
        int dof_;
};

#define DOFS(dof) (new DOFSupplier(dof)) // a macro to get a new Degree of freedom supplier 

//--------------------------------------------------------
// main() function
//--------------------------------------------------------
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 640x480 window by default...\n");
        Win[0] = 640; // width 
        Win[1] = 480; // height 
    } else {
        Win[0] = atoi(argv[1]); // window width
        Win[1] = atoi(argv[2]); // window height 
    }



    // Initialize data structs, glut, glui, and opengl
    initDS(); // Initialize key frames
    initGlut(argc, argv); // Initialize Glut
    initGlui(); // Set up user interface 
    initGl(); // Initialize openGL 
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
// DRAW THE PENGUIN HERE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

    // Initialize wireFrame Rendering Mode 
    wireFrameMode.AddComponent(Component::disable(GL_LIGHTING));	// no lights
    wireFrameMode.AddComponent(Component::polygonMode(GL_FRONT_AND_BACK, GL_LINE)); // draw with just line

    // Initialize solid Rendering Mode   
    solidMode.AddComponent(Component::disable(GL_LIGHTING)); // no lights
    solidMode.AddComponent(Component::polygonMode(GL_FRONT_AND_BACK, GL_FILL)); // draw with filled cuboids 

    //-----------------------------------
    // Eye 
    //-----------------------------------
    Entity eye;
    eye.AddComponent(Component::color(0.0, 0.0, 0.0)->onlyWhen(&colorPenguin));
    eye.AddComponent(Component::circle((-0.7), 0, 0, 0.1));
    eye.pushPopAttribute(GL_COLOR_BUFFER_BIT);

    //-----------------------------------
    // Beak   
    //-----------------------------------
    Entity Stagnantbeak; // The upper beak that doesn't move 
    Stagnantbeak.AddComponent(Component::color(0.7, 0.6, 0.4)->onlyWhen(&colorPenguin));
    Stagnantbeak.AddComponent(Component::cuboid((-HEAD_WIDTH/8), 0, -HEAD_DEPTH/2, HEAD_WIDTH/4, HEAD_HEIGHT*0.1,  HEAD_DEPTH/4)); 

    // The lower beak that moves up and down
    Entity beak;
    beak.AddComponent(Component::translatable(DOFS(Keyframe::BEAK_USElESS), DOFS(Keyframe::BEAK), DOFS(Keyframe::BEAK_USElESS))); // note: Must put translate before color as order is important 
    beak.AddComponent(Component::color(0.4, 0.5, 0.4)->onlyWhen(&colorPenguin));
    beak.AddComponent(Component::cuboid((-HEAD_WIDTH/8), 0, -HEAD_DEPTH/2, HEAD_WIDTH/4, HEAD_HEIGHT*0.1,  HEAD_DEPTH/4)); 

    //-----------------------------------
    // Head 
    //-----------------------------------
    Entity head;
    head.AddComponent(Component::rotatable(DOFS(Keyframe::HEAD), 0,0));	// Available enumerations for KeyFrame in keyframe.h 
    head.AddComponent(Component::color(0.9, 0.5, 0.5)->onlyWhen(&colorPenguin));
    head.AddComponent(Component::cuboid((-HEAD_WIDTH/4)*3, 0, -HEAD_DEPTH/2, HEAD_WIDTH/8, HEAD_HEIGHT*0.8,  HEAD_DEPTH/2));
    head.AddComponent(eye.attach(-HEAD_WIDTH/4, HEAD_HEIGHT/2, HEAD_DEPTH/8 + 0.01, false));
    head.AddComponent(Stagnantbeak.attach(-HEAD_WIDTH, HEAD_HEIGHT/2.5, HEAD_DEPTH/8 + 0.01, false)->polyOffset(1.0, 1.0, &colorPenguin));
    head.AddComponent(beak.attach(-HEAD_WIDTH, HEAD_HEIGHT/8, HEAD_DEPTH/8 + 0.01, false)->polyOffset(1.0, 1.0, &colorPenguin));

    //-----------------------------------
    // Right Elbow 
    //-----------------------------------
    Entity rightElbow;
    rightElbow.AddComponent(Component::rotatable(DOFS(Keyframe::USELESS),DOFS(Keyframe::USELESS), DOFS(Keyframe::R_ELBOW)));
    rightElbow.AddComponent(Component::color(0.4, 0.4, 0.4)->onlyWhen(&colorPenguin));
    rightElbow.AddComponent(Component::cuboid((-HEAD_WIDTH/8), -HEAD_HEIGHT*0.5, -HEAD_DEPTH/2, HEAD_WIDTH/8, HEAD_HEIGHT*0.1,  HEAD_DEPTH/2)); 

    //-----------------------------------
    // Right Shoulder  
    //-----------------------------------
    Entity rightShoulder;
    rightShoulder.AddComponent(Component::rotatable(DOFS(Keyframe::R_SHOULDER_ROLL),DOFS(Keyframe::R_SHOULDER_YAW), DOFS(Keyframe::R_SHOULDER_PITCH)));
    rightShoulder.AddComponent(Component::color(0.7, 0.5, 0.3)->onlyWhen(&colorPenguin));
    rightShoulder.AddComponent(Component::cuboid((-HEAD_WIDTH/8), -HEAD_HEIGHT*0.75, -HEAD_DEPTH/2, HEAD_WIDTH/8, HEAD_HEIGHT/3,  HEAD_DEPTH/2)); 
    rightShoulder.AddComponent(rightElbow.attach(0,-HEAD_HEIGHT*0.5,0)->polyOffset(1.0, 1.0, &colorPenguin));

    //-----------------------------------
    // Left Elbow  
    //-----------------------------------
    Entity leftElbow;
    leftElbow.AddComponent(Component::rotatable(DOFS(Keyframe::USELESS),DOFS(Keyframe::USELESS), DOFS(Keyframe::L_ELBOW)));
    leftElbow.AddComponent(Component::color(0.8, 0.8, 0.8)->onlyWhen(&colorPenguin));
    leftElbow.AddComponent(Component::cuboid((-HEAD_WIDTH/8), -HEAD_HEIGHT*0.5, -HEAD_DEPTH/2, HEAD_WIDTH/8, HEAD_HEIGHT*0.1,  HEAD_DEPTH/2)); 

    //-----------------------------------
    // Left Shoulder  
    //-----------------------------------
    Entity leftShoulder;
    leftShoulder.AddComponent(Component::rotatable(DOFS(Keyframe::L_SHOULDER_ROLL),DOFS(Keyframe::L_SHOULDER_YAW), DOFS(Keyframe::L_SHOULDER_PITCH)));
    leftShoulder.AddComponent(Component::color(0.5, 0.7, 0.3)->onlyWhen(&colorPenguin));
    leftShoulder.AddComponent(Component::cuboid((-HEAD_WIDTH/8), -HEAD_HEIGHT*0.75, -HEAD_DEPTH/2, HEAD_WIDTH/8, HEAD_HEIGHT/3,  HEAD_DEPTH/2)); 
    leftShoulder.AddComponent(leftElbow.attach(0,-HEAD_HEIGHT*0.5,0)->polyOffset(1.0, 1.0, &colorPenguin)); // put elbow after rotation so that it follows it 


    //-----------------------------------
    // Right Knee 
    //-----------------------------------
    Entity rightKnee;
    rightKnee.AddComponent(Component::rotatable(DOFS(Keyframe::USELESS),DOFS(Keyframe::USELESS), DOFS(Keyframe::R_KNEE)));
    rightKnee.AddComponent(Component::color(0.4, 0.4, 0.4)->onlyWhen(&colorPenguin));
    rightKnee.AddComponent(Component::cuboid((-HEAD_WIDTH/8), -HEAD_HEIGHT*0.5, -HEAD_DEPTH*0.1, HEAD_WIDTH/8, HEAD_HEIGHT*0.1,  HEAD_DEPTH*0.1)); 

    //-----------------------------------
    // Right Hip  
    //-----------------------------------
    Entity rightHip;
    rightHip.AddComponent(Component::rotatable(DOFS(Keyframe::R_HIP_ROLL),DOFS(Keyframe::R_HIP_YAW), DOFS(Keyframe::R_HIP_PITCH)));
    rightHip.AddComponent(Component::color(0.2, 0.4, 0.6)->onlyWhen(&colorPenguin));
    rightHip.AddComponent(Component::cuboid((-HEAD_WIDTH*0.1), -HEAD_HEIGHT*0.5, -HEAD_DEPTH*0.1, HEAD_WIDTH*0.1, 0,  HEAD_DEPTH*0.1));
    rightHip.AddComponent(rightKnee.attach(0,-HEAD_HEIGHT*0.5,0)->polyOffset(1.0, 1.0, &colorPenguin)); 

    //-----------------------------------
    // Left Knee 
    //-----------------------------------
    Entity leftKnee;
    leftKnee.AddComponent(Component::rotatable(DOFS(Keyframe::USELESS),DOFS(Keyframe::USELESS), DOFS(Keyframe::L_KNEE)));
    leftKnee.AddComponent(Component::color(0.6, 0.3, 0.3)->onlyWhen(&colorPenguin));
    leftKnee.AddComponent(Component::cuboid((-HEAD_WIDTH/8), -HEAD_HEIGHT*0.5, -HEAD_DEPTH*0.1, HEAD_WIDTH/8, HEAD_HEIGHT*0.1,  HEAD_DEPTH*0.1)); 

    //-----------------------------------
    // Left Hip  
    //-----------------------------------
    Entity leftHip;
    leftHip.AddComponent(Component::color(0.4, 0.1, 0.7)->onlyWhen(&colorPenguin));
    leftHip.AddComponent(Component::rotatable(DOFS(Keyframe::L_HIP_ROLL),DOFS(Keyframe::L_HIP_YAW), DOFS(Keyframe::L_HIP_PITCH)));
    leftHip.AddComponent(Component::cuboid((-HEAD_WIDTH*0.1), -HEAD_HEIGHT*0.5, -HEAD_DEPTH*0.1, HEAD_WIDTH*0.1, 0,  HEAD_DEPTH*0.1));
    leftHip.AddComponent(leftKnee.attach(0,-HEAD_HEIGHT*0.5,0)->polyOffset(1.0, 1.0, &colorPenguin));
    //-----------------------------------
    // Body    
    //-----------------------------------
    Entity body;
    body.AddComponent(Component::color(0.5, 1, 0.5)->onlyWhen(&colorPenguin));
    body.AddComponent(Component::cuboid(BODY_WIDTH, BODY_HEIGHT, BODY_DEPTH/2));
    body.AddComponent(head.attach(0, BODY_HEIGHT/2 - PAD, 0));
    body.AddComponent(rightShoulder.attach(0, 0, BODY_DEPTH/2));
    body.AddComponent(leftShoulder.attach(0, 0, -BODY_DEPTH/2));
    body.AddComponent(rightHip.attach(0, -BODY_HEIGHT*0.45, BODY_DEPTH*0.5));
    body.AddComponent(leftHip.attach(0, -BODY_HEIGHT*0.45, -BODY_DEPTH*0.5)); 
    //-----------------------------------
    // Penguin as a whole    
    //-----------------------------------
    // note: PENGUIN is a global variable outside this function. 
    // Root translation and rotation can be controlled by ROOT_* DOFs.
    PENGUIN.AddComponent(Component::translatable(DOFS(Keyframe::ROOT_TRANSLATE_X), DOFS(Keyframe::ROOT_TRANSLATE_Y), DOFS(Keyframe::ROOT_TRANSLATE_Z)));
    PENGUIN.AddComponent(Component::rotatable(DOFS(Keyframe::ROOT_ROTATE_X), DOFS(Keyframe::ROOT_ROTATE_Y), DOFS(Keyframe::ROOT_ROTATE_Z)));
    PENGUIN.AddComponent(body.attach()); // put body after translation and rotation 


//----------------------------------------------------------------------------------------------------------------------------------------------------------------
//Finish Drawing the Penguin and connecting all connections 
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

    // Set up OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glClearColor(0.7f, 0.7f, 0.9f, 1.0f);

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
    keyframes = new Keyframe[KEYFRAME_MAX];
    for( int i = 0; i < KEYFRAME_MAX; i++ )
        keyframes[i].setID(i);

    animationTimer = new Timer();
    frameRateTimer = new Timer();
    joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
    // Init GLUT
    glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutReshapeFunc(reshape);   // Call reshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed
    glutMouseFunc(mouse);       // Call mouse whenever mouse button pressed
    glutMotionFunc(motion);     // Call motion whenever mouse moves while button pressed
}


// Load Keyframe button handler. Called when the "load keyframe" button is pressed
void loadKeyframeButton(int)
{
    // Get the keyframe ID from the UI
    int keyframeID = joint_ui_data->getID();

    // Update the 'joint_ui_data' variable with the appropriate
    // entry from the 'keyframes' array (the list of keyframes)
    *joint_ui_data = keyframes[keyframeID];

    // Sync the UI with the 'joint_ui_data' values
    glui_joints->sync_live();
    glui_keyframe->sync_live();

    // Let the user know the values have been loaded
    sprintf(msg, "Status: Keyframe %d loaded successfully", keyframeID);
    status->set_text(msg);
}

// Update Keyframe button handler. Called when the "update keyframe" button is pressed
void updateKeyframeButton(int)
{
    ///////////////////////////////////////////////////////////
    //   This function saves the UI joint values into
    //   the appropriate keyframe entry in the keyframe list
    //   when the user clicks on the 'Update Keyframe' button.
    //   Refer to the 'loadKeyframeButton' function for help.
    ///////////////////////////////////////////////////////////

    // Get the keyframe ID from the UI
    int keyframeID = (*joint_ui_data).getID();

    // Update the 'maxValidKeyframe' index variable
    // (it will be needed when doing the interpolation)
    maxValidKeyframe = keyframeID;

    // Update the appropriate entry in the 'keyframes' array
    // with the 'joint_ui_data' data
    keyframes[keyframeID] = (*joint_ui_data);

    // Let the user know the values have been updated
    sprintf(msg, "Status: Keyframe %d updated successfully", keyframeID);
    status->set_text(msg);

/////////////////////
}

// Load Keyframes From File button handler. Called when the "load keyframes from file" button is pressed
//
// ASSUMES THAT THE FILE FORMAT IS CORRECT, ie, there is no error checking!
//
void loadKeyframesFromFileButton(int)
{
    // Open file for reading
    FILE* file = fopen(filenameKF, "r");
    if( file == NULL )
    {
        sprintf(msg, "Status: Failed to open file %s", filenameKF);
        status->set_text(msg);
        return;
    }

    // Read in maxValidKeyframe first
    fscanf(file, "%d", &maxValidKeyframe);

    // Now read in all keyframes in the format:
    //    id
    //    time
    //    DOFs
    //
    for( int i = 0; i <= maxValidKeyframe; i++ )
    {
        fscanf(file, "%d", keyframes[i].getIDPtr());
        fscanf(file, "%f", keyframes[i].getTimePtr());

        for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
            fscanf(file, "%f", keyframes[i].getDOFPtr(j));
    }

    // Close file
    fclose(file);

    // Let the user know the keyframes have been loaded
    sprintf(msg, "Status: Keyframes loaded successfully");
    status->set_text(msg);
}

// Save Keyframes To File button handler. Called when the "save keyframes to file" button is pressed
void saveKeyframesToFileButton(int)
{
    // Open file for writing
    FILE* file = fopen(filenameKF, "w");
    if( file == NULL )
    {
        sprintf(msg, "Status: Failed to open file %s", filenameKF);
        status->set_text(msg);
        return;
    }

    // Write out maxValidKeyframe first
    fprintf(file, "%d\n", maxValidKeyframe);
    fprintf(file, "\n");

    // Now write out all keyframes in the format:
    //    id
    //    time
    //    DOFs
    //
    for( int i = 0; i <= maxValidKeyframe; i++ )
    {
        fprintf(file, "%d\n", keyframes[i].getID());
        fprintf(file, "%f\n", keyframes[i].getTime());

        for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
            fprintf(file, "%f\n", keyframes[i].getDOF(j));

        fprintf(file, "\n");
    }

    // Close file
    fclose(file);

    // Let the user know the keyframes have been saved
    sprintf(msg, "Status: Keyframes saved successfully");
    status->set_text(msg);
}

// Animate button handler.  Called when the "animate" button is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 )
  {
    // start animation
    frameRateTimer->reset();
    animationTimer->reset();

    animate_mode = 1;
    GLUI_Master.set_glutIdleFunc(animate);

    // Let the user know the animation is running
    sprintf(msg, "Status: Animating...");
    status->set_text(msg);
  }
  else
  {
    // stop animation
    animate_mode = 0;
    GLUI_Master.set_glutIdleFunc(NULL);

    // Let the user know the animation has stopped
    sprintf(msg, "Status: Animation stopped");
    status->set_text(msg);
  }
}

// Render Frames To File button handler. Called when the "Render Frames To File" button is pressed.
void renderFramesToFileButton(int)
{
    // Calculate number of frames to generate based on dump frame rate
    int numFrames = int(keyframes[maxValidKeyframe].getTime() * DUMP_FRAME_PER_SEC) + 1;

    // Generate frames and save to file
    frameToFile = 1;
    for( frameNumber = 0; frameNumber < numFrames; frameNumber++ )
    {
        // Get the interpolated joint DOFs
        joint_ui_data->setDOFVector( getInterpolatedJointDOFS(frameNumber * DUMP_SEC_PER_FRAME) );

        // Let the user know which frame is being rendered
        sprintf(msg, "Status: Rendering frame %d...", frameNumber);
        status->set_text(msg);

        // Render the frame
        display();
    }
    frameToFile = 0;

    // Let the user know how many frames were generated
    sprintf(msg, "Status: %d frame(s) rendered to file", numFrames);
    status->set_text(msg);
}

// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// To change the rendering style for METAL and MATTE
void changeRender(int) 
{
    switch (renderStyle) {
        case METAL:
        case MATTE:
            glui_light->show();
            break;
        default:
            glui_light->hide();
            break;
    }
}


// Initialize GLUI and the user interface
void initGlui()
{
    GLUI_Panel* glui_panel;
    GLUI_Spinner* glui_spinner;
    GLUI_RadioGroup* glui_radio_group;

    GLUI_Master.set_glutIdleFunc(NULL);


    // Create GLUI window (joint controls) ***************
    //
    glui_joints = GLUI_Master.create_glui("Joint Control", 0, Win[0]+12, 0);

    //--------------------------------------------------------------------------
    // 1, 2, 3,4,5,6th DOF (Global orientation and translation of penguin) 
    //--------------------------------------------------------------------------
    // Create controls to specify root position and orientation
    glui_panel = glui_joints->add_panel("Root");

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_X));
    glui_spinner->set_float_limits(ROOT_TRANSLATE_X_MIN, ROOT_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED/15);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Y));
    glui_spinner->set_float_limits(ROOT_TRANSLATE_Y_MIN, ROOT_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED/15);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Z));
    glui_spinner->set_float_limits(ROOT_TRANSLATE_Z_MIN, ROOT_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED/15);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_X));
    glui_spinner->set_float_limits(ROOT_ROTATE_X_MIN, ROOT_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
    glui_spinner->set_speed(SPINNER_SPEED/15);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Y));
    glui_spinner->set_float_limits(ROOT_ROTATE_Y_MIN, ROOT_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
    glui_spinner->set_speed(SPINNER_SPEED/15);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Z));
    glui_spinner->set_float_limits(ROOT_ROTATE_Z_MIN, ROOT_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
    glui_spinner->set_speed(SPINNER_SPEED/15);

    //--------------------------------------------------------------------------
    // 7th DOF for head rotation 
    //--------------------------------------------------------------------------
    // Create controls to specify head rotation
    glui_panel = glui_joints->add_panel("Head");

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "head:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::HEAD));
    glui_spinner->set_float_limits(HEAD_MIN, HEAD_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    //--------------------------------------------------------------------------
    // 8 DOF for beak rotation 
    //--------------------------------------------------------------------------
    // Create controls to specify beak
    glui_panel = glui_joints->add_panel("Beak");

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "beak:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BEAK));
    glui_spinner->set_float_limits(BEAK_MIN, BEAK_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);


    glui_joints->add_column(false);


    //--------------------------------------------------------------------------------------------
    //  9, 10, 11,12, 13, 14, 15, 16th DOF for right arm & left arm  rotation and elbow movement
    //--------------------------------------------------------------------------------------------
    // Create controls to specify right arm
    glui_panel = glui_joints->add_panel("Right arm");

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_PITCH));
    glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_YAW));
    glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_ROLL));
    glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_ELBOW));
    glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    // Create controls to specify left arm
    glui_panel = glui_joints->add_panel("Left arm");

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_PITCH));
    glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_YAW));
    glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_ROLL));
    glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_ELBOW));
    glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);


    glui_joints->add_column(false);

    //---------------------------------------------------------------------------------------------
    // 17,18, 19, 20, 21, 22, 23, 24th DOF for right leg & left leg  rotation, and knee movement 
    //---------------------------------------------------------------------------------------------
    // Create controls to specify right leg
    glui_panel = glui_joints->add_panel("Right leg");

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_PITCH));
    glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_YAW));
    glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_ROLL));
    glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE));
    glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    // Create controls to specify left leg
    glui_panel = glui_joints->add_panel("Left leg");

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_PITCH));
    glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_YAW));
    glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_ROLL));
    glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE));
    glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);


    ///////////////////////////////////////////////////////////
    //  (for controlling light source position & additional
    //      rendering styles):
    //   Add more UI spinner elements here. Be sure to also
    //   add the appropriate min/max range values to this
    //   file, and to also add the appropriate enums to the
    //   enumeration in the Keyframe class (keyframe.h).
    ///////////////////////////////////////////////////////////
    // Add light source and shading
    glui_light = GLUI_Master.create_glui("Light and Shading", 0, Win[0] + 10, Win[1] + 10);

    glui_panel = glui_light->add_panel("Light");
    glui_spinner = glui_light->add_spinner_to_panel(glui_panel, "position:", GLUI_SPINNER_FLOAT, &light_angle);
    glui_spinner->set_float_limits(LIGHT_POS_MIN, LIGHT_POS_MAX, GLUI_LIMIT_WRAP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_light->add_checkbox_to_panel(glui_panel, "Colored Materials", &coloredMaterials);

    glui_light->add_column(false);

    glui_panel = glui_light->add_panel("Shade Model");
    glui_radio_group = glui_light->add_radiogroup_to_panel(glui_panel, &shadeModel);
    glui_light->add_radiobutton_to_group(glui_radio_group, "Flat");
    glui_light->add_radiobutton_to_group(glui_radio_group, "Smooth");

    glui_light->hide();
    //
    // ***************************************************
    // Create GLUI window (keyframe controls) ************
    //
    glui_keyframe = GLUI_Master.create_glui("Keyframe Control", 0, 0, Win[1]+64);

    // Create a control to specify the time (for setting a keyframe)
    glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
    glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Time:", GLUI_SPINNER_FLOAT, joint_ui_data->getTimePtr());
    glui_spinner->set_float_limits(TIME_MIN, TIME_MAX, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    // Create a control to specify a keyframe (for updating / loading a keyframe)
    glui_keyframe->add_column_to_panel(glui_panel, false);
    glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Keyframe ID:", GLUI_SPINNER_INT, joint_ui_data->getIDPtr());
    glui_spinner->set_int_limits(KEYFRAME_MIN, KEYFRAME_MAX-1, GLUI_LIMIT_CLAMP);
    glui_spinner->set_speed(SPINNER_SPEED);

    glui_keyframe->add_separator();

    // Add buttons to load and update keyframes
    // Add buttons to load and save keyframes from a file
    // Add buttons to start / stop animation and to render frames to file
    glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
    glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframe", 0, loadKeyframeButton);
    glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframes From File", 0, loadKeyframesFromFileButton);
    glui_keyframe->add_button_to_panel(glui_panel, "Start / Stop Animation", 0, animateButton);
    glui_keyframe->add_column_to_panel(glui_panel, false);
    glui_keyframe->add_button_to_panel(glui_panel, "Update Keyframe", 0, updateKeyframeButton);
    glui_keyframe->add_button_to_panel(glui_panel, "Save Keyframes To File", 0, saveKeyframesToFileButton);
    glui_keyframe->add_button_to_panel(glui_panel, "Render Frames To File", 0, renderFramesToFileButton);

    glui_keyframe->add_separator();

    // Add status line
    glui_panel = glui_keyframe->add_panel("");
    status = glui_keyframe->add_statictext_to_panel(glui_panel, "Status: Ready");

    // Add button to quit
    glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
    glui_keyframe->add_button_to_panel(glui_panel, "Quit", 0, quitButton);
    //
    // ***************************************************


    // Create GLUI window (render controls) ************
    //
    glui_render = GLUI_Master.create_glui("Render Control", 0, 367, Win[1]+64);

    // Create control to specify the render style
    glui_panel = glui_render->add_panel("Render Style");
    // Add light buttons 
    glui_radio_group = glui_render->add_radiogroup_to_panel(glui_panel, &renderStyle, -1,changeRender);
    glui_render->add_radiobutton_to_group(glui_radio_group, "Wireframe");
    glui_render->add_radiobutton_to_group(glui_radio_group, "Solid");
    glui_render->add_radiobutton_to_group(glui_radio_group, "Solid w/ outlines");
    glui_render->add_radiobutton_to_group(glui_radio_group, "Metal");
    glui_render->add_radiobutton_to_group(glui_radio_group, "Matte");
    //
    // ***************************************************
    // Tell GLUI windows which window is main graphics window
    glui_light->set_main_gfx_window(windowID);
    glui_joints->set_main_gfx_window(windowID);
    glui_keyframe->set_main_gfx_window(windowID);
    glui_render->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
    // Need to find the keyframes bewteen which
    // the supplied time lies.
    // At the end of the loop we have:
    //    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
    //
    int i = 0;
    while( i <= maxValidKeyframe && keyframes[i].getTime() < time )
        i++;

    // If time is before or at first defined keyframe, then
    // just use first keyframe pose
    if( i == 0 )
        return keyframes[0].getDOFVector();

    // If time is beyond last defined keyframe, then just
    // use last keyframe pose
    if( i > maxValidKeyframe )
        return keyframes[maxValidKeyframe].getDOFVector();

    // Need to normalize time to (0, 1]
    time = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

    // Get appropriate data points and tangent vectors
    // for computing the interpolation
    Vector p0 = keyframes[i-1].getDOFVector();
    Vector p1 = keyframes[i].getDOFVector();

    Vector t0, t1;
    if( i == 1 )                            // special case - at beginning of spline
    {
        t0 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
        t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
    }
    else if( i == maxValidKeyframe )        // special case - at end of spline
    {
        t0 = (keyframes[i].getDOFVector() - keyframes[i-2].getDOFVector()) * 0.5;
        t1 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
    }
    else
    {
        t0 = (keyframes[i].getDOFVector()   - keyframes[i-2].getDOFVector()) * 0.5;
        t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
    }

    // Return the interpolated Vector
    Vector a0 = p0;
    Vector a1 = t0;
    Vector a2 = p0 * (-3) + p1 * 3 + t0 * (-2) + t1 * (-1);
    Vector a3 = p0 * 2 + p1 * (-2) + t0 + t1;

    return (((a3 * time + a2) * time + a1) * time + a0);
}


// Callback idle function for animating the scene
void animate()
{
    // Only update if enough time has passed
    // (This locks the display to a certain frame rate rather
    //  than updating as fast as possible. The effect is that
    //  the animation should run at about the same rate
    //  whether being run on a fast machine or slow machine)
    if( frameRateTimer->elapsed() > SEC_PER_FRAME )
    {
        // Tell glut window to update itself. This will cause the display()
        // callback to be called, which renders the object (once you've written
        // the callback).
        glutSetWindow(windowID);
        glutPostRedisplay();

        // Restart the frame rate timer
        // for the next frame
        frameRateTimer->reset();
    }
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
    // Update internal variables and OpenGL viewport
    Win[0] = w;
    Win[1] = h;
    glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // View with perspective 
    gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}



// display callback
//
// README: This gets called by the event handler
// to draw the scene, so this is where you need
// to build your scene -- make your changes and
// additions here. All rendering happens in this
// function. For Assignment 2, updates to the
// joint DOFs (joint_ui_data) happen in the
// animate() function.
void display(void)
{
    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Specify camera transformation
    glTranslatef(camXPos, camYPos, camZPos);


    // Get the time for the current animation step, if necessary
    if( animate_mode )
    {
        float curTime = animationTimer->elapsed();

        if( curTime >= keyframes[maxValidKeyframe].getTime() )
        {
            // Restart the animation
            animationTimer->reset();
            curTime = animationTimer->elapsed();
        }

        ///////////////////////////////////////////////////////////
        // README:
        //   This statement loads the interpolated joint DOF vector
        //   into the global 'joint_ui_data' variable. Use the
        //   'joint_ui_data' variable below in your model code to
        //   drive the model for animation.
        ///////////////////////////////////////////////////////////
        // Get the interpolated joint DOFs
        joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

        // Update user interface
        joint_ui_data->setTime(curTime);
        glui_keyframe->sync_live();
    }

//--------------------------------------------------------------------------------
 
    ///////////////////////////////////////////////////////////
    //
    //   Modify this function to draw the scene.
    //   This should include function calls that apply
    //   the appropriate transformation matrices and render
    //   the individual body parts.
    //   Use the 'joint_ui_data' data structure to obtain
    //   the joint DOFs to specify your transformations.
    //   Sample code is provided below and demonstrates how
    //   to access the joint DOF values. This sample code
    //   should be replaced with your own.
    //   Use the 'renderStyle' variable and the associated
    //   enumeration to determine how the geometry should be
    //   rendered.
    ///////////////////////////////////////////////////////////

    glPushMatrix();

    float light_pos[] = {LIGHT_CIRCLE_RADIUS * cosf(deg2rad(light_angle)),
                         LIGHT_CIRCLE_RADIUS * sinf(deg2rad(light_angle)),
                         25, 0};

    const float METAL_SPECULAR[] = { 0.70, 0.70, 0.70, 1.0 };
    const float METAL_DIFFUSE[]  = { 0.50, 0.50, 0.50, 1.0 };
    const float METAL_SHININESS  = 128;

    const float MATTE_SPECULAR[] = { 0.01, 0.01, 0.01, 1.0 };
    const float MATTE_DIFFUSE[]  = { 0.50, 0.50, 0.50, 1.0 };
    const float MATTE_SHININESS  = 0;

    const float LIGHT_SPECULAR[] = { 0.8, 0.8, 0.8, 1.0 };

    Component *penguin = 0;

    // determine render style and set glPolygonMode appropriately
    switch (renderStyle) 
    {
        case WIREFRAME:
	{
	    Wrapper* aNew = &(PENGUIN.wrap());
	    aNew->AddPrev(ENABLE_COLOR_PENGUIN);
	    aNew->AddPrev(&wireFrameMode);
            penguin = aNew; 
            break;
	}
        case SOLID:
	{
	    Wrapper* aNew = &(PENGUIN.wrap());
	    aNew->AddPrev(ENABLE_COLOR_PENGUIN);
	    aNew->AddPrev(&solidMode);
            penguin = aNew; 
            break;
	}
        case OUTLINED:
	{
	    Wrapper* aNew = &(PENGUIN.wrap());
	    aNew->AddPrev(ENABLE_COLOR_PENGUIN);
	    aNew->AddPrev(&solidMode);
            penguin = aNew; 
	    Wrapper* bNew = &(PENGUIN.wrap());
	    bNew->AddPrev(penguin);
	    bNew->AddPrev(DISABLE_COLOR_PENGUIN);
	    bNew->AddPrev(Component::color(0, 0, 0));
	    bNew->AddPrev(&wireFrameMode);
	    bNew->AddPrev(Component::polygonOffset(1.0, 2.0));
	    bNew->AddNext(ENABLE_COLOR_PENGUIN);
	    bNew->enableDisable(GL_POLYGON_OFFSET_FILL)->pushPopAttribute(GL_COLOR_BUFFER_BIT);
	    penguin = bNew; 
            break;
	}
        case METAL:
	{
            glShadeModel(shadeModel == SHADE_FLAT ? GL_FLAT : GL_SMOOTH);
	    Wrapper* aNew = &(PENGUIN.wrap());
	    aNew->AddPrev(Component::polygonMode(GL_FRONT_AND_BACK, GL_FILL));
	    aNew->AddPrev(Component::light(GL_LIGHT0, GL_POSITION, light_pos));
	    aNew->AddPrev(Component::light(GL_LIGHT0, GL_SPECULAR, LIGHT_SPECULAR));
	    aNew->AddPrev(Component::material(GL_FRONT, GL_SPECULAR, METAL_SPECULAR));
	    aNew->AddPrev(Component::material(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, METAL_DIFFUSE));
	    aNew->AddPrev(Component::material(GL_FRONT, GL_SHININESS, METAL_SHININESS));
	    aNew->enableDisable(GL_LIGHTING)->enableDisable(GL_LIGHT0);
	    penguin = aNew; 
            if (coloredMaterials)
                penguin = penguin->enableDisable(GL_COLOR_MATERIAL);
            break;
	}
        case MATTE:
	{
            glShadeModel(shadeModel == SHADE_FLAT ? GL_FLAT : GL_SMOOTH);
	    Wrapper* aNew = &(PENGUIN.wrap());
	    aNew->AddPrev(Component::polygonMode(GL_FRONT_AND_BACK, GL_FILL));
	    aNew->AddPrev(Component::light(GL_LIGHT0, GL_POSITION, light_pos));
	    aNew->AddPrev(Component::light(GL_LIGHT0, GL_SPECULAR, LIGHT_SPECULAR));
	    aNew->AddPrev(Component::material(GL_FRONT, GL_SPECULAR, MATTE_SPECULAR));
	    aNew->AddPrev(Component::material(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, MATTE_DIFFUSE));
	    aNew->AddPrev(Component::material(GL_FRONT, GL_SHININESS, MATTE_SHININESS));
	    aNew->enableDisable(GL_LIGHTING)->enableDisable(GL_LIGHT0);
	    penguin = aNew; 
            if (coloredMaterials)
                penguin = penguin->enableDisable(GL_COLOR_MATERIAL);
            break;
	}
    }
    penguin->Update();
//--------------------------------------------------------------------------------
    glPopMatrix();

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Dump frame to file, if requested
    if( frameToFile )
    {
        sprintf(filenameF, "frame%03d.ppm", frameNumber);
        writeFrame(filenameF, false, false);
    }

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Handles mouse button pressed / released events
void mouse(int button, int state, int x, int y)
{
    // If the RMB is pressed and dragged then zoom in / out
    if( button == GLUT_RIGHT_BUTTON )
    {
        if( state == GLUT_DOWN )
        {
            lastX = x;
            lastY = y;
            updateCamZPos = true;
        }
        else
        {
            updateCamZPos = false;
        }
    }
}


// Handles mouse motion events while a button is pressed
void motion(int x, int y)
{
    // If the RMB is pressed and dragged then zoom in / out
    if( updateCamZPos )
    {
        // Update camera z position
        camZPos += (x - lastX) * ZOOM_SCALE;
        lastX = x;

        // Redraw the scene from updated camera position
        glutSetWindow(windowID);
        glutPostRedisplay();
    }
}

/*
// Draw a unit cube, centered at the current location
// README: Helper code for drawing a cube
void drawCube()
{
#if 1
    glBegin(GL_QUADS);
        // draw front face
        glVertex3f(-1.0, -1.0, 1.0);
        glVertex3f( 1.0, -1.0, 1.0);
        glVertex3f( 1.0,  1.0, 1.0);
        glVertex3f(-1.0,  1.0, 1.0);

        // draw back face
        glVertex3f( 1.0, -1.0, -1.0);
        glVertex3f(-1.0, -1.0, -1.0);
        glVertex3f(-1.0,  1.0, -1.0);
        glVertex3f( 1.0,  1.0, -1.0);

        // draw left face
        glVertex3f(-1.0, -1.0, -1.0);
        glVertex3f(-1.0, -1.0,  1.0);
        glVertex3f(-1.0,  1.0,  1.0);
        glVertex3f(-1.0,  1.0, -1.0);

        // draw right face
        glVertex3f( 1.0, -1.0,  1.0);
        glVertex3f( 1.0, -1.0, -1.0);
        glVertex3f( 1.0,  1.0, -1.0);
        glVertex3f( 1.0,  1.0,  1.0);

        // draw top
        glVertex3f(-1.0,  1.0,  1.0);
        glVertex3f( 1.0,  1.0,  1.0);
        glVertex3f( 1.0,  1.0, -1.0);
        glVertex3f(-1.0,  1.0, -1.0);

        // draw bottom
        glVertex3f(-1.0, -1.0, -1.0);
        glVertex3f( 1.0, -1.0, -1.0);
        glVertex3f( 1.0, -1.0,  1.0);
        glVertex3f(-1.0, -1.0,  1.0);
    glEnd();
 #else
    GLuint VAOs[1];
    glGenVertexArrays(1, VAOs);
    glBindVertexArray(VAOs[0]);

    const int NUM_VERTICES = 36;
    GLfloat  vertices[NUM_VERTICES][3] = {
        { -1, -1, -1 },
        {  1, -1, -1 },
        {  1,  1, -1 },

        { -1, -1, -1 },
        {  1,  1, -1 },
        { -1,  1, -1 },

        {  1, -1, -1 },
        {  1, -1,  1 },
        {  1,  1,  1 },

        {  1, -1, -1 },
        {  1,  1,  1 },
        {  1,  1, -1 },

        {  1, -1,  1 },
        { -1, -1,  1 },
        { -1,  1,  1 },

        {  1, -1,  1 },
        { -1,  1,  1 },
        {  1,  1,  1 },

        { -1, -1,  1 },
        { -1, -1, -1 },
        { -1,  1, -1 },

        { -1, -1, -1 },
        { -1,  1, -1 },
        { -1,  1,  1 },

        { -1,  1, -1 },
        {  1,  1, -1 },
        {  1,  1,  1 },

        { -1,  1, -1 },
        {  1,  1,  1 },
        { -1,  1,  1 },

        {  1, -1, -1 },
        { -1, -1, -1 },
        { -1, -1,  1 },

        {  1, -1, -1 },
        { -1, -1,  1 },
        {  1, -1,  1 },
    };

    GLuint Buffers[1];
    glGenBuffers(1, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                 vertices, GL_STATIC_DRAW);

    ShaderInfo  shaders[] = {
        { GL_VERTEX_SHADER, "triangles.vert" },
        { GL_FRAGMENT_SHADER, "triangles.frag" },
        { GL_GEOMETRY_SHADER, "triangles.geom" },
        { GL_NONE, NULL }
    };

    GLuint program = LoadShaders(shaders);
    glUseProgram(program);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);
#endif
}*/

///////////////////////////////////////////////////////////
//
// BELOW ARE FUNCTIONS FOR GENERATING IMAGE FILES (PPM/PGM)
//
///////////////////////////////////////////////////////////

void writePGM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
    FILE* fp = fopen(filename,"wt");

    if( fp == NULL )
    {
        printf("WARNING: Can't open output file %s\n",filename);
        return;
    }

    if( raw )
    {
        fprintf(fp,"P5\n%d %d\n%d\n",width,height,255);
        for(int y=height-1;y>=0;y--)
        {
            fwrite(&buffer[y*width],sizeof(GLubyte),width,fp);
            /*
            for(int x=0;x<width;x++)
            {
                fprintf(fp,"%c",int(buffer[x+y*width];
            }
            */
        }
    }
    else
    {
        fprintf(fp,"P2\n%d %d\n%d\n",width,height,255);
        for(int y=height-1;y>=0;y--)
        {
            for(int x=0;x<width;x++)
            {
                fprintf(fp,"%p ", &buffer[x+y*width]);
            }
            fprintf(fp,"\n");
        }
    }

    fclose(fp);
}

#define RED_OFFSET   0
#define GREEN_OFFSET 1
#define BLUE_OFFSET  2

void writePPM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
    FILE* fp = fopen(filename,"wt");

    if( fp == NULL )
    {
        printf("WARNING: Can't open output file %s\n",filename);
        return;
    }

    if( raw )
    {
        fprintf(fp,"P6\n%d %d\n%d\n",width,height,255);
        for(int y=height-1;y>=0;y--)
        {
            for(int x=0;x<width;x++)
            {
                GLubyte* pix = &buffer[4*(x+y*width)];

                fprintf(fp,"%c%c%c",int(pix[RED_OFFSET]),
                                    int(pix[GREEN_OFFSET]),
                                    int(pix[BLUE_OFFSET]));
            }
        }
    }
    else
    {
        fprintf(fp,"P3\n%d %d\n%d\n",width,height,255);
        for(int y=height-1;y>=0;y--)
        {
            for(int x=0;x<width;x++)
            {
                GLubyte* pix = &buffer[4*(x+y*width)];

                fprintf(fp,"%d %d %d ",int(pix[RED_OFFSET]),
                                       int(pix[GREEN_OFFSET]),
                                       int(pix[BLUE_OFFSET]));
            }
            fprintf(fp,"\n");
        }
    }

    fclose(fp);
}

void writeFrame(char* filename, bool pgm, bool frontBuffer)
{
    static GLubyte* frameData = NULL;
    static int currentSize = -1;

    int size = (pgm ? 1 : 4);

    if( frameData == NULL || currentSize != size*Win[0]*Win[1] )
    {
        if (frameData != NULL)
            delete [] frameData;

        currentSize = size*Win[0]*Win[1];

        frameData = new GLubyte[currentSize];
    }

    glReadBuffer(frontBuffer ? GL_FRONT : GL_BACK);

    if( pgm )
    {
        glReadPixels(0, 0, Win[0], Win[1],
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData);
        writePGM(filename, frameData, Win[0], Win[1]);
    }
    else
    {
        glReadPixels(0, 0, Win[0], Win[1],
                     GL_RGBA, GL_UNSIGNED_BYTE, frameData);
        writePPM(filename, frameData, Win[0], Win[1]);
    }
}
