#include "Vulkan.hpp"
#include "./Wayland.hpp"
#include <Logging.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_wayland.h>

using GPU = VkPhysicalDevice;
using GPUProperties = VkPhysicalDeviceProperties;
using GPUFeatures = VkPhysicalDeviceFeatures;
using ScoreType = std::uint8_t;

static vk::Instance instance;
VkPhysicalDevice gpu_device;
VkDevice gpu_logical_device;

VkCommandPool command_pool;
VkRenderPass renderpass;
VkSwapchainKHR swapchain;
VkSurfaceKHR surface;
VkQueue queue;
VkFormat format;
uint32_t image_count = 0;
uint32_t queue_family_index = 0;
VkDebugUtilsMessengerEXT debugMessenger;

uint32_t current_frame = 0;
uint32_t image_index = 0;

struct SwapchainElement
{
    public:
        VkCommandBuffer commandBuffer;
        VkImage image;
        VkImageView imageView;
        VkFramebuffer framebuffer;
        VkSemaphore startSemaphore;
        VkSemaphore endSemaphore;
        VkFence fence;
        VkFence lastFence;
};
struct SwapchainElement *elements = NULL;

static constexpr std::array<const char *, 3> required_extensions = {
    vk::EXTDebugUtilsExtensionName, vk::KHRSurfaceExtensionName,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME};

static constexpr std::array<const char *, 1> required_layers = {
    "VK_LAYER_KHRONOS_validation"};

const char *const dnames[] = {"VK_KHR_swapchain"};

//! settings to add/remove vk validation layers at compile time
//! check queue families properly
//! check like all return values

ScoreType RateGPU(GPU device)
{
    GPUProperties device_properties;
    GPUFeatures device_features;

    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    ScoreType device_score = 1;
    Iridium::Logging::Log((std::string) "Physical device: " +
                          device_properties.deviceName);

    //! Add more / an interface to register requirements
    if (!device_features.geometryShader) return 0;

    return device_score;
}

#define GET_EXTENSION_FUNCTION(_id)                                       \
    ((PFN_##_id)(vkGetInstanceProcAddr(instance, #_id)))

static VkBool32
onError(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void *userData)
{
    printf("Vulkan ");

    switch (type)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            printf("general ");
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            printf("validation ");
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            printf("performance ");
            break;
    }

    switch (severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            printf("(verbose): ");
            break;
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            printf("(info): ");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            printf("(warning): ");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            printf("(error): ");
            break;
    }

    printf("%s\n", callbackData->pMessage);

    return 0;
}

namespace Iridium::Vulkan
{
    bool Connect(const std::string &application_name)
    {
        if (instance != nullptr)
        {
            Logging::RaiseError(Logging::double_init);
            return false;
        }

        vk::ApplicationInfo application_info(
            "application_name.c_str()", VK_MAKE_VERSION(1, 0, 0),
            "Iridium", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_4);
        vk::InstanceCreateInfo instance_info(
            {}, &application_info, required_layers.size(),
            required_layers.data(), required_extensions.size(),
            required_extensions.data());

        std::uint32_t extension_count = 0;
        vk::Result failure = vk::enumerateInstanceExtensionProperties(
            nullptr, &extension_count, nullptr);
        if (failure != vk::Result::eSuccess)
        {
            Logging::RaiseError(
                Logging::enumeration_failure, Logging::infer,
                "Failed to enumerate Vulkan extensions. Code: " +
                    std::to_string((std::uint64_t)failure));
            return false;
        }

        std::vector<vk::ExtensionProperties> found_extensions(
            extension_count);
        failure = vk::enumerateInstanceExtensionProperties(
            nullptr, &extension_count, found_extensions.data());
        if (failure != vk::Result::eSuccess)
        {
            Logging::RaiseError(
                Logging::enumeration_failure, Logging::infer,
                "Failed to enumerate Vulkan extensions. Code: " +
                    std::to_string((std::uint64_t)failure));
            return false;
        }

        std::uint32_t found_required_extensions = 0;
        for (const auto &extension : found_extensions)
        {
            for (std::size_t i = 0; i < required_extensions.size(); i++)
                if (strcmp(extension.extensionName.data(),
                           required_extensions[i]) == 0)
                {
                    Logging::Log("Got extension " +
                                     (std::string)extension.extensionName +
                                     ".",
                                 Logging::success);
                    found_required_extensions++;
                    break;
                }
        }

        if (found_required_extensions < required_extensions.size())
        {
            Logging::RaiseError(
                Logging::failed_null_assertion, Logging::infer,
                "Failed to find required Vulkan extensions.");
            return false;
        }

        std::uint32_t layer_count = 0;
        failure =
            vk::enumerateInstanceLayerProperties(&layer_count, nullptr);
        if (failure != vk::Result::eSuccess)
        {
            Logging::RaiseError(
                Logging::enumeration_failure, Logging::infer,
                "Failed to enumerate Vulkan layers. Code: " +
                    std::to_string((std::uint64_t)failure));
            return false;
        }

        std::vector<vk::LayerProperties> found_layers(layer_count);
        failure = vk::enumerateInstanceLayerProperties(
            &layer_count, found_layers.data());
        if (failure != vk::Result::eSuccess)
        {
            Logging::RaiseError(
                Logging::enumeration_failure, Logging::infer,
                "Failed to enumerate Vulkan layers. Code: " +
                    std::to_string((std::uint64_t)failure));
            return false;
        }

        std::uint32_t found_required_layers = 0;
        for (const auto &layer : found_layers)
        {
            for (std::size_t i = 0; i < required_layers.size(); i++)
                if (strcmp(layer.layerName.data(), required_layers[i]) ==
                    0)
                {
                    Logging::Log("Got layer " +
                                     (std::string)layer.layerName + ".",
                                 Logging::success);
                    found_required_layers++;
                    break;
                }
        }

        if (found_required_layers < required_layers.size())
        {
            Logging::RaiseError(Logging::failed_null_assertion,
                                Logging::infer,
                                "Failed to find required Vulkan layers.");
            return false;
        }

        instance = vk::createInstance(instance_info, nullptr);

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = onError;

        GET_EXTENSION_FUNCTION(vkCreateDebugUtilsMessengerEXT)
        (instance, &createInfo, NULL, &debugMessenger);

        VkWaylandSurfaceCreateInfoKHR wayland_create_info = {};
        wayland_create_info.sType =
            VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        wayland_create_info.display = Windowing::Wayland::GetDisplay();
        wayland_create_info.surface = Windowing::Wayland::GetSurface();
        vkCreateWaylandSurfaceKHR(instance, &wayland_create_info, NULL,
                                  &surface);

        uint32_t physDeviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &physDeviceCount, NULL);

        std::vector<VkPhysicalDevice> physDevices(physDeviceCount);
        vkEnumeratePhysicalDevices(instance, &physDeviceCount,
                                   physDevices.data());

        uint32_t bestScore = 0;

        for (uint32_t i = 0; i < physDeviceCount; i++)
        {
            VkPhysicalDevice device = physDevices[i];

            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            uint32_t score;

            switch (properties.deviceType)
            {
                default:                            continue;
                case VK_PHYSICAL_DEVICE_TYPE_OTHER: score = 1; break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    score = 4;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    score = 5;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: score = 3; break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:         score = 2; break;
            }

            if (score > bestScore)
            {
                gpu_device = device;
                bestScore = score;
            }
        }

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu_device,
                                                 &queueFamilyCount, NULL);

        std::vector<VkQueueFamilyProperties> queueFamilies(
            queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            gpu_device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            VkBool32 present = 0;

            vkGetPhysicalDeviceSurfaceSupportKHR(gpu_device, i, surface,
                                                 &present);

            if (present &&
                (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                queue_family_index = i;
                break;
            }
        }

        float priority = 1;

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queue_family_index;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priority;

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &queueCreateInfo;
        device_create_info.enabledExtensionCount = 1;
        device_create_info.ppEnabledExtensionNames = dnames;
        device_create_info.enabledLayerCount = 0;

        // uint32_t deviceLayerCount;
        // vkEnumerateDeviceLayerProperties(gpu_device, &deviceLayerCount,
        //                                  NULL);

        // VkLayerProperties *layerProperties = (VkLayerProperties
        // *)malloc(
        //     deviceLayerCount * sizeof(VkLayerProperties));
        // vkEnumerateDeviceLayerProperties(gpu_device, &deviceLayerCount,
        //                                  layerProperties);

        // size_t foundLayers = 0;

        // for (uint32_t i = 0; i < deviceLayerCount; i++)
        // {
        //     for (size_t j = 0;
        //          j < sizeof(layerNames) / sizeof(const char *); j++)
        //     {
        //         if (strcmp(layerProperties[i].layerName, layerNames[j])
        //         ==
        //             0)
        //         {
        //             foundLayers++;
        //         }
        //     }
        // }

        // free(layerProperties);

        // if (foundLayers >= sizeof(layerNames) / sizeof(const char *))
        // {
        //     createInfo.enabledLayerCount =
        //         sizeof(layerNames) / sizeof(const char *);
        //     createInfo.ppEnabledLayerNames = layerNames;
        // }

        vkCreateDevice(gpu_device, &device_create_info, NULL,
                       &gpu_logical_device);
        vkGetDeviceQueue(gpu_logical_device, queue_family_index, 0,
                         &queue);

        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = queue_family_index;
        command_pool_create_info.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        vkCreateCommandPool(gpu_logical_device, &command_pool_create_info,
                            NULL, &command_pool);

        StartSwapchain();

        return true;
    }

    void Disconnect()
    {
        WaitForIdle();
        EndSwapchain();
        vkDestroyCommandPool(gpu_logical_device, command_pool, NULL);
        vkDestroyDevice(gpu_logical_device, NULL);
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyInstance(instance, nullptr);
    }

    void StartSwapchain()
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu_device, surface,
                                                  &capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu_device, surface,
                                             &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu_device, surface,
                                             &formatCount, formats.data());

        VkSurfaceFormatKHR chosenFormat = formats[0];

        for (uint32_t i = 0; i < formatCount; i++)
        {
            if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                chosenFormat = formats[i];
                break;
            }
        }

        format = chosenFormat.format;

        image_count =
            capabilities.minImageCount + 1 < capabilities.maxImageCount
                ? capabilities.minImageCount + 1
                : capabilities.minImageCount;

        const Windowing::Wayland::Monitor &monitor =
            Windowing::Wayland::GetMonitor();

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = image_count;
        createInfo.imageFormat = chosenFormat.format;
        createInfo.imageColorSpace = chosenFormat.colorSpace;
        createInfo.imageExtent.width = monitor.width;
        createInfo.imageExtent.height = monitor.height;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        createInfo.clipped = 1;

        vkCreateSwapchainKHR(gpu_logical_device, &createInfo, nullptr,
                             &swapchain);

        VkAttachmentDescription attachment = {0};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference attachmentRef = {0};
        attachmentRef.attachment = 0;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {0};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentRef;

        VkRenderPassCreateInfo renderpass_create_info{};
        renderpass_create_info.sType =
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.flags = 0;
        renderpass_create_info.attachmentCount = 1;
        renderpass_create_info.pAttachments = &attachment;
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = &subpass;

        vkCreateRenderPass(gpu_logical_device, &renderpass_create_info,
                           NULL, &renderpass);
        vkGetSwapchainImagesKHR(gpu_logical_device, swapchain,
                                &image_count, NULL);

        std::vector<VkImage> images(image_count);
        vkGetSwapchainImagesKHR(gpu_logical_device, swapchain,
                                &image_count, images.data());

        elements = (SwapchainElement *)malloc(image_count *
                                              sizeof(SwapchainElement));

        for (uint32_t i = 0; i < image_count; i++)
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType =
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = command_pool;
            allocInfo.commandBufferCount = 1;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            vkAllocateCommandBuffers(gpu_logical_device, &allocInfo,
                                     &elements[i].commandBuffer);

            elements[i].image = images[i];

            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            createInfo.image = elements[i].image;
            createInfo.format = format;
            createInfo.subresourceRange.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;

            vkCreateImageView(gpu_logical_device, &createInfo, NULL,
                              &elements[i].imageView);

            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType =
                VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = renderpass;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = &elements[i].imageView;
            framebuffer_create_info.width = monitor.width;
            framebuffer_create_info.height = monitor.height;
            framebuffer_create_info.layers = 1;

            vkCreateFramebuffer(gpu_logical_device,
                                &framebuffer_create_info, NULL,
                                &elements[i].framebuffer);

            VkSemaphoreCreateInfo semaphore_create_info{};
            semaphore_create_info.sType =
                VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            vkCreateSemaphore(gpu_logical_device, &semaphore_create_info,
                              NULL, &elements[i].startSemaphore);

            VkSemaphoreCreateInfo semaphore_create_info_2 = {};
            semaphore_create_info_2.sType =
                VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            vkCreateSemaphore(gpu_logical_device, &semaphore_create_info_2,
                              NULL, &elements[i].endSemaphore);

            VkFenceCreateInfo fence_create_info{};
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            vkCreateFence(gpu_logical_device, &fence_create_info, NULL,
                          &elements[i].fence);

            elements[i].lastFence = VK_NULL_HANDLE;
        }
    }

    void EndSwapchain()
    {
        for (uint32_t i = 0; i < image_count; i++)
        {
            vkDestroyFence(gpu_logical_device, elements[i].fence, NULL);
            vkDestroySemaphore(gpu_logical_device,
                               elements[i].endSemaphore, NULL);
            vkDestroySemaphore(gpu_logical_device,
                               elements[i].startSemaphore, NULL);
            vkDestroyFramebuffer(gpu_logical_device,
                                 elements[i].framebuffer, NULL);
            vkDestroyImageView(gpu_logical_device, elements[i].imageView,
                               NULL);
            vkFreeCommandBuffers(gpu_logical_device, command_pool, 1,
                                 &elements[i].commandBuffer);
        }

        free(elements);
        vkDestroyRenderPass(gpu_logical_device, renderpass, NULL);
        vkDestroySwapchainKHR(gpu_logical_device, swapchain, NULL);

        current_frame = 0;
        image_index = 0;
    }

    void WaitForIdle() { vkDeviceWaitIdle(gpu_logical_device); }

    void Frame()
    {
        struct SwapchainElement *currentElement = &elements[current_frame];

        vkWaitForFences(gpu_logical_device, 1, &currentElement->fence, 1,
                        UINT64_MAX);
        VkResult result = vkAcquireNextImageKHR(
            gpu_logical_device, swapchain, UINT64_MAX,
            currentElement->startSemaphore, NULL, &image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR ||
            result == VK_SUBOPTIMAL_KHR)
        {
            vkDeviceWaitIdle(gpu_logical_device);
            EndSwapchain();
            StartSwapchain();
            return;
        }
        // else if (result < 0) { CHECK_VK_RESULT(result); }

        struct SwapchainElement *element = &elements[image_index];

        if (element->lastFence)
        {
            vkWaitForFences(gpu_logical_device, 1, &element->lastFence, 1,
                            UINT64_MAX);
        }

        element->lastFence = currentElement->fence;

        vkResetFences(gpu_logical_device, 1, &currentElement->fence);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(element->commandBuffer, &beginInfo);

        //! BACKGROUND COLOR!!!!
        VkClearValue clearValue = {{{1.0f, 0.0f, 1.0f, 1.0f}}};

        const Windowing::Wayland::Monitor &monitor =
            Windowing::Wayland::GetMonitor();

        VkRenderPassBeginInfo renderpass_info = {};
        renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_info.renderPass = renderpass;
        renderpass_info.framebuffer = element->framebuffer;
        renderpass_info.renderArea.offset.x = 0;
        renderpass_info.renderArea.offset.y = 0;
        renderpass_info.renderArea.extent.width = monitor.width;
        renderpass_info.renderArea.extent.height = monitor.height;
        renderpass_info.clearValueCount = 1;
        renderpass_info.pClearValues = &clearValue;

        vkCmdBeginRenderPass(element->commandBuffer, &renderpass_info,
                             VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(element->commandBuffer);
        vkEndCommandBuffer(element->commandBuffer);

        const VkPipelineStageFlags waitStage =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &currentElement->startSemaphore;
        submitInfo.pWaitDstStageMask = &waitStage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &element->commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &currentElement->endSemaphore;

        vkQueueSubmit(queue, 1, &submitInfo, currentElement->fence);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &currentElement->endSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &image_index;

        result = vkQueuePresentKHR(queue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR ||
            result == VK_SUBOPTIMAL_KHR)
        {
            vkDeviceWaitIdle(gpu_logical_device);
            EndSwapchain();
            StartSwapchain();
        }
        // else if (result < 0) { CHECK_VK_RESULT(result); }

        current_frame = (current_frame + 1) % image_count;
    }
}
