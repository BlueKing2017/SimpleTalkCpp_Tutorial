#pragma once

#include "MyVec.h"

class MyTriangle {
public:
	MyTriangle() = default;

	MyTriangle(const MyVec3f& v0_, const MyVec3f& v1_, const MyVec3f& v2_) 
		: v0(v0_)
		, v1(v1_)
		, v2(v2_)
	{}

	MyVec3f v0;
	MyVec3f v1;
	MyVec3f v2;
};

class MyPlane {
public:
	MyPlane() = default;

	MyPlane(const MyVec3f& normal_, float distance_) 
		: normal(normal_)
		, distance(distance_)
	{}

	MyPlane(const MyVec3f& normal_, const MyVec3f& point_)
		: normal(normal_)
		, distance(normal_.dot(point_))
	{}

	MyPlane(const MyTriangle& tri) {
		auto n =(tri.v1 - tri.v0).cross(tri.v2 - tri.v0).normalize();
		distance = n.dot(tri.v0);
		normal = n;
	}

	MyVec3f normal;
	float distance;
};