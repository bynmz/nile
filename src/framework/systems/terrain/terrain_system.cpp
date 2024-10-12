#include "terrain_system.hpp"
#include "../utility/math.hpp"

namespace nile
{
ProceduralTerrain::ProceduralTerrain(NileDevice &device, int gridX, int gridZ, 
    std::shared_ptr<MaterialPack> textures, std::string heightMap) 
    : device{device}
{
    this->x = gridX * SIZE;
    this->z = gridZ * SIZE;
    this->textures = textures;
    this->generator = std::make_shared<HeightsGenerator>(gridX, gridZ, VERTEX_COUNT, SEED);
    this->mesh = generateTerrain(textures, heightMap);
}

ProceduralTerrain::~ProceduralTerrain(){}

float ProceduralTerrain::getHeightOfTerrain(float worldX, float worldZ)
{
    float terrainX = worldX - this->x;
    float terrainZ = worldZ - this->z;
    float gridSquareSize = SIZE / (float)(heights.size() - 1);
    int gridX = (int) std::floor(terrainX / gridSquareSize);
    int gridZ = (int) std::floor(terrainZ / gridSquareSize);
    if(gridX >= heights.size() - 1 || gridZ >= heights.size() - 1 || gridX < 0 || gridZ < 0) {
        return 0;
    }
    float xCoord = std::fmod(terrainX, gridSquareSize)/gridSquareSize;
    float zCoord = std::fmod(terrainZ, gridSquareSize)/gridSquareSize;
    float answer;
    if (xCoord <= (1-zCoord)) {
			answer = Math::barryCentric(glm::vec3(0, heights[gridX][gridZ], 0), glm::vec3(1,
							heights[gridX + 1][gridZ], 0), glm::vec3(0,
							heights[gridX][gridZ + 1], 1), glm::vec2(xCoord, zCoord));
		} else {
			answer = Math::barryCentric(glm::vec3(1, heights[gridX + 1][gridZ], 0),glm::vec3(1,
							heights[gridX + 1][gridZ + 1], 1), glm::vec3(0,
							heights[gridX][gridZ + 1], 1),glm::vec2(xCoord, zCoord));
		}
        return answer;
}

std::shared_ptr<NileModel> ProceduralTerrain::generateTerrain(std::shared_ptr<MaterialPack> textures, std::string path)
{
    std::shared_ptr<NileTexture> heightMap = NULL;
    try
    {
       heightMap = NileTexture::createTextureFromFile(device, path);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    heights = std::vector<std::vector<float>> (VERTEX_COUNT, std::vector<float>(VERTEX_COUNT));
    std::vector<NileModel::Vertex> vertices(VERTEX_COUNT * VERTEX_COUNT);
    int count = VERTEX_COUNT * VERTEX_COUNT;

    std::vector<float> positions(count * 3);
    std::vector<float> color(count * 3);
    std::vector<float> normals(count * 3);
    std::vector<float> textureCoords(count * 2);

    std::vector<uint32_t> indices(6 * (VERTEX_COUNT - 1) * (VERTEX_COUNT - 1));

    int vertexPointer = 0;
    for(int i = 0; i < VERTEX_COUNT; i++){
			for(int j = 0; j < VERTEX_COUNT; j++){
				positions[vertexPointer * 3] = (float)j/((float)VERTEX_COUNT - 1) * SIZE;
				float height = getHeight(j, i, generator);
				heights[j][i] = height;
				positions[vertexPointer * 3 + 1] = height;
				positions[vertexPointer * 3 + 2] = (float)i/((float)VERTEX_COUNT - 1) * SIZE;
				glm::vec3 normal = calculateNormal(j, i, generator);
				normals[vertexPointer * 3] = normal.x;
				normals[vertexPointer * 3 + 1] = normal.y;
				normals[vertexPointer * 3 + 2] = normal.z;
				textureCoords[vertexPointer * 2] = (float) j / ((float)VERTEX_COUNT - 1);
				textureCoords[vertexPointer * 2 + 1] = (float) i / ((float)VERTEX_COUNT - 1);
				
			NileModel::Vertex vertex = {
                glm::vec3(
                -positions[vertexPointer * 3],
                -positions[vertexPointer * 3 + 1],
                -positions[vertexPointer * 3 + 2]
                ), 
                glm::vec3(1.f), 
                glm::vec3(
                -normals[vertexPointer * 3],
                -normals[vertexPointer * 3 + 1],
                -normals[vertexPointer * 3 + 2]
                        ), 
                glm::vec2(
                textureCoords[vertexPointer * 2],
                textureCoords[vertexPointer * 2 + 1]
                        )};	
				vertices[vertexPointer] = vertex;
				vertexPointer++;
			}
    }
    int pointer = 0;
    for(int gz=0;gz<VERTEX_COUNT-1;gz++){
        for(int gx=0;gx<VERTEX_COUNT-1;gx++){
            int topLeft = (gz*VERTEX_COUNT)+gx;
            int topRight = topLeft + 1;
            int bottomLeft = ((gz+1)*VERTEX_COUNT)+gx;
            int bottomRight = bottomLeft + 1;
            indices[pointer++] = topLeft;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = topRight;
            indices[pointer++] = topRight;
            indices[pointer++] = bottomLeft;
            indices[pointer++] = bottomRight;
        }
    }
    NileModel::Builder meshBuilder{};
    meshBuilder.vertices = vertices;
    meshBuilder.indices = indices;
    return std::make_shared<NileModel>(device, meshBuilder, textures);
}

glm::vec3 ProceduralTerrain::calculateNormal(int x, int z, std::shared_ptr<HeightsGenerator> generator)
{
    float heightL = getHeight(x-1, z, generator);
    float heightR = getHeight(x+1, z, generator);
    float heightD = getHeight(x, z-1, generator);
    float heightU = getHeight(x, z+1, generator);
    glm::vec3 normal = glm::vec3(heightL-heightR, 2.f, heightD - heightU);
    glm::normalize(normal);
    return normal;
}

float ProceduralTerrain::getHeight(int x, int z, std::shared_ptr<HeightsGenerator> generator)
{
    if (generator != nullptr) {
        return generator->generateHeight(x, z);
    }
    std::cout<<"Failed to generate height."<<std::endl;
    return 0;
}

} // namespace nile
