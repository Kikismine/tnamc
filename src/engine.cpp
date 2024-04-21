//
// Created by kikismine on 4/20/24.
//

#include <engine.hpp>

struct Window window;

void errorCallback(int error, const char* description) {
    std::cerr << "glfw error: " << description << "\n";
}

static void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Engine::init() {
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
        throw std::runtime_error("failed to initialize glfw");
    else
        std::cout << "glfw initialized successfully\n";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window.size = {1500, 800};
    window.title = "vulkan";

    window.handle = glfwCreateWindow((int) window.size.width, (int) window.size.height, window.title.c_str(), nullptr, nullptr);

    if (!window.handle) {
        glfwDestroyWindow(window.handle);
        glfwTerminate();
        throw std::runtime_error("failed to create glfw window");
    }
    std::cout << std::unitbuf << "glfw window created successfully";

    glfwSetKeyCallback(window.handle, keyboardCallback);

    init_vulkan();
    init_swapchain();
    init_commands();
    init_sync_structures();

    // everything is fine
    is_init = true;
}

void Engine::init_vulkan() {
    vkb::InstanceBuilder builder;

    // make the vulkan instance
    auto inst_ret = builder.set_app_name("Vulkan Engine")
            .request_validation_layers(r_validation_layers)
            .use_default_debug_messenger()
            .require_api_version(1, 3, 0)
            .build();

    vkb::Instance vkb_inst = inst_ret.value();

    instance = vkb_inst.instance;
    debug_messenger = vkb_inst.debug_messenger;

    // create the surface from the surface variable on the glfw window
    VkResult err = glfwCreateWindowSurface(instance, window.handle, nullptr, &surface);
    if (err) {
        terminate();
        throw std::runtime_error("failed to create a window surface");
    }

    // vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    // vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    // vkb selects the best GPU
    // filter gpu(s) based on `features` (vulkan 1.3) and `features12` (vulkan 1.2)
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice vkb_physical_device = selector
            .set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .set_surface(surface)
            .select()
            .value();

    // create the vulkan (logical) device
    vkb::DeviceBuilder device_builder{vkb_physical_device};
    vkb::Device vkb_device = device_builder.build().value();

    // get the VkDevice handle from the `vkb_device`
    device = vkb_device.device;
    physical_device = vkb_physical_device.physical_device;
}

void Engine::create_swapchain(std::uint32_t width, std::uint32_t height) {
    vkb::SwapchainBuilder builder{physical_device, device, surface};

    swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;

    // set the swapchain builder params
    vkb::Swapchain vkb_swapchain = builder
//            .use_default_format_selection()
            .set_desired_format(VkSurfaceFormatKHR{.format = swapchain_image_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                    // vsync
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();

    swapchain_extent = vkb_swapchain.extent;

    // store swapchain and its images (or vectors)
    swapchain = vkb_swapchain.swapchain;
    swapchain_images = vkb_swapchain.get_images().value();
    swapchain_image_views = vkb_swapchain.get_image_views().value();
}

void Engine::destroy_swapchain() {
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    // destroy swapchain + its image(s)
    for (int i = 0; i < swapchain_image_views.size(); ++i) {
        vkDestroyImageView(device, swapchain_image_views[i], nullptr);
    }
}

void Engine::init_swapchain() {
    create_swapchain(window.size.width, window.size.height);
}

void Engine::init_commands() {

}

void Engine::init_sync_structures() {

}

void Engine::terminate() {
    if (is_init) {
        destroy_swapchain();

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);

        vkb::destroy_debug_utils_messenger(instance, debug_messenger);

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window.handle);
        glfwTerminate();
    }
}

void Engine::draw() {}

void Engine::run() {
    while (!glfwWindowShouldClose(window.handle)) {
        glfwPollEvents();
        draw();
    }
}