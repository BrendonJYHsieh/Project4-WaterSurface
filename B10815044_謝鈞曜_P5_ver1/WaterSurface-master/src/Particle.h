#include <glm/glm.hpp>
class Particle
{
public:

		glm::mat4 Module=glm::mat4(1.0);
		glm::vec4 Color;
		glm::vec3 trainPos;
		float Offset;
		float Scale;
		float Life;
		GLfloat Scale_randon;
		GLfloat X_random;
		GLfloat Y_random;
		GLfloat Z_random;
		
		Particle(){ 
			Module = glm::mat4(1.0);
			Color = glm::vec4(1.0);
			Life = 0.0;
			Offset = 0.0;
			Scale = 0.0;
		}

private:

};
