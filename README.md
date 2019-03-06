# OpenGLXWindow

This project creates a basic x window (using Xlib) and initializes the OpenGL context for the same.

Required packages:

1. libx11-dev

Command:- sudo apt-get install libX11-dev

2. mesa-common-dev

Command:- sudo apt-get install mesa-common-dev


3. libglu1-mesa-dev

Command:- sudo apt-get install libglu1-mesa-dev


Command to create an executable:
g++ -o XOpenGLWindow XOpenGLWindow.cpp -lX11 -lGL

