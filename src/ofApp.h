
#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Primitives.h"
  

class ofApp : public ofBaseApp{
private:
	// Cameras 
	ofEasyCam  mainCam;
	ofCamera sideCam;
	ofCamera previewCam;
	ofCamera  *theCam;

	// some gui sliders
	ofxPanel panel;
	ofxFloatSlider KdCoefficient;
	ofxFloatSlider phongPower;
	ofxFloatSlider KsCoefficient;
	ofxFloatSlider AmbientCoefficient;
	ofxFloatSlider lightPower;

	ofxVec3Slider colorSlider;

	// set up one render camera to render image throughn
	RenderCam renderCam;
	ofImage image;

	// storage of all sceneobjects
	vector<SceneObject *> scene;
	// storage of all lights
	vector<Light *> lightSources;

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


	float imageWidth = 1200;//600;
	float imageHeight = 800;//400;

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


};
 