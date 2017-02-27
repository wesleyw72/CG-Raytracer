#define PI 3.14159265359
class Light
{
	public:
		Light(const glm::vec3& pos,const glm::vec3& c,const float& inten) : position(pos), color(c), intensity(inten) {}
		//virtual ~Light() = 0;
		virtual void illuminate (const glm::vec3 &refPoint,glm::vec3 &dir,glm::vec3 &Lightinten,glm::vec3 normal,float &rL) = 0;
		glm::vec3 position;
		glm::vec3 color;

		float intensity;
};
class pointLight : public Light
{
	glm::vec3 norm;
	float distance;
	public:
		~pointLight(){};
		pointLight(const glm::vec3& pos,const glm::vec3& c,const float& inten): Light(pos,c,inten) {};
		void illuminate(const glm::vec3 &refPoint,glm::vec3 &dir,glm::vec3 &Lightinten,glm::vec3 normal,float &rL) 
		{
			dir = (-refPoint+position);
			rL = glm::length(dir);
			dir = glm::normalize(dir);
			norm = glm::normalize(normal);
			float comp = glm::dot(dir,norm);
			comp = glm::max(0.0f,comp);
			Lightinten = (color*(float)intensity) * ((float)(comp/ (4*PI*(rL*rL)))) ;
		}
};
