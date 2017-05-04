#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>
enum MaterialType { kDiffuse, kReflection, kReflectionAndRefraction }; 
class Shape
{
public:
	glm::vec3 color;
	MaterialType matType;
	float ior;
	virtual glm::vec3 GetNormal(glm::vec3 intersectPoint) =0;
	virtual bool intersect(glm::vec3 start,glm::vec3 dir,float *t)=0;
};
class Sphere
{
	glm::vec3 centre;
	float radius;
	bool quadSolver(float a,float b,float c, float &x0,float &x1)
	{
		float discrim = b * b - 4 * a * c;
		if(discrim<0) return false;
		else if (discrim==0)
		{
			x0 = -0.5* b/a;
			x1 = -0.5* b/a;

		}
		else
		{
			float q;
			if(b>0)
			{
				q = -0.5 * (b + sqrt(discrim));
			}
			else
			{
				q = -0.5 * (b - sqrt(discrim));
			}
			x0 = q/a;
			x1 = c/q;

		}
		if (x0>x1) std::swap(x0,x1);
		return true;
	}
public:
	glm::vec3 color;
	MaterialType matType;
	Sphere(glm::vec3 centre,float radius) : centre(centre),radius(radius)
	{

	}
	glm::vec3 GetNormal(glm::vec3 intersectPoint)
	{
		return glm::normalize(intersectPoint-centre);
	}
	bool intersect(glm::vec3 start,glm::vec3 dir,float *t)
	{
		glm::vec3 L = start - centre;
		float a = glm::dot(dir,dir);
		float b = 2* glm::dot(dir,L);
		float c = glm::dot(L,L)-radius;
		float t0,t1;
		if(!quadSolver(a,b,c,t0,t1)) return false;
		if(t0>t1) std::swap(t0,t1);
		if(t0<0)
		{
			t0=t1;
			if(t0<0) return false;
		}
		*t=t0;
		return true;
	}
};
// Used to describe a triangular surface:
class Triangle : public Shape
{
public:
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 normal;
	glm::vec3 color;
	MaterialType matType;
	float ior;

	Triangle( glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color )
		: v0(v0), v1(v1), v2(v2), color(color)
	{
		matType =kDiffuse;
		ior = 1.9f;
		ComputeNormal();
	}
	Triangle( glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color, MaterialType matType )
		: v0(v0), v1(v1), v2(v2), color(color), matType(matType)
	{
		ior = 1.9f;
		ComputeNormal();
	}

	void ComputeNormal()
	{
		glm::vec3 e1 = v1-v0;
		glm::vec3 e2 = v2-v0;
		normal = glm::normalize( glm::cross( e2, e1 ) );
	}
	glm::vec3 GetNormal(glm::vec3 intersectPoint)
	{
		return normal;
	}
	bool intersect(glm::vec3 start,glm::vec3 dir,float * t)
	{
		glm::vec3 e1 = v1 - v0;
		glm::vec3 e2 = v2 - v0;
		glm::vec3 b = start - v0;
		glm::mat3 A( -dir, e1, e2 );
		glm::vec3 x = glm::inverse( A ) * b;

		*t = x.x;
		//std::cout << ("T: %f",*t);
		float u = x.y;
		float v = x.z;
		if( 0.0f <= *t && 0.0f <= u && 0.0f<= v && u + v <= 1.0f){
			
			return true;

		}
		return false;
		//determine if a ray intersects with this triangle.

	}
};

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Triangle*>& triangles )
{
	using glm::vec3;

	// Defines colors:
	vec3 red(    0.75f, 0.15f, 0.15f );
	vec3 yellow( 0.75f, 0.75f, 0.15f );
	vec3 green(  0.15f, 0.75f, 0.15f );
	vec3 cyan(   0.15f, 0.75f, 0.75f );
	vec3 blue(   0.15f, 0.15f, 0.75f );
	vec3 purple( 0.75f, 0.15f, 0.75f );
	vec3 white(  0.75f, 0.75f, 0.75f );

	triangles.clear();
	triangles.reserve( 5*2*3 );

	// ---------------------------------------------------------------------------
	// Room

	float L = 555;			// Length of Cornell Box side.

	vec3 A(L,0,0);
	vec3 B(0,0,0);
	vec3 C(L,0,L);
	vec3 D(0,0,L);

	vec3 E(L,L,0);
	vec3 F(0,L,0);
	vec3 G(L,L,L);
	vec3 H(0,L,L);

	// Floor:
	triangles.push_back( new Triangle( C, B, A, green ) );
	triangles.push_back( new Triangle( C, D, B, green ) );

	// Left wall
	// triangles.push_back( Triangle( A, E, C, purple ,kReflection) );
	// triangles.push_back( Triangle( C, E, G, purple ,kReflection) );
	triangles.push_back( new Triangle( A, E, C, purple ) );
	triangles.push_back( new Triangle( C, E, G, purple ) );

	// Right wall
	triangles.push_back( new Triangle( F, B, D, yellow ) );
	triangles.push_back( new Triangle( H, F, D, yellow ) );

	// Ceiling
	triangles.push_back( new Triangle( E, F, G, cyan ) );
	triangles.push_back( new Triangle( F, H, G, cyan ) );

	// Back wall
	triangles.push_back( new Triangle( G, D, C, white ) );
	triangles.push_back( new Triangle( G, H, D, white ) );

	// ---------------------------------------------------------------------------
	// Short block

	A = vec3(290,0,114);
	B = vec3(130,0, 65);
	C = vec3(240,0,272);
	D = vec3( 82,0,225);

	E = vec3(290,165,114);
	F = vec3(130,165, 65);
	G = vec3(240,165,272);
	H = vec3( 82,165,225);

	// Front
	triangles.push_back( new Triangle(E,B,A,red) );
	triangles.push_back( new Triangle(E,F,B,red) );

	// Front
	triangles.push_back( new Triangle(F,D,B,red) );
	triangles.push_back( new Triangle(F,H,D,red) );

	// BACK
	triangles.push_back( new Triangle(H,C,D,red) );
	triangles.push_back( new Triangle(H,G,C,red) );

	// LEFT
	triangles.push_back( new Triangle(G,E,C,red) );
	triangles.push_back( new Triangle(E,A,C,red) );

	// TOP
	triangles.push_back( new Triangle(G,F,E,red) );
	triangles.push_back( new Triangle(G,H,F,red) );

	// ---------------------------------------------------------------------------
	// Tall block

	A = vec3(423,0,247);
	B = vec3(265,0,296);
	C = vec3(472,0,406);
	D = vec3(314,0,456);

	E = vec3(423,330,247);
	F = vec3(265,330,296);
	G = vec3(472,330,406);
	H = vec3(314,330,456);

	// Front
	triangles.push_back( new Triangle(E,B,A,blue,kReflectionAndRefraction) );
	triangles.push_back( new Triangle(E,F,B,blue,kReflectionAndRefraction) );

	// Front
	triangles.push_back( new Triangle(F,D,B,blue,kReflectionAndRefraction) );
	triangles.push_back( new Triangle(F,H,D,blue,kReflectionAndRefraction) );

	// BACK
	triangles.push_back( new Triangle(H,C,D,blue,kReflectionAndRefraction) );
	triangles.push_back( new Triangle(H,G,C,blue,kReflectionAndRefraction) );

	// LEFT
	triangles.push_back( new Triangle(G,E,C,blue,kReflectionAndRefraction) );
	triangles.push_back( new Triangle(E,A,C,blue,kReflectionAndRefraction) );

	// TOP
	triangles.push_back( new Triangle(G,F,E,blue,kReflectionAndRefraction) );
	triangles.push_back( new Triangle(G,H,F,blue,kReflectionAndRefraction) );


	// ----------------------------------------------
	// Scale to the volume [-1,1]^3

	for( size_t i=0; i<triangles.size(); ++i )
	{
		((Triangle*)triangles[i])->v0 *= 2/L;
		((Triangle*)triangles[i])->v1 *= 2/L;
		((Triangle*)triangles[i])->v2 *= 2/L;

		((Triangle*)triangles[i])->v0 -= vec3(1,1,1);
		((Triangle*)triangles[i])->v1 -= vec3(1,1,1);
		((Triangle*)triangles[i])->v2 -= vec3(1,1,1);

		((Triangle*)triangles[i])->v0.x *= -1;
		((Triangle*)triangles[i])->v1.x *= -1;
		((Triangle*)triangles[i])->v2.x *= -1;

		((Triangle*)triangles[i])->v0.y *= -1;
		((Triangle*)triangles[i])->v1.y *= -1;
		((Triangle*)triangles[i])->v2.y *= -1;

		((Triangle*)triangles[i])->ComputeNormal();
	}
}

#endif