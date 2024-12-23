#include "particle_system.hpp"

namespace nile {

    struct ParticlePushConstants {
        glm::vec2 position{};
        alignas(16) glm::vec4 color{};
    };

    ParticleGenerator::ParticleGenerator(
        NileDevice& device,
        NileGameObjectManager& gom,
        VkRenderPass renderPass,
        VkDescriptorSetLayout offscreenSetLayout, 
        unsigned int amount
        )
        : device(device), gom(gom), amount(amount)
    {
        this->init(gom);
        createPipelineLayout(offscreenSetLayout);
        createPipeline(renderPass);
    }
    
    ParticleGenerator::~ParticleGenerator()
    {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void ParticleGenerator::init(
        NileGameObjectManager& gom)
    {
        NileModel::Builder meshBuilder{};
        meshBuilder.vertices = {
            // vertices (position, color)
            {{-0.5f, -0.5f, 0.0f}, glm::vec3(1.f)}, // bottom left
            {{0.5f, -0.5f, 0.0f}, glm::vec3(1.f)},  // bottom right
            {{0.5f, 0.5f, 0.0f}, glm::vec3(1.f)},   // top right
            {{-0.5f, 0.5f, 0.0f}, glm::vec3(1.f)}   // top left
        };
        // indices to form two triangles (bottom left, bottom right, top right) and (bottom left, top right, top left)
        meshBuilder.indices = {0, 1, 2, 0, 2, 3};
        auto square = std::make_shared<NileModel>(device, meshBuilder);

        // create this->amount default particle instances
        for (unsigned int i = 0; i < this->amount; ++i){
            auto& particle = gom.makeParticle(0.0f);
            particle.rigidBody2d.velocity = glm::vec2(0.0f);
            particle.model = square;
            particle.transform2d.scale = glm::vec3(.07f);
            this->particles.push_back(&particle);
        }
    }

    void ParticleGenerator::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ParticlePushConstants);

        renderSystemLayout = 
            NileDescriptorSetLayout::Builder(device)
                .addBinding(
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout, 
            renderSystemLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ParticleGenerator::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        NilePipeline::defaultPipelineConfigInfo(pipelineConfig);
        
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        nilePipeline = std::make_unique<NilePipeline>(
            device,
            "shaders/particle.vert.spv",
            "shaders/particle.frag.spv",
            pipelineConfig);
    }

    void ParticleGenerator::update(
            float dt, 
            NileGameObject &object, 
            unsigned int newParticles,
            glm::vec2 offset) {
        // add new particles
        for (unsigned int i = 0; i < newParticles; i++)
        {
            int unusedParticle = this->firstUnusedParticle();
            this->respawnParticle(
                *this->particles[unusedParticle]->particle, object, offset);
        }

        // update all particles
        for (unsigned int i = 0; i < this->amount; ++i)
        {
            ParticleComponent &p = *this->particles[i]->particle;
            p.Life -= dt; // reduce life
            if (p.Life > 0.0f)
            {	// particle is alive, thus update
               // std::cout<<"Updating particle..."<<std::endl;
                p.Position -= p.Velocity * dt; 
                p.Color.a -= dt * 2.5f;
            }
        }
    }

    unsigned int ParticleGenerator::firstUnusedParticle() {
        // first search from last used particle, this will usually return almost instantly
        for (unsigned int i = lastUsedParticle; i < this->amount; i++)
        {
            if (this->particles[i]->particle->Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
        // otherwise, do a linear search
        for (unsigned int i = 0; i < lastUsedParticle; i++)
        {
            if (this->particles[i]->particle->Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
        // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
        lastUsedParticle = 0;
        return 0;
    }

    void ParticleGenerator::respawnParticle(
        ParticleComponent &particle, 
        NileGameObject &object, 
        glm::vec2 offset) 
    {
        float random = ((rand() % 100) - 50) / 10.0f;
        float rColor = 0.5f + ((rand() % 100) / 100.0f);
        particle.Position = glm::vec2(object.transform2d.translation.x, 
                                object.transform2d.translation.y) + random + offset;
        particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
        particle.Life = 1.0f;
        particle.Velocity = object.rigidBody2d.velocity * 0.1f;
    }

    void ParticleGenerator::render(FrameInfo& frameInfo) {
        nilePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,  // starting set (0 is the globalDescriptorSet)
            1,  // set count
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        // Create a map to cache descriptor sets for each game object
        std::unordered_map<NileGameObject*, VkDescriptorSet> cachedDesriptorSets;

        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.model == nullptr || !obj.isParticle || obj.getIsHidden()) continue;
            if (obj.particle->Life > 0.0f)
            {
                
                // Check if the descriptor set is already cached
                auto it = cachedDesriptorSets.find(&obj);
                if (it != cachedDesriptorSets.end()) {
                    // If cached, bind the cached descriptor set
                    vkCmdBindDescriptorSets(
                        frameInfo.commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipelineLayout,
                        1,
                        1,  // set count
                        &it->second,
                        0,
                        nullptr);
                } else {
                    auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
                    auto diffuseMapInfo =  obj.diffuseMap->getImageInfo();
                    VkDescriptorSet gameObjectDescriptorSet;
                    NileDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
                        .writeBuffer(0, &bufferInfo)
                        .writeImage(1, &diffuseMapInfo)
                        .build(gameObjectDescriptorSet);
            
                    cachedDesriptorSets[&obj] = gameObjectDescriptorSet;

                    // Bind the newly created descriptor set
                    vkCmdBindDescriptorSets(
                        frameInfo.commandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipelineLayout,
                        1,
                        1,  // set count
                        &gameObjectDescriptorSet,
                        0,
                        nullptr);
                    }

                    ParticlePushConstants push{};
                    push.position = glm::vec2(obj.particle->Position);
                    push.color = glm::vec4(obj.particle->Color);

                    vkCmdPushConstants(
                        frameInfo.commandBuffer,
                        pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,
                        sizeof(ParticlePushConstants),
                        &push);
                    obj.model->bind(frameInfo.commandBuffer);
                    obj.model->draw(frameInfo.commandBuffer);
                }
        }         
    }
}