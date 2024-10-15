#include "apps/app.hpp"

namespace nile{
    class Skybox : public App3D {

    public:
    Skybox();
    ~Skybox() override;

    void loop() override;
    void start() override;

    private:
    void loadGameObjects() override;

    };

    Skybox::Skybox() {}

    Skybox::~Skybox() {}

    void Skybox::start() {
        loadGameObjects();
        loop();
    }

    void Skybox::loop() {

        RenderSystem3D renderSystem3D{
            nileDevice,
            nileRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()};

        auto& viewerObject = gameObjectManager.createGameObject();

        viewerObject.transform.translation.z = -2.5f;

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!nileWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();

            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXYZ(nileWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = nileRenderer.getAspectRatio();

            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            ui.startUI();

            if (auto commandBuffer = nileRenderer.beginFrame())
            {
                int frameIndex = nileRenderer.getFrameIndex();
                framePools[frameIndex]->resetPool();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    *framePools[frameIndex],
                    gameObjectManager.gameObjects
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                gameObjectManager.updateBuffer(frameIndex);

                nileRenderer.beginSwapChainRenderPass(commandBuffer);

                renderSystem3D.renderGameObjects(frameInfo);

                ui.renderUI(commandBuffer, nileRenderer);

                nileRenderer.endSwapChainRenderPass(commandBuffer);
                nileRenderer.endFrame();
                
            }
        }
        vkDeviceWaitIdle(nileDevice.device());
        }

        void Skybox::loadGameObjects() {
            std::shared_ptr<NileModel> floor_mesh =
                NileModel::createModelFromFile(nileDevice, "resources/models/quad.obj");
            auto& floor = gameObjectManager.createGameObject();
            floor.model = floor_mesh;
            floor.transform.translation = {0.f, .25f, 0.f};
            floor.transform.scale = {3.f, 1.f, 3.f};

        }


    } // namespace nile