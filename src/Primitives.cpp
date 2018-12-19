
#pragma once
#include "Primitives.h"

int SceneObject::id = 0;

void Sphere::draw() {
	ofSetColor(diffuseColor);
	ofDrawSphere(position, radius);
}

bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;

	}
	return (hit);
}

void Plane::draw() {
	ofSetColor(diffuseColor);
	plane.setPosition(position);
	plane.setWidth(width);
	plane.setHeight(height);
	plane.setResolution(4, 4);
	plane.drawWireframe();
}


// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

void RenderCam::drawFrustum() {
	view.draw();
	Ray r1 = getRay(0, 0); // bottom left
	Ray r2 = getRay(0, 1); // top left
	Ray r3 = getRay(1, 1); // top right
	Ray r4 = getRay(1, 0); // bottom right
	float dist = glm::length((view.toWorld(0, 0) - position));
	r1.draw(dist);
	r2.draw(dist);
	r3.draw(dist);
	r4.draw(dist);
}

// Function for Drawing grid
void RenderCam::drawGrid(float width, float height) {
	float pixelW = 1 / width;
	float pixelH = 1 / height;

	// for drawing vertical lines
	for (int vert = 1; vert < width; vert++) {
		glm::vec3 pointOnUpperBorder = view.toWorld(pixelW * vert, 1);
		glm::vec3 pointOnBottomBorder = view.toWorld(pixelW * vert, 0);
		Ray ray = Ray(pointOnUpperBorder, glm::normalize(pointOnBottomBorder - pointOnUpperBorder));
		ray.draw(view.height());
	}

	// for drawing horizontal lines
	for (int hor = 1; hor < height; hor++) {
		glm::vec3 pointOnLeftBorder = view.toWorld(0, pixelH * hor);
		glm::vec3 pointOnRightBorder = view.toWorld(1, pixelH * hor);
		Ray ray = Ray(pointOnLeftBorder, glm::normalize(pointOnRightBorder - pointOnLeftBorder));
		ray.draw(view.width());
	}
}

// draw rays of each pixel
void RenderCam::drawAxis(float width, float height) {
	float pixelW = 1 / width;
	float pixelH = 1 / height;
	float pixelHalfW = pixelW / 2;
	float pixelHalfH = pixelH / 2;

	// go through each pixels
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			Ray ray = getRay(col * pixelW + pixelHalfW, row * pixelH + pixelHalfH);
			//cout << "ray position: " << ray.p << endl;
			//cout << "ray direction: " << ray.d << endl;
			ray.draw(20);
		}
	}
}

void Light::draw() {
	ofFill();
	ofSetColor(diffuseColor);
	ofDrawSphere(position, radius);
	ofNoFill();
}
