//
//  RayCaster - Set of simple classes to create a camera/view setup for our Ray Tracer HW Project
//
//  I've included these classes as a mini-framework for our introductory ray tracer.
//  You are free to modify/change.   
//
//  These classes provide a simple render camera which can can return a ray starting from
//  it's position to a (u, v) coordinate on the view plane.
//
//  The view plane is where we can locate our photorealistic image we are rendering.
//  The field-of-view of the camera by moving it closer/further 
//  from the view plane.  The viewplane can be also resized.  When ray tracing an image, the aspect
//  ratio of the view plane should the be same as your image. So for example, the current view plane
//  default size is ( 6.0 width by 4.0 height ).   A 1200x800 pixel image would have the same
//  aspect ratio.
//
//  This is not a complete ray tracer - just a set of skelton classes to start.  The current
//  base scene object only stores a value for the diffuse/specular color of the object (defaut is gray).
//  at some point, we will want to replace this with a Material class that contains these (and other 
//  parameters)
//  
//  (c) Kevin M. Smith  - 24 September 2018
//

// Modified by Yu Jun Zhao (010570820)
#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Primitives.h"
  

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		// RayTracing function
		void rayTrace(string);
		ofColor shade(const glm::vec3 &, const glm::vec3 &, const ofColor, const ofColor, float, const SceneObject *);
		float lambertAlgorithm(const glm::vec3 &,const glm::vec3 &, const float);
		float phongAlgorithm(const glm::vec3 &, const glm::vec3 &, const float);

		// util function
		// for animation
		glm::vec3 linearUpdate(float frame, glm::vec3 startKey, glm::vec3 slope, float totalFrame) {
			return slope * (frame / totalFrame) + startKey;

		};
		// for antialiasing
		ofColor SSAAliasing(const float, const float, const float, const float, bool);

		// helper function
		int findClosestIndex(const Ray &, glm::vec3 &, glm::vec3 &);
		void resetAllToStartFrame();

		// Cameras 
		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera previewCam;
		ofCamera  *theCam;    

		// set up one render camera to render image throughn
		//
		RenderCam renderCam;
		ofImage image;

		// storage of all sceneobjects
		vector<SceneObject *> scene;
		// storage of all lights
		vector<Light *> lightSources;

		float imageWidth = 1200;//600;
		float imageHeight = 800;//400;

		// some gui sliders
		ofxPanel panel;
		ofxFloatSlider KdCoefficient;
		ofxFloatSlider phongPower;
		ofxFloatSlider KsCoefficient;
		ofxFloatSlider AmbientCoefficient;
		ofxFloatSlider lightPower;

		ofxVec3Slider colorSlider;

		// for animation
		int currentFrame = 0;
		ofxIntSlider totalFrame; 
		bool b_animatable = false;
		bool b_translate = false;
		// mouse interaction
		SceneObject *interSectedObj = nullptr;
		glm::vec2 lastXYpoint;

		// Boolean variables
		//bool mouseDown = false;
		bool objPicked = false; 
		

		bool bHide = true;
		bool bShowImage = false;
		bool bGrid = false; // for showing or hiding grid
		bool bAxis = false;
		bool bTrace = false;
		bool sliderBHide = false;
		bool b_antiAliasing = true;
};
 