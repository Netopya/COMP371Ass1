COMP371 - Computer Graphics - Assignment 1
Michael Bilinsky 26992358

This app is able to read vertex information from a text file and render a translational or rotational construction using opengl.

Created with Visual Studio 2015

Controls:
Arrow keys to rotate the model around the global axes.
Mouse buttons to zoom in and out.
P, W, and T to toggle between Points, Wireframe, and Triangle view

Movement Speed:
In testing, interaction with the program has different sensitivities on different machines.
To modify the rate of rotation and zoom, change the rotatationSpeed and zoomSpeed constants on lines 76 and 77.

Input file:
Please place any input files in the same directory as the main.cpp file as this is the root directory.

Rotational Sweep:
In the assignment requirements, the second number in the text file for rotational sweep was suppose to refer to the number of sweeps to perform.
However the sample files provided only ever have 1 for this number, which is nonsensical. Therefore the program provides the option of manually specifying the number of sweeps.

 
