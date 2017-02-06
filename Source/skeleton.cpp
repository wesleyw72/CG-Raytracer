#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

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
vec3 cameraPos( 0.0f, 0.0f,-2.0f);
glm::mat3 R;
float angle = 0.0f;
float yaw = 0.1f;


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
bool ClosestIntersection(vec3 start,vec3 dir,const vector<Triangle>& triangles,Intersection& closestIntersection);
void ModifyRotationMatrix(float value);

int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.
	LoadTestModel( triangles );
	cout << triangles.size() << endl;
	ModifyRotationMatrix(0);
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
	printf("%f\n",angle);
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
			if(ClosestIntersection(cameraPos,d,triangles,closestIntersection)){
				color = triangles[closestIntersection.triangleIndex].color;
			}
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

bool ClosestIntersection(vec3 start,vec3 dir,const vector<Triangle>& triangles,Intersection& closestIntersection){
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

		if( 0 <= t && 0 < u && 0< v && u + v <= 1){
			toReturn = true;
			if(t < closestIntersection.distance ){
				closestIntersection.position = start+t*dir;
				closestIntersection.distance = t*glm::length(dir);
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



