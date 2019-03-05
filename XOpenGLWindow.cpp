// This program creates a basic X window (using Xlib) and initializes OpenGL context for the same.
#include <iostream> // Anticipated to use C++ later (not used in this program)
#include <stdio.h> // for printf()
#include <stdlib.h> // for exit()
#include <memory.h> // for memset()

#include <X11/Xlib.h> // for native windowing
#include <X11/Xutil.h> // For the Visual and related api
#include <X11/XKBlib.h> // for keyboard api - XkbKeycodeToKeysym()

#include <GL/gl.h>
#include <GL/glx.h> // for bridging-api
#include <GL/glu.h> // for OpenGL

//namespaces
using namespace std; // Anticipated to use C++ later (not used in this program)

// global variable declarations
bool bFullscreen = false; 
int giWindowWidth = 800;
int giWindowHeight = 600;
Display *gpDisplay = NULL; // Struct Display is the handle of the X client for the requests to the X Server.
XVisualInfo *gpXVisualInfo=NULL; // Struct XVisualInfo stores information about the visual (Read note in the code below for details)
Colormap gColormap; // color palette
Window gWindow;
GLXContext gGLXContext;
// entry point function
int main(void) // fyi, command line arguments can be used to give ip address of remote client
{
	// function prototypes
	void createWindow(void);
	void toggleFullscreen(void);
	void initialize(void);
	void display(void);
	void resize(int, int);
	void uninitialize();

	// variable declarations
	int winWidth = giWindowWidth; // local val of windowWidth will be changed later on
	int winHeight = giWindowHeight; // local val of windowHeight will be changed later on

	bool bDone = false; // this flag will be used while closing the window

	// code
	createWindow(); // Combines functionality of createWindow(...), registerWindow(...), showWindow(...), updateWindow(...)

	// initialize
	initialize();

	// Message loop
	XEvent event;
	KeySym keysym;

	while(bDone==false){
		
		while(XPending(gpDisplay)){ // XPending() func. returns the number of events that have been received from the X server but have not been removed from the event queue. gpDisplay - connection to the X server.
		
			XNextEvent(gpDisplay, &event); // XNextEvent() - Copies the first event from the event queue into the specified 'event' obj and then removes it from the event queue.
			switch(event.type){

				case MapNotify: // Similar to WM_CREATE. This event is generated when the window changes state from unmapped to mapped
					break;
				case KeyPress: keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode,0,0); // event.xkey.keycode-> keycode, 0 -> locale (0=default=english), 0-> Shift not used
				 	switch(keysym){
						case XK_Escape: // XK - X Keycode symbol
							uninitialize();
							exit(0);
							   //OR
							//bDone = true;
						case XK_F:
						case XK_f:
							if(bFullscreen==false){
								toggleFullscreen();
								bFullscreen=true;
							}
							else{
							toggleFullscreen();
							bFullscreen=false;								
							}
							break;
						default:
							break;						
					}
					break;
				case ButtonPress: // Mouse button events
				     switch(event.xbutton.button){
					case 1:	// similar to WM_LBUTTONDOWN
						break;
					case 2:	// similar to WM_MBUTTONDOWN
						break;
					case 3:	// similar to WM_RBUTTONDOWN
						break;
					case 4:	// similar to MOUSE_WHEELUP
						break;
					case 5: // similar to MOUSE_WHEELDOWN
						break;
				     }
				     break;
	 			case MotionNotify: // WM_MOUSEMOVE
					break;
				case ConfigureNotify: // similar to WM_RESIZE
					winWidth = event.xconfigure.width;
					winHeight= event.xconfigure.height;
					break;
				case Expose: // similar to WM_PAINT
					break;
				case DestroyNotify: // similar to WM_DESTROY
					break;
				case 33: // handles click on 'Close' box, sys. menu 'Close' (this number never changes).
					// Note:- Preparation for this case has been done in createWindow(...) by creating Atom and setting WM protocols.
					uninitialize();
					exit(0);
				default:
					break;
			}			
			
			

		}

		display(); // Rendering is done here (acts like 'else' block of Gameloop).	
		
	} // END: while(bDone==false)
	
	uninitialize();
	return (0); // Technically, we never reach till this point during execution. We exit from case 33.
}

void display(void){

	// code
	glClear(GL_COLOR_BUFFER_BIT); // glClear() - clear buffers to preset values. GL_COLOR_BUFFER_BIT - Indicates the buffers currently enabled for writing.

	glFlush(); // Empty the buffer
}

void resize(int width, int height){
	
	//code
	if(height==0)
		height=1;
	glViewport(0,0,(GLsizei) width, (GLsizei) height); // set the viewport
}

void uninitialize(void){ // works like a destructor i.e. destroy in the reverse order of creation
	
	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();

	if((currentGLXContext!=NULL) && currentGLXContext==gGLXContext){
		glXMakeCurrent(gpDisplay,0,0);
	}
	
	if(gGLXContext){
		glXDestroyContext(gpDisplay, gGLXContext);
	}

	if(gWindow){
		XDestroyWindow(gpDisplay, gWindow);
	}

	if(gColormap){
		XFreeColormap(gpDisplay, gColormap);
	}

	if(gpXVisualInfo){
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}

	if(gpDisplay){
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
}


void toggleFullscreen(void){

	//variable declarations
	Atom wm_state;
	Atom fullscreen;
	XEvent xev = {0};

	// code
	// Get the current placement of the window and save it in an atom
	wm_state = XInternAtom(gpDisplay,"_NET_WM_STATE",False); // '_NET_WM_STATE' - Network compliant, False - don't create if this atom exists.
	memset(&xev,0,sizeof(xev));

	// create a custom (user-defined) event/message
	xev.type=ClientMessage;
	xev.xclient.window = gWindow; // xclient is a sub-structure in the XEvent;
	xev.xclient.message_type=wm_state; // message - atom created above
	xev.xclient.format=32; // 32-bit 
	// Check the value present in 'l[0]' and perform the corresponding action in 'l[1]'
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1; // "data" is a union in XEvent and "l" is the sub-union in it.

	// create an atom for fullscreen
	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False); // '_NET_WM_STATE_FULLSCREEN' - atom related to fullscreen, False - don't create if this atom already exists.
	xev.xclient.data.l[1] = fullscreen;// add 'fullscreen' atom to l[1]

	// Send the event to the queue
	XSendEvent(gpDisplay,
		   RootWindow(gpDisplay, gpXVisualInfo->screen), // Propogate the message to this window.
		   False, // Do not propogate the message to the child window
		   StructureNotifyMask, // similar to WM_SIZE
		   &xev); // custom-event created above.
}

void initialize(void){

	//function prototype
	void resize(int, int);
	
	// code
	// Step 1: Create an OpenGL context
	gGLXContext = glXCreateContext(gpDisplay,gpXVisualInfo, NULL, GL_TRUE); // Null - this is not a shared context, GL_True - hardware rendering
	
	// Step 2: Make the 'gGLXContext' (created above) as the current context
	glXMakeCurrent(gpDisplay,gWindow, gGLXContext);

	glClearColor(0.0f,0.0f,1.0f,0.0f);

	resize(giWindowWidth, giWindowHeight);
}

void createWindow(void) // Note:- all the programs which create their own window are XClients of the XServer (i.e. O.S)
{
	// function prototypes
	void uninitialize (void);
	
	// variable declarations
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int styleMask;

	static int frameBufferAttributes[] = {
		GLX_RGBA, // consider only TrueColor and DirectColor visuals (otherwise PseudoColor and StaticColor visuals are considered).
		GLX_RED_SIZE, 1, // The returned visual should support a buffer (1-bit atleast) of red color.
		GLX_GREEN_SIZE, 1, // The returned visual should support a buffer (1-bit atleast) of green color.
		GLX_BLUE_SIZE, 1, // The returned visual should support a buffer (1-bit atleast) of blue color.
		GLX_ALPHA_SIZE, 1, // The returned visual should support a buffer (1-bit atleast) of alpha.
		None // frameBufferAttribute array must be terminated by 0 and hence 'None' is used.
};
	// code
	// Step 1: Get the connection of the local display
	gpDisplay = XOpenDisplay(NULL); // NULL - Gives default local connection.
	if(gpDisplay == NULL){
		printf("ERROR: Unable to open X-display. \nExiting now...");
		uninitialize();
		exit(1); // Abortive exit and hence a +ve number.
	}

	// Step 2: Get the default monitor/screen (from "Display" struct) to which the graphic card is connected.
	defaultScreen = XDefaultScreen(gpDisplay);
	
	// Step 3: Get the XVisualInfo structure that best meets the minimum requirements. 
	// Note:- A single display can support multiple screens. Each screen can have several different visual types supported at different depths.
	gpXVisualInfo=glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
	
	// Step 4: Set the window attributes
	winAttribs.border_pixel = 0; // 0 - default border color 	
	winAttribs.background_pixmap = 0; // background pixmap - images like cursors, icon, etc. (0 - default)
	// Get the colormap ID.
	// Note:- The XCreateColormap() function creates a colormap of the specified visual type for the screen on which the specified window resides and returns the colormap ID associated with it.
	winAttribs.colormap = XCreateColormap(gpDisplay,RootWindow(gpDisplay, gpXVisualInfo->screen), gpXVisualInfo->visual, AllocNone); // AllocNone - Don't allocate fixed memory.
	gColormap = winAttribs.colormap;
	winAttribs.background_pixel = BlackPixel(gpDisplay,defaultScreen);
	
	// Specify the events which should be sent to this window
	winAttribs.event_mask = ExposureMask | // similar to WM_PAINT 
				VisibilityChangeMask | // similar to WM_CREATE
				ButtonPressMask | // handles mouse button events
				KeyPressMask | // handles keyboard key press events
				PointerMotionMask | 
				StructureNotifyMask; //case ConfigureNotify: (similar to WM_SIZE)
	// Specify the window style
	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap; 

	// Step 5: Create the window
	gWindow = XCreateWindow(gpDisplay, // 'Display' obj - specifies the connection to the X server
				RootWindow(gpDisplay,gpXVisualInfo->screen), // Specifies the parent window
				0, // x-coordinate		
				0, // y-coordinate
				giWindowWidth, // window width
				giWindowHeight, // window height
				0, // border width (0-default)
				gpXVisualInfo->depth, // depth
				InputOutput, // type of window
				gpXVisualInfo->visual, // type of visual
				styleMask, // style
				&winAttribs); // attributes of the window

	if(!gWindow){
		printf("ERROR: Failed to create the main window. \n Exiting now...\n");
		uninitialize();
		exit(1);
	}

	// Step 6: Name in the caption bar
	XStoreName(gpDisplay, gWindow, "First OpenGL window");
	
	// Step 7: Process the window close event
	
	// Step 7.1: Create an atom to handle the close window event
	// Note:- 'Atom' is a unique string and it stays in the memory till the app ends
	Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True); // XInternAtom returns the atom identifier associated with "WM_DELETE_WINDOW" (WM- Window Message) atom string, 'True' - Create an atom (even if it exists).
	
	// Step 7.2: Ask window manager to add the atom created above to the protocols
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete, 1); // 1 - number of protocols to set
	
	// Step 8: Map this window (created above) to the screen
	XMapWindow(gpDisplay, gWindow);
}


