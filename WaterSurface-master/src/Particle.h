#include <glm/glm.hpp>
class Particle
{
public:

		glm::mat4 Position;
		glm::vec4 Color;
		float scale;
		float Life;
		Particle():Position(0.0f), Color(1.0f), Life(0.0f), scale(0.0){ }

private:

};
