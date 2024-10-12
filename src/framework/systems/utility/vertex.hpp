// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace nile
{
class Vertex
{
    private:
    glm::vec3 position, normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec2 guiPos;

    public:
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoord) 
        : position(position),
        normal(normal),
        texCoord(texCoord) {};

    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoord, glm::vec3 meshTangent)
        : position(position),
        normal(normal),
        texCoord(texCoord),
        tangent(meshTangent)
    {};

    ~Vertex();

    glm::vec3 getPosition() { return position; }
    glm::vec3 getNormal() { return normal; }
    glm::vec2 getTextureCoord() { return texCoord; }
    glm::vec3 getTangent() { return tangent; }

};
}