//  General Purpose Ray class 
//

#pragma once

#include "ofMain.h"
#include "ofxGui.h"

// Ray for ray tracing
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
private:
	glm::vec3 startFramePos = glm::vec3(0, 0, 0); // place holder value
	glm::vec3 endFramePos = glm::vec3(0, 0, 0);

	bool b_glazed = false;  // for mirror 
	bool animatable = true; // all object by default are animatable
	bool b_startFrame = false;
	bool b_endFrame = false;

protected:
	glm::vec3 position = glm::vec3(0, 0, 0);
	ofColor diffuseColor = ofColor::lightBlue;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	bool intersectable_by_cam = true; // for mouse picking
	bool intersectable_by_light = true;

public:
	static int id;
	// common data
	string obj_name = "object";
	
	// Functions that must be overrided
	virtual void draw() = 0;    
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { cout << "SceneObject::intersect" << endl; return false; }
	
	// Getter and Setter
	string name() { return obj_name; }
	void setMirrorAble(bool b) { b_glazed = b; }
	void setAnimatable(bool b) { animatable = b; }
	void setStartFrame(glm::vec3 pos) { startFramePos = pos; b_startFrame = true; }
	void setEndFrame(glm::vec3 pos) { endFramePos = pos;  b_endFrame = true;}
	void setPosition(glm::vec3 pos) { position.x = pos.x; position.y = pos.y, position.z = pos.z; }

	glm::vec3 getPosition() const { return position; }
	glm::vec3 getStartFramePos() const { return startFramePos; }
	glm::vec3 getEndFramePos() const { return endFramePos; }
	ofColor getDiffuseColor() const { return diffuseColor; }
	ofColor getSpecularColor() const { return specularColor; }
	bool is_bglazed() const { return b_glazed; }
	bool is_animatable() const { return animatable; }
	bool is_b_SandEKeyFrameSet() const { return b_startFrame && b_endFrame; }
	bool is_intersectable_by_cam() const { return intersectable_by_cam; }
	bool is_intersectable_by_light() const { return intersectable_by_light; }
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
protected:
	float radius;
public:
	
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray):radius(r) {
		position = p; 
		diffuseColor = diffuse;
		obj_name = "Sphere_" + to_string(id);
		id++;

	}

	Sphere():radius(1) {
		obj_name = "Sphere_" + to_string(id);
		id++;
	}

	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}

	void setRadius(float rad) {
		radius = rad;
	}

	void draw();
};

//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
};


//  General purpose plane 
//
class Plane : public SceneObject {
protected:
	// common data
	ofPlanePrimitive plane;
	glm::vec3 normal = glm::vec3(0, 1, 0);
	float width;
	float height;

public:
	

	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::white, float w = 40, float h = 40): 
		normal(n),width(w), height(h) {

		position = p;
		diffuseColor = diffuse;
		plane.rotateDeg(90, 1, 0, 0);
		intersectable_by_cam = false;
		intersectable_by_light = true;
		
		obj_name = "Plane_" + to_string(id);
		id++;
	}
	Plane() {
		obj_name = "Plane_" + to_string(id);
		id++;
	}
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	void draw();
	
};

// view plane for render camera
//  To define an infinite plane, we just need a point and normal.
//  The ViewPlane is a finite plane so we need to define the boundaries.
//  We will define this in terms of min, max  in 2D.  
//  (in local 2D space of the plane)
//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
//  in the scene, so it is easier to define the View rectangle in a local'
//  coordinate system. 

class  ViewPlane : public Plane {
public:
	glm::vec2 min, max;

	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; obj_name = "ViewPlane"; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation

		intersectable_by_light = false;

		obj_name = "ViewPlane";
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}

	float width() {return (max.x - min.x);}
	float height() {return (max.y - min.y);}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }


};

// Most basic light class, point light
// Since it looks like a sphere, it inherits from the Sphere class.
class Light :public Sphere {
protected:
	float lightIntensity;

public:
	
	Light(glm::vec3 p, ofColor diffuse = ofColor::white, float light = 0.5f):
		lightIntensity(light){

		position = p;
		diffuseColor = diffuse;
		radius = 0.5;
		intersectable_by_cam = true;
		intersectable_by_light = false;

		obj_name = "Light_" + to_string(id);
		id++;
	}

	//void intersect
	void draw();

	//getters
	float getLightIntensity() { return lightIntensity; }
};


//  render camera  - currently must be z axis aligned (we will improve this in project 4)
//
class RenderCam : public SceneObject {
public:

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 

	RenderCam() : aim(glm::vec3(0, 0, -1)) {
		position = glm::vec3(0, 0, 10);
		intersectable_by_cam = false;
		intersectable_by_light = false;
		obj_name = "RenderCam";
	}

	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();
	void drawGrid(float, float);
	void drawAxis(float, float);

};
