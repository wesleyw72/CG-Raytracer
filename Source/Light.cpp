#define PI 3.14159265359
class Light
{
	public:
		Light(const glm::vec3& pos,const glm::vec3& c,const float& inten) : position(pos), color(c), intensity(inten) {}
		virtual ~Light() {}
		virtual void illuminate (const glm::vec3 &refPoint,glm::vec3 &dir,glm::vec3 &Lightinten,float &distance,glm::vec3 normal);
		glm::vec3 position;
		glm::vec3 color;

		float intensity;
};
class pointLight : public Light
{
	glm::vec3 norm;
	public:
		void illuminate(const glm::vec3 &refPoint,glm::vec3 &dir,glm::vec3 &Lightinten,float &distance,glm::vec3 normal)
		{
			dir = (refPoint-position);
			float rL = glm::length(dir);
			dir = glm::normalize(dir);
			norm = glm::normalize(normal);
			float comp = glm::dot(dir,norm);
			Lightinten = color * ((float)(comp/ (4*PI*(rL*rL)))) ;
		}
};