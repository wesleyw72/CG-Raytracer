#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include "Light.cpp"
#define PI 3.14159265359
#define MAXDEPTH 2
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
std::vector<pointLight*> lights;
int currentlySelectedLight = 0;
vec3 cameraPos( 0.f, 0.0f,-1.8f);

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
// vec3 DirectLight( const Intersection& i,const vec3 &dir,int depth )
// {
// 	vec3 d;
// 	//Calculate Vector to light source
// 	switch(triangles[i.triangleIndex].matType)
// 	{
		
// 		case kDiffuse:
// 		{
// 			vec3 r = -lightPos + i.position;
// 			float rL = glm::length(r);
// 			r = glm::normalize(r);
// 			vec3 norm = glm::normalize(triangles[i.triangleIndex].normal);
// 			//Calculte dot product between normal and ray to light
// 			float comp = glm::dot(r,norm);
// 			comp = glm::max(0.0f,comp);
			
// 			//float temp = (float)(comp/ 4*PI*(rL*rL));
// 			d = lightColor * ((float)(comp/ (4*PI*(rL*rL)))) ;
// 			//Shadow
// 			Intersection closestIntersection2;
// 			float m = std::numeric_limits<float>::max();
// 			closestIntersection2.distance = m;

// 			bool cl = ClosestIntersection(i.position,r,triangles,closestIntersection2,i.triangleIndex);
// 			if(cl)
// 			{
// 				if(closestIntersection2.distance<rL)
// 				{
// 					//cast shadow
// 					vec3 black(0.0f,0.0f,0.0f);
// 					d = black;
// 				}
// 			}
// 			//Return colour of direct light
// 			break;
// 		}
// 		// case kReflection:
// 		// {
// 		// 	Vec3 R = reflect(dir, triangles[i.triangleIndex].normal); 
// 		// 	hitColor += 0.8 * DirectLight(i.position + triangles[i.triangleIndex].normal, R, objects, lights, options, depth + 1);
// 		// 	//Return colour of direct light
// 		// 	break;
// 		// }


// 	}
// 	return d;
	
// }
void fresnel(const vec3& I,const vec3& N,const float& ior,float &kr)
{
	float cosi = glm::clamp(glm::dot(I,N),-1.0f,1.0f);
	float etai = 1;
	float etat = ior;
	if(cosi < 0)
		cosi = -cosi;
	else
		std::swap(etai,etat);
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi)); 
	if (sint >= 1) { 
        kr = 1; 
    } 
    else { 
        float cost = sqrtf(std::max(0.f, 1 - sint * sint)); 
        cosi = fabsf(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 


}
vec3 refract(const vec3& I, const vec3& N, const float& ior)
{
		float cosi = glm::clamp(glm::dot(I,N),-1.0f,1.0f);
		float etai = 1;
		float etat = ior;
		vec3 n = N;
		if(cosi < 0)
			cosi = -cosi;
		else
		{
			std::swap(etai,etat);
			n= -N;
		}
		float eta = etai/etat;
		float k = 1- eta * eta * (1-cosi*cosi);
		vec3 zero(0.0f,0.0f,0.0f);
		return k < 0 ? zero : eta * I + (eta*cosi-sqrtf(k))*n;

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
	if(depth>MAXDEPTH)
		return d;
	if(ClosestIntersection(origin,dir,triangles,closestIntersection,-1))
	{
		switch(triangles[closestIntersection.triangleIndex].matType)
		{
			case kDiffuse:
			{
				//pointLight testLight(lightPos,lightColor,14);
				for (uint32_t i = 0; i < lights.size(); ++i) { 
					vec3 dir;
					float rL;
					vec3 pLi;
					vec3 norm = glm::normalize(triangles[closestIntersection.triangleIndex].normal);
					lights[i]->illuminate(closestIntersection.position,dir,pLi,norm,rL);
					pLi = pLi* triangles[closestIntersection.triangleIndex].color;
					Intersection closestIntersection2;
					float m = std::numeric_limits<float>::max();
					closestIntersection2.distance = m;
					bool vis = true;
				//Shadow Code
					if(ClosestIntersection(closestIntersection.position,dir,triangles,closestIntersection2,closestIntersection.triangleIndex))
					{
						if(closestIntersection2.distance<rL)
						{
							// if(triangles[closestIntersection2.triangleIndex].matType==kReflectionAndRefraction)
							vis = true;
						//d = black;
						}
					}
					if(vis)
					{
						d += pLi;
					}
					vec3 ambientLight(1.0f,1.0f,1.0f);
					//d+= 0.5f*ambientLight;
				}
				break;
			}
			case kReflection:
			{
				vec3 R = reflect(dir,triangles[closestIntersection.triangleIndex].normal);
				vec3 neight(0.6f,0.6f,0.6f);
				d += neight * CastRay(closestIntersection.position+0.0000001f*R, R,depth+1,-1);
				break;
			}
			case kReflectionAndRefraction:
			{
				vec3 refractionColor(0.0f,0.0f,0.0f);
				float kr;
				fresnel(dir, triangles[closestIntersection.triangleIndex].normal, triangles[closestIntersection.triangleIndex].ior, kr); 
				bool outside = glm::dot(dir,triangles[closestIntersection.triangleIndex].normal)<0;
				vec3 bias = 0.00001f * triangles[closestIntersection.triangleIndex].normal;
				if (kr < 1) { 
					vec3 refractionDirection = glm::normalize(refract(dir, triangles[closestIntersection.triangleIndex].normal, triangles[closestIntersection.triangleIndex].ior)); 
					vec3 refractionRayOrig = outside ? closestIntersection.position - bias : closestIntersection.position + bias; 
					refractionColor = CastRay(refractionRayOrig, refractionDirection, depth + 1,-1); 
				} 
				vec3 reflectionDirection = glm::normalize(reflect(dir, triangles[closestIntersection.triangleIndex].normal)); 
        		vec3 reflectionRayOrig = outside ? closestIntersection.position + bias : closestIntersection.position - bias; 
        		vec3 reflectionColor = CastRay(reflectionRayOrig, reflectionDirection, depth + 1,-1); 
        		d += reflectionColor * kr + refractionColor * (1 - kr); 
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
void cycleSelectedLight()
{

	currentlySelectedLight++;
	currentlySelectedLight = currentlySelectedLight % lights.size();
	cout<<"NEW LIGHT: "<<currentlySelectedLight;
}
void makeLights()
{	

	//vec3 lightPos( 0, -0.5, -0.7 );
	vec3 lightColor =  vec3( 1, 1, 1);
	vec3 lightPos( 0, -0.5, -0.7 );
	lights.push_back(new pointLight(lightPos, lightColor, 14));
	currentlySelectedLight=0;
	lightColor =  vec3( 1, 1, 1);
	lightPos = vec3( 0, -0.5, -0.7 );
	//lights.push_back(new pointLight(lightPos, lightColor, 14));
	 // std::vector<pointLight&> lights;
	 
	 // pointLight& temp = new pointLight(lightPos,lightColor,14);
	 // lights.push_back(temp);

}
int main( int argc, char* argv[] )
{
	makeLights();
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
	glm::vec3 neg(-1,-1,-1);
	if( keystate[SDLK_w] )
		lights[currentlySelectedLight]->displacePosition(Vforward);
	if( keystate[SDLK_s] )
	{
		vec3 neg2 = neg*Vforward;
		lights[currentlySelectedLight]->displacePosition(neg2);
	}
	if( keystate[SDLK_d] )
		lights[currentlySelectedLight]->displacePosition(Vright);
	if( keystate[SDLK_a] )
	{
		vec3 neg2 = neg*Vright;
		lights[currentlySelectedLight]->displacePosition(neg2);
	}
	if( keystate[SDLK_q] )
		lights[currentlySelectedLight]->displacePosition(Vup);
	if( keystate[SDLK_e] )
	{
		vec3 neg2 = neg*Vup;
		lights[currentlySelectedLight]->displacePosition(neg2);
	}
	if( keystate[SDLK_r] )
		cycleSelectedLight();
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);
	ModifyRotationMatrix(angle);
	#pragma omp parallel for
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

	// for(int i = 0; i < triangles.size() ; i ++){
	// 	Triangle triangle = triangles[i];
	// 	float t;
	// 	if(triangle.intersect(start,dir,&t))
	// 	{
	// 		if(t*glm::length(dir) < closestIntersection.distance && i!=TriangleToIgnore)
	// 		{
	// 			toReturn = true;
	// 			closestIntersection.position = start+t*dir;
	// 			closestIntersection.distance = t*glm::length(dir); //length of dir
	// 			closestIntersection.triangleIndex = i;
	// 		}
	// 	}

		
	// }
	// return toReturn;
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


		if( 0 <= t && 0 <= u && 0<= v && u + v <= 1){
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



