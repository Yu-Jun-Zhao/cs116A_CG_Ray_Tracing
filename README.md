# cs116A_CG_Ray_Tracing

Ray Tracing is an image rendering technique.
This application uses this technique to render multiple 3d images.
This application is created with [openFrameworks](https://openframeworks.cc/).
If you are using visual studio, please run in release mode.

![RayTracing Sample](/bin/data/RayTraced.jpg)

## Concepts Supported
	
* Shading Algorithms: Lambert, Blinn-Phong
* Supersampling Anti-Aliasing
* Support basic reflections and shadows rendering

## Controls
	Press F1 - main cam, F2 - side cam, F3 - preview Cam
	Press c to lock/unlock the main cam
	
	To Create Object (camera must be locked first): 
		Press 1 - Sphere
		Press 2 - Point Light
	To Set KeyFrames (camera must be locked first):
		Click on the object, hold, and press s - Set Start Key Frame Position
		Click on the object, hold, and press e - Set End Key Frame Position
		Press spacebar - To see objects in action
	
	To Render Image (default location: bin/data/):
		Press r - render a single image 
		
	To Render multiple images (default location: bin/data/):
		Set the total number of frames with the slidebar
		Press v - to enable ray tracing multiple frames
		Press left-arrow-key - Set all objects to their start key frame position
		Press r - start rendering
		
	For more information, please take a look at the source code.
	
