#include "ofApp.h"

/*
	Default image size is 1200x800

	Press a - draws axis
	Press g - draws grid
	Press r - render the image and save to bin/data
	Press f3 - See what the renderCam is looking at
	Press n - enable/disable SSAA
	Press v - enable/disable animation

	For moving the spheres or lights:
		Click a sphere to select it.
		Click outside the sphere to deselect it.
		After selecting a sphere, press arrow keys for x, y direction movement.
		Press i to go into the screen.
		Press o to go out the screen.
		Moving the object with a mouse is not supported.
	To delete an object, press d while mouse is holding onto the object.

	To ray trace multiple frames:
	Default frame rate is 24
	To set KeyFrame for an object:
		Press and hold onto the object, then press
			s - To set startFrame
			e - To set endFrame
	Press spacebar to start/stop object movement
	Press Left Arrow key to reset all "animatable" object position 
	Press V to enable Ray Tracing multiple frames. Then press R for ray tracing.


	Completed lambert and phong shading.
	Phong shading includes lambert shading.
	Support ambient lighting and shadows.
	@author: Yu Jun Zhao
*/

/*
 * A ray tracing function used for computing an image of the 3d space.
 *
 */

void ofApp::rayTrace(string fileName) {

	float pixelW = 1 / imageWidth;
	float pixelH = 1 / imageHeight;
	float pixelHalfW = pixelW / 2;
	float pixelHalfH = pixelH / 2;

	for (int row = 0; row < imageHeight; row++) {
		for (int col = 0; col < imageWidth; col++) {

			float centerU = col * pixelW + pixelHalfW;
			float centerV = row * pixelH + pixelHalfH;
	
			// Compute the color for a pixel
			ofColor SSColor = SSAAliasing(centerU, centerV, pixelW, pixelH, b_antiAliasing);

			image.setColor(col, imageHeight - row - 1, SSColor);
			
			
		}
	}

	image.save(fileName);

}


float ofApp::lambertAlgorithm(const glm::vec3 &light_normal, 
	const glm::vec3 &norm, const float lightIntensity) {
	
	float max = glm::max(0.0f, glm::dot(norm, light_normal));
	float illumination = KdCoefficient * lightIntensity;
	return illumination * max;
}

float ofApp::phongAlgorithm(const glm::vec3 &hBiSector,
	const glm::vec3 &norm, const float lightIntensity) {

	float max = glm::max(0.0f, glm::dot(norm, hBiSector));
	float illumination = KsCoefficient * lightIntensity;
	return illumination * glm::pow(max, phongPower);
}
/**
 * Algorithm for shading
 * Uses phong and lambert algorithm. 
 * 
 * @param poi: the Point of Intersection
 * @param norm: the normal of the intersection.
 */


ofColor ofApp::shade(const glm::vec3 &poi, const glm::vec3 &norm, 
	const ofColor diffuse, const ofColor specular, float power, const SceneObject * interObj) {

	ofColor ambientColor = AmbientCoefficient * diffuse;
	ofColor addUpColor(ambientColor);
	glm::vec3 normal = glm::normalize(norm);
	glm::vec3 normal_cam_v = glm::normalize(renderCam.getPosition() - poi);


	for (Light *light : lightSources) {
		
		ofColor diffuseColor = diffuse; 
		ofColor specularColor = specular;

		glm::vec3 light_vector = light->getPosition() - poi;

		glm::vec3 lightv_n = glm::normalize(light_vector);
		float lightv_length = glm::length(light_vector); // Light vector length

		float lightIntensity = (light->lightIntensity / (glm::pow2(lightv_length)));

		//addUpColor += (specularColor + diffuseColor);


		// For calculating the shadows
		// Calculate the distance between the shape surface and the shape center
		// Create an abstract test point that is slightly above the shape surface
		// To prevent shadow rounding errors
		glm::vec3 centerToPointDir = glm::normalize(poi - interObj->getPosition());
		float centerToPointLength = glm::length(poi - interObj->getPosition());
		float newLength = centerToPointLength + 0.05f;
		glm::vec3 testP = (interObj->getPosition() + centerToPointDir * newLength);
		

		// Calculate Shadows
		
		unsigned int i = 0;
		for (; i < scene.size(); i++) {
			SceneObject *obj = scene[i];
			glm::vec3 interP;
			glm::vec3 interN;
			
			// lightv_n the dir toward the light
			if (obj->intersect(Ray(testP, lightv_n), interP, interN)) {
				float distance = glm::length(interP - poi); // use the real point of intersection
				// distance < lightv_length something is between the light and the surface
				// since lightv_length is the distance between the poi and the light source
				// distance is the distance between the intersected scene object point and poi. 
				if (distance < lightv_length) {
					
					break;  //break until check next light source
				}	
			}
			
		}
		
		// i == scene.size means nothing blocks the two light, 
		//  since all sceneObject was not intersecteds
		if (i == scene.size()) {

			// for specular
			glm::vec3 hbiSector = glm::normalize(normal_cam_v + lightv_n);

			specularColor *= phongAlgorithm(hbiSector, normal, lightIntensity);
			diffuseColor *= lambertAlgorithm(lightv_n, normal, lightIntensity);

			addUpColor += (specularColor + diffuseColor);
		}

	}


	// Calculate Reflection
	// this will never stop if there is always a reflectedClosestObjIndex
	if (interObj->is_bglazed()) {
		glm::vec3 rp, rn;
		glm::vec3 reflectedRayDir = 2 * (glm::dot(normal, normal_cam_v)) * normal - normal_cam_v;
		int reflectedClosestObjIndex = findClosestIndex(Ray(poi, glm::normalize(reflectedRayDir)), rp, rn);
		// recurse
		if (reflectedClosestObjIndex >= 0) {
			//found reflected obj
			SceneObject *reflectedObj = scene[reflectedClosestObjIndex];
			addUpColor += shade(rp, rn,
				reflectedObj->getDiffuseColor(),
				reflectedObj->getSpecularColor(),
				power, reflectedObj);
		}
	}
	
	return addUpColor;
}

/**
 * Super Sampling Anti-Aliasing
 * Divide a single pixel into 9 smaller pixels. 
 * Create 9 different rays to intersect the 9 subpixels center point 
 *
 * @param (centerU, centerV) == the center point on the viewport
 * @param pixelW == the pixel width
 * @param pixelH == the pixel height
 * @param on_or_off == Activate SSAA or not
 * @return the average color collected from the 9 smaller pixels or centerPointColor if on_or_off == false 
 */

ofColor ofApp::SSAAliasing(const float centerU, const float centerV, const float pixelW,
	const float pixelH, bool on_or_off) {
	
	if (on_or_off) {
		float smallPixelW = pixelW / 3;
		float smallPixelH = pixelH / 3;
		float topLeftU = centerU - smallPixelW;
		float topLeftV = centerV - smallPixelH;

		glm::vec3 p1, norm1;
		float r = 0, g = 0, b = 0;
		ofColor avgColor;

		for (int row = 0; row < 3; row++) {
			for (int col = 0; col < 3; col++) {
				Ray ray = renderCam.getRay(topLeftU + smallPixelW * col, topLeftV + smallPixelH * row);
				int indexIntersected = findClosestIndex(ray, p1, norm1);

				ofColor intersectedColor = (indexIntersected >= 0) ? 
					shade(p1, norm1, scene[indexIntersected]->getDiffuseColor(), scene[indexIntersected]->getSpecularColor(), 
					phongPower, scene[indexIntersected]) : ofColor::black;
				
				r += intersectedColor.r;
				g += intersectedColor.g;
				b += intersectedColor.b;
			}
		}
	
	
		avgColor.r = r / 9;
		avgColor.g = g / 9;
		avgColor.b = b / 9;
	
		return avgColor;
	}
	else {
		
		glm::vec3 p, norm;

		Ray midMid = renderCam.getRay(centerU, centerV);
		int mm = findClosestIndex(midMid, p, norm);
		ofColor mmc = (mm >= 0) ? shade(p, norm, scene[mm]->getDiffuseColor(),
			scene[mm]->getSpecularColor(), phongPower, scene[mm]) : ofColor::black;
		return mmc;
	}
		
	
}



 

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetBackgroundColor(ofColor::black);
	
	//setup Sliders
	panel.setup();
	panel.add(KdCoefficient.setup("Light Kd",33.0f, 1.0f, 100.0f));
	panel.add(phongPower.setup("shade Power", 60.0f, 10.0f, 10000.0f));
	panel.add(KsCoefficient.setup("Light Ks", 70.0f, 1.0f, 100.0f));
	panel.add(AmbientCoefficient.setup("Light Ambient", 0.1f, 0, 1));
	// Default is blue
	panel.add(colorSlider.setup("Colors RGB", glm::vec3(0,0,255), glm::vec3(0,0,0), glm::vec3(255,255,255))); 
	panel.add(lightPower.setup("Light Intensity", 0.5, 0, 1));
	panel.add(totalFrame.setup("Total Animation Frame", 50, 0, 199));

	mainCam.setDistance(30);
	mainCam.setNearClip(.1);
	sideCam.setPosition(40, 0, 0);
	sideCam.lookAt(glm::vec3(0, 0, 0));

	previewCam.setPosition(renderCam.getPosition() + glm::vec3(0,0,0));
	// for calculating the field of view angle
	float x = renderCam.view.width() / 2;
	float y = glm::length(renderCam.getPosition() - renderCam.view.getPosition());

	// 180/PI for converting radian to degree, times 2 since atan(x/y) only gives out half of the degree
	float fovDegree = (atan(x/y) * 180/PI) * 2;
	previewCam.setFov(fovDegree);

	theCam = &mainCam;
	//cout << theCam->getPosition() << endl;
	//-----Create default Sphere
	Sphere *sphere1 = new Sphere();
	Sphere *sphere2= new Sphere(glm::vec3(1.5,-0.5,0), 0.5, ofColor::green);
	Sphere *sphere3 = new Sphere(glm::vec3(1, 1, -5), 1, ofColor::yellow);

	Plane *plane = new Plane(glm::vec3(0,-1,0),glm::vec3(0,1,0));
	plane->setMirrorAble(true);
	plane->setAnimatable(true);

	Light *light1 = new Light(glm::vec3(1, 5, 2));
	Light *light2 = new Light(glm::vec3(-1, 4, -3.5));

	scene.push_back(sphere3);
	scene.push_back(sphere1);
	scene.push_back(sphere2);
	scene.push_back(plane);
	
	lightSources.push_back(light1);
	lightSources.push_back(light2);

	image.allocate(imageWidth,imageHeight,OF_IMAGE_COLOR);

}

//--------------------------------------------------------------
void ofApp::update() {

	// if space bar is pressed
	if (b_translate) {
		currentFrame = currentFrame >= totalFrame ? 0 : currentFrame + 1;

		// reset position when frame == 0
		if (currentFrame == 0) {
			resetAllToStartFrame();
		}
		else {
			// Update all animatable object's position
			// translate 
			
			for (unsigned int i = 0; i < scene.size(); i++) {
				SceneObject *obj = scene[i];
				if (obj->is_animatable() && obj->is_b_SandEKeyFrameSet()) {
					glm::vec3 slope = obj->getEndFramePos() - obj->getStartFramePos();
					obj->setPosition(linearUpdate(currentFrame, obj->getStartFramePos(), slope, totalFrame));
				}
			}

			for (unsigned int i = 0; i < lightSources.size(); i++) {
				Light *light = lightSources[i];
				if (light->is_animatable() && light->is_b_SandEKeyFrameSet()) {
					glm::vec3 slope = light->getEndFramePos() - light->getStartFramePos();
					light->setPosition(linearUpdate(currentFrame, light->getStartFramePos(), slope, totalFrame));
				}
			}
		}
		
	}

	// for raytracing
	// if not animatable ray trace once
	if (bTrace && !b_animatable) {
		cout << "tracing" << endl;

		rayTrace("RayTraced.jpg");
		bTrace = false;

		cout << "complete" << endl;

	}
	// trace from frame zero
	else if (bTrace && b_animatable) {
		b_translate = true;
		cout << "tracing frame: " + std::to_string(currentFrame)<< endl;

		rayTrace("RayTraced." + std::to_string(currentFrame) + ".jpg");
		//bTrace = false;

		cout << "complete" << endl;

		if (currentFrame == totalFrame) {
			bTrace = false;
			b_animatable = false;
			b_translate = false;

		}
	}
	

}

//--------------------------------------------------------------
void ofApp::draw() {
	if (!sliderBHide) {
		panel.draw();
	}


	theCam->begin();

	ofSetColor(ofColor::green);
	ofNoFill();

	for (SceneObject *obj : scene) {
		//ofSetColor(sphere->diffuseColor);
		obj->draw();
	}

	for (Light *light: lightSources) {
		light->draw();
	}

	ofSetColor(ofColor::lightSkyBlue);
	renderCam.drawFrustum();
	ofSetColor(ofColor::blue);
	renderCam.draw();

	if (bGrid) {
		renderCam.drawGrid(imageWidth, imageHeight);
	}
	if (bAxis) {
		renderCam.drawAxis(imageWidth, imageHeight);
	}
	

	theCam->end();
	// Draw some states
	string str = "Frame Rate: " + std::to_string(ofGetFrameRate()) +
		"\nFrame: " + std::to_string(currentFrame) + "/" + std::to_string(totalFrame);
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);

	str = "Cam Movement: ";
	str += mainCam.getMouseInputEnabled() ? "enabled" : "disabled";
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() - 175, 45);
	
	str = "SSAA: ";
	str += b_antiAliasing ? "true" : "false";
	ofDrawBitmapString(str, ofGetWindowWidth() - 80, 60);

	str = "Object moving: ";
	str += b_translate ? "true" : "false";
	ofDrawBitmapString(str, ofGetWindowWidth() - 160, 75);

	str = "Ray Tracing: ";
	str += b_animatable ? "Animation" : "Single";
	if(b_animatable)ofSetColor(ofColor::green);
	else ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() - 180, 90);

	if (objPicked && !mainCam.getMouseInputEnabled()) {
		if (interSectedObj->is_animatable()) {
			str = "";
			str += "\nkey1 at: " + to_string(interSectedObj->getStartFramePos().x) + ", " + to_string(interSectedObj->getStartFramePos().y) + ", " + to_string(interSectedObj->getStartFramePos().z);
			str += "\nkey2 at: " + to_string(interSectedObj->getEndFramePos().x) + ", " + to_string(interSectedObj->getEndFramePos().y) + ", " + to_string(interSectedObj->getEndFramePos().z);
			ofDrawBitmapString(str, 0, 250);
		}
	}


}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case 'a':
		bAxis = !bAxis;
		break;
	case 'b':
		sliderBHide = !sliderBHide;
		break;
	case 'C':
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'd':
		if (objPicked && !mainCam.getMouseInputEnabled()) {
			// Delete From the scene list
			vector<SceneObject *>::iterator s = scene.begin();
			while (s != scene.end()) {
				if((*s)->name().compare(interSectedObj->name()) == 0) {
					s = scene.erase(s);
					break;
				}
				else {
					s++;
				}
			}

			// Delete From the light list
			vector<Light *>::iterator l = lightSources.begin();
			while (l != lightSources.end()) {
				if ((*l)->name().compare(interSectedObj->name()) == 0) {
					l = lightSources.erase(l);
					break;
				}
				else {
					l++;
				}
			}

			// Delete actual object
			//cout << "Before: " << interSectedObj << endl;
			delete interSectedObj;
			//cout << "After: " << interSectedObj << endl;
			objPicked = false;
		}
		break;
	case 'e':
		if (objPicked && !mainCam.getMouseInputEnabled()) {
			if (interSectedObj->is_animatable()) {
				interSectedObj->setEndFrame(interSectedObj->getPosition());
			}
		}
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'g':
		bGrid = !bGrid;
		break;
	case 'h':
		bHide = !bHide;
		break;
	case 'm': // stop or move object
		if (objPicked && !mainCam.getMouseInputEnabled()) {
			interSectedObj->setAnimatable(!interSectedObj->is_animatable());
		}
		break;
	case 'n':
		b_antiAliasing = !b_antiAliasing;
		break;
	
	case 'r':
		bTrace = true;
		break;
	case 's':
		if (objPicked && !mainCam.getMouseInputEnabled()) {
			if (interSectedObj->is_animatable()) {
				interSectedObj->setStartFrame(interSectedObj->getPosition());
			}
		}
		break;
	case 'v':
		b_animatable = !b_animatable;
		if (b_animatable) ofSetFrameRate(24);
		else ofSetFrameRate(60);
		break;
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &previewCam;
		break;
	case '1':
		//for now
		if (!mainCam.getMouseInputEnabled()) {
			
			glm::vec3 mouseWorldPos = theCam->screenToWorld(
				glm::vec3(ofGetMouseX(), ofGetMouseY(), 0)); 
			
			glm::vec3 dir_normal = glm::normalize(mouseWorldPos - theCam->getPosition());
			// intersect an abstract plane that crosses the origin.
			float dist;
			if (glm::intersectRayPlane(
				mouseWorldPos, dir_normal, glm::vec3(0, 0, 0), theCam->getZAxis(), dist)) {
				mouseWorldPos = mouseWorldPos + dir_normal * dist;
			}

			Sphere * sphere = new Sphere(mouseWorldPos, 1, ofColor(colorSlider->x, colorSlider->y, colorSlider->z));
			scene.push_back(sphere);
		}
		break;
	case '2':
		if (!mainCam.getMouseInputEnabled()) {

			glm::vec3 mouseWorldPos = theCam->screenToWorld(
				glm::vec3(ofGetMouseX(), ofGetMouseY(), 0));

			glm::vec3 dir_normal = glm::normalize(mouseWorldPos - theCam->getPosition());
			// intersect an abstract plane that crosses the origin.
			float dist;
			if (glm::intersectRayPlane(
				mouseWorldPos, dir_normal, glm::vec3(0, 0, 0), theCam->getZAxis(), dist)) {
				mouseWorldPos = mouseWorldPos + dir_normal * dist;
			}

			Light * light = new Light(mouseWorldPos, ofColor::white, lightPower);
			lightSources.push_back(light);
		}
		break;
	case ' ':
		b_translate = !b_translate;
		break;

	case OF_KEY_LEFT:
		// If currently not moving reset to all animatable object to start keyframe
		if (!b_translate) {
			currentFrame = 0;
			resetAllToStartFrame();
		}
		break;
	default:
		break;
	}
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// 1) Convert object position from world space to screen space
	// 2) Add the offset
	// 3) Convert the object position from screen space back to world space.

	// Drag enabled only if an object is selectet/picked AND
	// mainCam movement is disabled.
	if (objPicked && !mainCam.getMouseInputEnabled()) {
		glm::vec3 objScreenPos = theCam->worldToScreen(interSectedObj->getPosition());
		glm::vec2 currentPoint = glm::vec2(x, y);
		glm::vec2 offset = currentPoint - lastXYpoint;
		objScreenPos += glm::vec3(offset, 0.0);
		interSectedObj->setPosition(theCam->screenToWorld(objScreenPos));
		lastXYpoint = currentPoint;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	// For picking the object
	glm::vec3 worldPos = theCam->screenToWorld(glm::vec3(x, y, 0.0));
	glm::vec3 d = worldPos - theCam->getPosition();
	glm::vec3 normal_d = glm::normalize(d);
	glm::vec3 intersectionPoint, norm;
	objPicked = false;
	
	int index = -1;
	float closest = INT_MAX; // infinity
	for (unsigned int i = 0; i < scene.size(); i++) {
		SceneObject *obj = scene[i];
		if (obj->is_intersectable_by_cam()) {
			Ray ray(theCam->getPosition(), normal_d);
			if (obj->intersect(ray, intersectionPoint, norm)) {
				objPicked = true;
				interSectedObj = obj;
				float distance = glm::length(interSectedObj->getPosition() - theCam->getPosition());
				if (closest > distance) {
					closest = distance;
					index = i;
				}
			}
		}
	}
	if (index > -1) {
		interSectedObj = scene[index];
	}

	// Lights take priority in getting selected.
	// To decrease the computation, it will not compare the distance between the objects.

	for (unsigned int i = 0; i < lightSources.size(); i++) {

		Light *light = lightSources[i];
		
		if (light->intersect(Ray(theCam->getPosition(), normal_d), intersectionPoint, norm)) {
			objPicked = true;
			interSectedObj = light;
		}
	}

	lastXYpoint = glm::vec2(x,y);

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	//reset the selectedObj
	interSectedObj = nullptr;
	objPicked = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}


// Helper function for Ray Tracing
/**
 * Find the intersected point and norm of the closest obj from the render cam
 * @param ray used for intersecting object
 * @param p for holding the intersected point 
 * @param norm for holding the intersected normal
 */
int ofApp::findClosestIndex(const Ray & ray, glm::vec3 & p, glm::vec3 & norm) {
	
	int closestIndex = -1;
	float closest = INT_MAX;

	for (unsigned int i = 0; i < scene.size(); i++) {
		glm::vec3 point;
		glm::vec3 normal;
		SceneObject *obj = scene[i];
		if (obj->intersect(ray, point, normal)) {

			float dist = glm::length(ray.p - point);
			if (dist < closest) {
				closest = dist;
				closestIndex = i;
				p = point;
				norm = normal;
			}
		}
	}

	return closestIndex;
}

// Reset all animatable object's position to their startFrame position
void ofApp::resetAllToStartFrame() {
	for (unsigned int i = 0; i < scene.size(); i++) {
		SceneObject *obj = scene[i];
		if (obj->is_animatable() && obj->is_b_SandEKeyFrameSet()) {
			obj->setPosition(obj->getStartFramePos());

		}
	}

	for (unsigned int i = 0; i < lightSources.size(); i++) {
		Light *light = lightSources[i];
		if (light->is_animatable() && light->is_b_SandEKeyFrameSet()) {
			light->setPosition(light->getStartFramePos());

		}
	}
}
