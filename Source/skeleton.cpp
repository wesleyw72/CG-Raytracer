#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include "Light.cpp"
#define PI 3.14159265359
using namespace std;
using glm::vec3;
using glm::mat3;

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
vector<Triangle> triangles;
int t;
float focalLength = SCREEN_WIDTH/2;
glm::mat3 R;
float angle = 0.0f;
float yaw = 0.1f;


vec3 cameraPos( 0.f, 0.0f,-1.8f);
vec3 lightPos( 0, -0.5, -0.7 );
vec3 lightColor = 14.0f * vec3( 1, 1, 1 );
vec3 indirectLight = 0.5f*vec3( 1, 1, 1 );

/* ----------------------------------------------------------------------------*/
/* STRUCTS                                                           */
struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};

Intersection closestIntersection;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
void Update();
void Draw();
void ModifyRotationMatrix(float value);
vec3 reflect(const vec3& I,const vec3& N);
bool ClosestIntersection(vec3 start,vec3 dir,const vector<Triangle>& triangles,Intersection& closestIntersection,int TriangleToIgnore);
vec3 DirectLight( const Intersection& i,const vec3 &dir,int depth )
{
	vec3 d;
	//Calculate Vector to light source
	switch(triangles[i.triangleIndex].matType)
	{
		
		case kDiffuse:
		{
			vec3 r = -lightPos + i.position;
			float rL = glm::length(r);
			r = glm::normalize(r);
			vec3 norm = glm::normalize(triangles[i.triangleIndex].normal);
			//Calculte dot product between normal and ray to light
			float comp = glm::dot(r,norm);
			comp = glm::max(0.0f,comp);
			
			//float temp = (float)(comp/ 4*PI*(rL*rL));
			d = lightColor * ((float)(comp/ (4*PI*(rL*rL)))) ;
			//Shadow
			Intersection closestIntersection2;
			float m = std::numeric_limits<float>::max();
			closestIntersection2.distance = m;

			bool cl = ClosestIntersection(i.position,r,triangles,closestIntersection2,i.triangleIndex);
			if(cl)
			{
				if(closestIntersection2.distance<rL)
				{
					//cast shadow
					vec3 black(0.0f,0.0f,0.0f);
					d = black;
				}
			}
			//Return colour of direct light
			break;
		}
		// case kReflection:
		// {
		// 	Vec3 R = reflect(dir, triangles[i.triangleIndex].normal); 
		// 	hitColor += 0.8 * DirectLight(i.position + triangles[i.triangleIndex].normal, R, objects, lights, options, depth + 1);
		// 	//Return colour of direct light
		// 	break;
		// }


	}
	return d;
	
}
vec3 CastRay(const vec3& origin,const vec3& dir,const int depth,const int TriangleToIgnore=-1)
{
	//Check if depth has been exceeded
	if(depth>4)
	{
		vec3 black(1.0f,1.0f,1.0f);
		return black;
	}
	Intersection closestIntersection;
	vec3 black(0.0f,0.0f,0.0f);
	vec3 d;
	d = black;
	float m = std::numeric_limits<float>::max();
	closestIntersection.distance = m;
	if(ClosestIntersection(origin,dir,triangles,closestIntersection,-1))
	{
		switch(triangles[closestIntersection.triangleIndex].matType)
		{
			case kDiffuse:
			{
				pointLight testLight;
				vec3 r = lightPos - closestIntersection.position;
				float rL = glm::length(r);
				r = glm::normalize(r);
				vec3 norm = glm::normalize(triangles[closestIntersection.triangleIndex].normal);
			//Calculte dot product between normal and ray to light
				float comp = glm::dot(r,norm);
				comp = glm::max(0.0f,comp);
				d = lightColor * ((float)(comp/ (4*PI*(rL*rL)))) * triangles[closestIntersection.triangleIndex].color;
			//Shadow
				Intersection closestIntersection2;
				float m = std::numeric_limits<float>::max();
				closestIntersection2.distance = m;

				bool cl = ClosestIntersection(closestIntersection.position,r,triangles,closestIntersection2,closestIntersection.triangleIndex);
				if(cl)
				{
					if(closestIntersection2.distance<rL)
					{
					//cast shadow
						//vec3 black(0.0f,0.0f,0.0f);
						d = black;
					}
				}
				break;
			}
			case kReflection:
			{
				break;
			}
		}
	}
	return d;
}
vec3 reflect(const vec3& I,const vec3& N)
{
	return I - 2 * glm::dot(I,N)*N;
}
int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.
	LoadTestModel( triangles );
	cout << triangles.size() << endl;
	ModifyRotationMatrix(angle);
	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
		
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update()
{
	vec3 Vforward(0.0f,0.0f,0.1f);
	vec3 Vright(0.1f,0.0f,0.0f);
	vec3 Vup(0.0f,0.1f,0.0f);
	// Compute frame time
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;

	Uint8* keystate = SDL_GetKeyState( 0 );
	if( keystate[SDLK_UP] )
	{
	// Move camera forward
		cameraPos.z = cameraPos.z + 0.1f;
		
	}
	if( keystate[SDLK_DOWN] )
	{
	// Move camera backward
		cameraPos.z = cameraPos.z - 0.1f;
	}
	if( keystate[SDLK_LEFT] )
	{
		angle -= yaw;
		ModifyRotationMatrix(-yaw);
		cameraPos = R * cameraPos;
	}
	if( keystate[SDLK_RIGHT] )
	{
		angle += yaw;
		ModifyRotationMatrix(yaw);
		cameraPos = R * cameraPos;
		
	}

	if( keystate[SDLK_w] )
		lightPos += Vforward;
	if( keystate[SDLK_s] )
		lightPos -= Vforward;
	if( keystate[SDLK_d] )
		lightPos += Vright;
	if( keystate[SDLK_a] )
		lightPos -= Vright;
	if( keystate[SDLK_q] )
		lightPos += Vup;
	if( keystate[SDLK_e] )
		lightPos -= Vup;
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
	ModifyRotationMatrix(angle);
	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			float m = std::numeric_limits<float>::max();
			closestIntersection.distance = m;

			float xDiff = x - float(SCREEN_WIDTH/2);
			float yDiff = y - float(SCREEN_HEIGHT/2);
			vec3 d (xDiff, yDiff, focalLength);
			d = R*d;
			vec3 color ( 0.0f,0.0f, 0.0f);
			color = CastRay(cameraPos,d,0,-1);
			// if(ClosestIntersection(cameraPos,d,triangles,closestIntersection,-1)){
			// 	// color = DirectLight(closestIntersection)*triangles[closestIntersection.triangleIndex].color;
			// 	// color = triangles[closestIntersection.triangleIndex].color * ( color + indirectLight);
			// }
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

bool ClosestIntersection(vec3 start,vec3 dir,const vector<Triangle>& triangles,Intersection& closestIntersection,int TriangleToIgnore){
	bool toReturn = false;

	for(int i = 0; i < triangles.size() ; i ++){
		Triangle triangle = triangles[i];
		vec3 v0 = triangle.v0;
		vec3 v1 = triangle.v1;
		vec3 v2 = triangle.v2;
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;
		vec3 b = start - v0;
		mat3 A( -dir, e1, e2 );
		vec3 x = glm::inverse( A ) * b;

		float t = x.x;
		float u = x.y;
		float v = x.z;


		if( 0.0f <= t && 0.0f <= u && 0.0f<= v && u + v <= 1.0f){
			if(t*glm::length(dir) < closestIntersection.distance && i!=TriangleToIgnore){
				toReturn = true;
				closestIntersection.position = start+t*dir;
				closestIntersection.distance = t*glm::length(dir); //length of dir
				closestIntersection.triangleIndex = i;
			}
		}
	}
	return toReturn;
}

void ModifyRotationMatrix(float value){
	R[0][0] = glm::cos(value);
	R[0][1] = 0;
	R[1][0] = 0;
	R[1][1] = 1;
	R[1][2] = 0;
	R[2][1] = 0;
	R[2][2] = glm::cos(value);
	R[0][2] = glm::sin(value);
	R[2][0] = -(glm::sin(value));
}



