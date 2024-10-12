#include "simple_ui.hpp"

namespace nile {
    SimpleUI::SimpleUI(
        NileDevice &device,  
        GLFWwindow* window,
        VkRenderPass renderPass, 
        size_t imageCount)
        : 
    mDevice{device},
    mWindow{window},
    mRenderPass{renderPass},
    mImageCount{imageCount},
    io( [] -> ImGuiIO& {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        return ImGui::GetIO();
        }())
    {
        init();
    }

    SimpleUI::~SimpleUI() {
        // Cleanup
        err = vkDeviceWaitIdle(mDevice.device());
        check_vk_result(err);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
		vkDestroyDescriptorPool(mDevice.device(), imguiPool, nullptr);
    }

    void SimpleUI::init() {
        //1: Create descriptor pool for IMGUI
        VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; 
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(mDevice.device(), &pool_info, nullptr, &imguiPool);
        check_vk_result(err);

        
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls 

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(mWindow, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = mDevice.getInstance();
        init_info.PhysicalDevice = mDevice.getPhysicalDevice();
        init_info.Device = mDevice.device();
        init_info.QueueFamily = indices.graphicsFamily;
        init_info.Queue = mQueue;
        init_info.PipelineCache = mPipelineCache;
        init_info.DescriptorPool = imguiPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = mImageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, mRenderPass);
    }

    void SimpleUI::loadFonts() {
        //         Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // io.Fonts->AddFontDefault();
        // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        // IM_ASSERT(font != nullptr);
    }

    void SimpleUI::check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }


void SimpleUI::startUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Scene config");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / this->getFrameRate(), this->getFrameRate());

        ImGui::Checkbox("Settings", &show_demo_window);      // Edit bools storing our window open/close state
        // ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::Checkbox("Point Lights", &enable_point_lights);
        ImGui::Checkbox("Terrain", &set_show_model);

        ImGui::SliderFloat("Terrain translate x", &terrain_pos.x, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderFloat("Terrain translate y", &terrain_pos.y, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderFloat("Terrain translate z", &terrain_pos.z, 0.0f, 10.0f); 

        // ImGui::SliderFloat("Terrain rotate x-axis", &terrain_rot.x, 0.0f, 10.0f);
        ImGui::SliderFloat("Terrain rotate y", &terrain_rot.y, 0.0f, 10.0f);
        // ImGui::SliderFloat("Terrain rotate z-axis", &terrain_rot.z, 0.0f, 10.0f);

        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

void SimpleUI::renderUI(VkCommandBuffer commandBuffer, NileRenderer& renderer) {
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized)
    {   
      renderer.clearValues[0].color.float32[0] = clear_color.x * clear_color.w;
      renderer.clearValues[0].color.float32[1] = clear_color.y * clear_color.w;
      renderer.clearValues[0].color.float32[2] = clear_color.z * clear_color.w;
      renderer.clearValues[0].color.float32[3] = clear_color.w;
      // Record dear imgui primitives into command buffer
      ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    }
}

}
