#include "../rendering/render_system.hpp"

namespace nile {
    class ParticleGenerator
    {
    private:
        // state
        std::vector<NileGameObject*> particles;
        unsigned int amount;
        unsigned int lastUsedParticle = 0; // stores the index of the last particle used (for quick access to next dead particle)
        unsigned int firstUnusedParticle();
        void respawnParticle(ParticleComponent &particle, NileGameObject &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        // VkDescriptorImageInfo imageDescriptor;
        
        NileDevice& device;
        NileGameObjectManager& gom;
        std::unique_ptr<NilePipeline> nilePipeline;
        VkPipelineLayout pipelineLayout;

        std::unique_ptr<NileDescriptorSetLayout> renderSystemLayout;
        void init(NileGameObjectManager& gom);

    public:
        ParticleGenerator(
            NileDevice& device,
            NileGameObjectManager& gom,
            VkRenderPass renderPass,
            VkDescriptorSetLayout globalSetLayout, 
            unsigned int amount
        );
        ~ParticleGenerator();

        ParticleGenerator(const ParticleGenerator &) = delete;
        ParticleGenerator &operator=(const ParticleGenerator &) = delete;

        void update(
            float dt, 
            NileGameObject &object, 
            unsigned int newParticles,
            glm::vec2 offset = glm::vec2(0.0f, 0.0f)
            );

        void render(FrameInfo& frameInfo);
    };
    
}