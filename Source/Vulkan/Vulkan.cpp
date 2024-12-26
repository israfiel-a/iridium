#include "Vulkan.hpp"
#include <Logging.hpp>
#include <vulkan/vulkan.hpp>

VkInstance instance = nullptr;
// VkPhysicalDevice graphics_card = nullptr;

const char *const names[] = {"VK_EXT_debug_utils", "VK_KHR_surface",
                             "VK_KHR_wayland_surface"};

// const char *const vnames[] = {"VK_LAYER_KHRONOS_validation"};

//! settings to add/remove vk validation layers at compile time

namespace Iridium::Vulkan
{
    bool Connect()
    {
        VkApplicationInfo info;
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info.pApplicationName = "SimpleWindow";
        info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        info.pEngineName = "Iridium";
        info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instance_info{};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &info;

        instance_info.enabledExtensionCount = 3;
        instance_info.ppEnabledExtensionNames = names;
        instance_info.enabledLayerCount = 0;
        // instance_info.ppEnabledLayerNames = vnames;

        VkResult result =
            vkCreateInstance(&instance_info, nullptr, &instance);
        if (result != VK_SUCCESS) exit(255);

        //! check for needed extensions / layers!!!

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                               nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                               extensions.data());

        // "Vulkan extension found"
        for (const auto &extension : extensions)
            Logging::Log((std::string) "VKEF: " + extension.extensionName);

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount,
                                           availableLayers.data());

        // "Vulkan validation layer found"
        for (const auto &layer : availableLayers)
            Logging::Log((std::string) "VVLF: " + layer.layerName);

        return true;
    }

    void Disconnect() { vkDestroyInstance(instance, nullptr); }
}
