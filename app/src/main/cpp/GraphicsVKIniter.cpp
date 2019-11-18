#include "GraphicsVKIniter.h"
#include "vulkan_wrapper.h"
#include "HeapArray.h"
#include <vulkan/vulkan_android.h>
#include <vulkan/vk_platform.h>

GraphicsVKIniter::GraphicsVKIniter(uint32_t renderWidth, uint32_t renderHeight,
                                   View::ViewportType viewportType) : IGraphics(renderWidth, renderHeight), viewportType(viewportType)
{
}

bool GraphicsVKIniter::startGfx(ANativeWindow* nativeWindow)
{
    if(!InitVulkan())
        return false;

    if(!createInstance())
        return false;

	if (!createDebugExt())
		return false;

    gpu = getGPU();
    if(gpu == nullptr)
        return false;

    if(!createSurface(nativeWindow))
        return false;

    if(!createDevice(gpu, surface))
        return false;

    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &queue);

//    auto vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
//    auto vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
//    auto vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
//    auto vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
//    auto vkQueuePresentKHR = (PFN_vkQueuePresentKHR) vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
//    auto vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
//    auto vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
//    auto vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceSurfacePresentModesKHR");

    surfaceFormat = getSurfaceFormat(gpu, surface);
    if(surfaceFormat == VK_FORMAT_UNDEFINED)
        return false;

    VkPresentModeKHR presentMode = getPresentMode(gpu, surface);
    if(presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
        return false;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkResult vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);
    if(vkResult != VK_SUCCESS)
        return false;

    uint32_t minImageCount = getNumSwapchainImageBuffers(surfaceCapabilities);
    
	swapchainExtent = getSwapchainExtent(surfaceCapabilities);
	screenWidth = swapchainExtent.width;
	screenHeight = swapchainExtent.height;

    VkSurfaceTransformFlagBitsKHR transformFlag = getSwapchainTransformFlag(surfaceCapabilities);
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlag = getSwapchainAlphaFlag(surfaceCapabilities);

    if(!createSwapchain(minImageCount, surface, surfaceFormat, swapchainExtent, transformFlag, compositeAlphaFlag))
        return false;

    if(!getSwapchainImagesAndViews(surfaceFormat))
        return false;

    if(!createCommandPool())
        return false;

	renderPass = createRenderPass(surfaceFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    if(!createFramebuffers(swapchainExtent))
        return false;

    if(!createSemaphores())
        return false;

	if (!createPipelineCache())
		return false;

    return true;
}

void GraphicsVKIniter::stopGfx()
{
	if (pipelineCache != VK_NULL_HANDLE)
	{
		vkDestroyPipelineCache(device, pipelineCache, nullptr);
		pipelineCache = VK_NULL_HANDLE;
	}

    if(drawingCompleteSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(device, drawingCompleteSemaphore, nullptr);
        drawingCompleteSemaphore = VK_NULL_HANDLE;
    }
    if(presentCompleteSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(device, presentCompleteSemaphore, nullptr);
        presentCompleteSemaphore = VK_NULL_HANDLE;
    }

    for(uint32_t i = 0; i < NUM_BUFFERS; ++i)
    {
		if (framebuffers[i] != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
			framebuffers[i] = VK_NULL_HANDLE;
		}
    }

    if(renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }

    if(commandPool != VK_NULL_HANDLE)
    {
        //vkResetCommandPool(device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT || 0);
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }

    for(uint32_t i = 0; i < swapchainImageCount; ++i)
    {
		if (swapchainImageViews[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
			swapchainImageViews[i] = VK_NULL_HANDLE;
		}
    }
    swapchainImageCount = 0;

	if (swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
    if(surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(vkInstance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    if(device != nullptr)
    {
        vkDestroyDevice(device, nullptr);
        device = nullptr;
        queue = nullptr;
    }

	if (debugUtilsMessenger != VK_NULL_HANDLE)
	{
		auto vkDestroyDebugUtilsMessangerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
		assert(vkDestroyDebugUtilsMessangerEXT);
		vkDestroyDebugUtilsMessangerEXT(vkInstance, debugUtilsMessenger, nullptr);
	}

    if(vkInstance != nullptr)
    {
        vkDestroyInstance(vkInstance, nullptr);
        vkInstance = nullptr;
    }
}

bool GraphicsVKIniter::createInstance()
{
    uint32_t instanceLayerPropertyCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, nullptr);
    HeapArray<VkLayerProperties> instanceLayerProperties(instanceLayerPropertyCount, VkLayerProperties{});
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertyCount, instanceLayerProperties.data());

    const char* instanceLayerNames[] = { "VK_LAYER_KHRONOS_validation" };

    assert(arrayCount(instanceLayerNames) <= instanceLayerPropertyCount);

    for(uint32_t i = 0; i < arrayCount(instanceLayerNames); ++i)
    {
        bool found = false;
        for(auto it = instanceLayerProperties.begin(); it != instanceLayerProperties.end(); ++it)
        {
            if(String::createIneffectivlyFrom(instanceLayerNames[i]) == it->layerName)
            {
                found = true;
                break;
            }
        }

        if(!found)
            return false;
    }

    uint32_t instanceExtensionPropertyCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionPropertyCount, nullptr);
    HeapArray<VkExtensionProperties> instanceExtensionProperties(instanceExtensionPropertyCount, VkExtensionProperties{});
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionPropertyCount, instanceExtensionProperties.data());

    const char* instanceExtensionNames[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    assert(arrayCount(instanceExtensionNames) <= instanceExtensionPropertyCount);

    for(uint32_t i = 0; i < arrayCount(instanceExtensionNames); ++i)
    {
        bool found = false;
        for(auto it = instanceExtensionProperties.begin(); it != instanceExtensionProperties.end(); ++it)
        {
            if(String::createIneffectivlyFrom(instanceExtensionNames[i]) == it->extensionName)
            {
                found = true;
                break;
            }
        }

        if(!found)
            return false;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "AndroidEngine";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "AndroidEngine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = arrayCount(instanceLayerNames);
    createInfo.ppEnabledLayerNames = instanceLayerNames;
    createInfo.enabledExtensionCount = arrayCount(instanceExtensionNames);
    createInfo.ppEnabledExtensionNames = instanceExtensionNames;

    VkResult vkResult = vkCreateInstance(&createInfo, nullptr, &vkInstance);

    return (vkResult == VK_SUCCESS);
}

VkPhysicalDevice GraphicsVKIniter::getGPU() const
{
    uint32_t gpuDeviceCount;
    VkResult vkResult = vkEnumeratePhysicalDevices(vkInstance, &gpuDeviceCount, nullptr);

    if(vkResult != VK_SUCCESS)
        return nullptr;
    if(gpuDeviceCount == 0)
        return nullptr;

    HeapArray<VkPhysicalDevice> gpuList(gpuDeviceCount, nullptr);
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &gpuDeviceCount, gpuList.data());
    if(vkResult != VK_SUCCESS)
        return nullptr;

    //vkGetPhysicalDeviceProperties();
    //vkEnumerateDeviceExtensionProperties();

    VkPhysicalDevice result = gpuList[0];
    return result;
}

uint32_t GraphicsVKIniter::getMemoryTypeIndex(uint32_t memoryRequirementsIndexBits, VkMemoryPropertyFlags memoryPropertyFlags) const
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(gpu, &physicalDeviceMemoryProperties);
    for(uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i)
    {
		bool isIndexSet = (memoryRequirementsIndexBits >> i) & 1;
        if(isIndexSet && physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags)
        {
			return i;
        }
        //deviceSize size = physicalDeviceMemoryProperties.memoryHeaps[physicalDeviceMemoryProperties.memoryTypes[i].heapIndex].size;
    }

	InvalidCodePath;
	return 0;
}

bool GraphicsVKIniter::createDevice(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
    uint32_t queueFamilyPropertiesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, nullptr);
    if(queueFamilyPropertiesCount == 0)
    {
        return false;
    }
    HeapArray<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount, VkQueueFamilyProperties{});
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertiesCount, queueFamilyProperties.data());
    graphicsQueueFamilyIndex = (uint32_t)-1;
    presentQueueFamilyIndex = (uint32_t)-1;
    for(uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        VkBool32 hasPresentationSupport;
        VkResult vkResult = vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &hasPresentationSupport);
        if(vkResult != VK_SUCCESS)
            return false;

        if(hasPresentationSupport)
        {
            presentQueueFamilyIndex = i;
        }

        if(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamilyIndex = i;
        }

        if(presentQueueFamilyIndex != -1 && graphicsQueueFamilyIndex != -1)
        {
            break;
        }
    }

    if(presentQueueFamilyIndex == -1 && graphicsQueueFamilyIndex == -1)
        return false;

    float queuePriorities[] = { 0.0f };
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.flags = 0;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.pQueuePriorities = queuePriorities;
    queueCreateInfo.queueCount = arrayCount(queuePriorities);

    const char* deviceExtensioNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    //NOTE: Device layers are deprecated!
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = arrayCount(deviceExtensioNames);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensioNames;
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    VkResult vkResult = vkCreateDevice(gpu, &deviceCreateInfo, nullptr, &device);
    return (vkResult == VK_SUCCESS);
}

bool GraphicsVKIniter::createSurface(ANativeWindow* nativeWindow)
{
    auto vkCreateWin32SurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) vkGetInstanceProcAddr(vkInstance, "vkCreateAndroidSurfaceKHR");

    VkAndroidSurfaceCreateInfoKHR androidSurfaceCreateInfo = {};
    androidSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    androidSurfaceCreateInfo.pNext = nullptr;
    androidSurfaceCreateInfo.flags = 0;
    androidSurfaceCreateInfo.window = nativeWindow;

    VkResult vkResult = vkCreateWin32SurfaceKHR(vkInstance, &androidSurfaceCreateInfo, nullptr, &surface);

    return (vkResult == VK_SUCCESS);
}

VkFormat GraphicsVKIniter::getSurfaceFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface) const
{
    VkSurfaceFormatKHR result;

    uint32_t surfaceFormatCount;

    VkResult vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatCount, nullptr);
    if(vkResult != VK_SUCCESS)
        return VK_FORMAT_UNDEFINED;

    assert(surfaceFormatCount != 0);
    HeapArray<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount, VkSurfaceFormatKHR{});

    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatCount, surfaceFormats.data());
    if(vkResult != VK_SUCCESS)
        return VK_FORMAT_UNDEFINED;

    //NOTE: If format is not defined, one can choose one!
    if(surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
        result.format = VK_FORMAT_R8G8B8_UNORM;
    else
        result.format = surfaceFormats[0].format;

    return result.format;
}

VkPresentModeKHR GraphicsVKIniter::getPresentMode(VkPhysicalDevice gpu, VkSurfaceKHR surface) const
{
    uint32_t presentationModeCount;

    VkResult vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentationModeCount, nullptr);
    if(vkResult != VK_SUCCESS)
        return VK_PRESENT_MODE_MAX_ENUM_KHR;

    assert(presentationModeCount != 0);
    HeapArray<VkPresentModeKHR> presentationModes(presentationModeCount, VK_PRESENT_MODE_MAX_ENUM_KHR);

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentationModeCount, presentationModes.data());
    if(vkResult != VK_SUCCESS)
        return VK_PRESENT_MODE_MAX_ENUM_KHR;

    VkPresentModeKHR result = VK_PRESENT_MODE_FIFO_KHR;
    for(auto it = presentationModes.begin(); it != presentationModes.end(); ++it)
    {
        if((*it) == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            result = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        else if((*it) == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
        {
            result = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        }
    }

    return result;
}

bool GraphicsVKIniter::createSwapchain(uint32_t minImageCount, VkSurfaceKHR surface, VkFormat surfaceFormat,
        VkExtent2D swapchainExtent, VkSurfaceTransformFlagBitsKHR transformFlag, VkCompositeAlphaFlagBitsKHR alphaBit)
{
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.minImageCount = minImageCount;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.imageFormat = surfaceFormat;
    swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.preTransform = transformFlag;
    swapchainCreateInfo.compositeAlpha = alphaBit;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    uint32_t queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
    if(graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
		//TODO: In this case I have to insert an image memory barrier in the rendering process!
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    VkResult vkResult = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);

    return (vkResult == VK_SUCCESS);
}

bool GraphicsVKIniter::getSwapchainImagesAndViews(VkFormat surfaceFormat)
{
    VkResult vkResult = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
    if(vkResult != VK_SUCCESS)
        return false;

    assert(swapchainImageCount == NUM_BUFFERS);
    vkResult = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);
    if(vkResult != VK_SUCCESS)
        return false;

    for(uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.format = surfaceFormat;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.image = swapchainImages[i];

        vkResult = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
        if(vkResult != VK_SUCCESS)
            return false;
    }

    return true;
}

uint32_t GraphicsVKIniter::getNumSwapchainImageBuffers(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    //Number of front- and backbuffers
    uint32_t result = NUM_BUFFERS;
    if(result > surfaceCapabilities.maxImageCount)
    {
        result = surfaceCapabilities.maxImageCount;
    }
    assert(result >= surfaceCapabilities.minImageCount);

    return result;
}

VkExtent2D GraphicsVKIniter::getSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    VkExtent2D result;

    if(surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
    {
        result = surfaceCapabilities.maxImageExtent;
    }
    else
    {
        result = surfaceCapabilities.currentExtent;
    }

    return result;
}

VkSurfaceTransformFlagBitsKHR GraphicsVKIniter::getSwapchainTransformFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    VkSurfaceTransformFlagBitsKHR result;

    if(surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        result = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        result = surfaceCapabilities.currentTransform;

    return result;
}

VkCompositeAlphaFlagBitsKHR GraphicsVKIniter::getSwapchainAlphaFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const
{
    VkCompositeAlphaFlagBitsKHR result = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    //NOTE: One of these is set!
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++)
    {
        if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
        {
            result = compositeAlphaFlags[i];
            break;
        }
    }

    return result;
}

bool GraphicsVKIniter::createCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT that buffers form this pool will change frequently
    VkResult vkResult = vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);

    return (vkResult == VK_SUCCESS);
}

VkRenderPass GraphicsVKIniter::createRenderPass(VkFormat surfaceFormat, VkImageLayout finalLayout)
{
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = surfaceFormat;
    attachmentDescription.flags = 0;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = finalLayout;

    VkAttachmentReference attachmentReference = {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentReference;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 0;
    renderPassCreateInfo.pDependencies = nullptr;

	VkRenderPass result;
    VkResult vkResult = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &result);
	assert(vkResult == VK_SUCCESS);

	return result;
}

bool GraphicsVKIniter::createFramebuffers(VkExtent2D swapchainExtent)
{
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.width = swapchainExtent.width;
    framebufferCreateInfo.height = swapchainExtent.height;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.layers = 1;

    for(uint32_t i = 0; i < NUM_BUFFERS; ++i)
    {
        framebufferCreateInfo.pAttachments = &swapchainImageViews[i];
        VkResult vkResult = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]);
        if(vkResult != VK_SUCCESS)
            return false;
    }

    return true;
}

VkPipelineVertexInputStateCreateInfo GraphicsVKIniter::createPipelineVertexInputState(const VulkanVertexLayout& vertexLayout) const
{
    VkPipelineVertexInputStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.vertexBindingDescriptionCount = 1;
    result.pVertexBindingDescriptions = &vertexLayout.binding;
    result.vertexAttributeDescriptionCount = vertexLayout.attributes.size();
    result.pVertexAttributeDescriptions = vertexLayout.attributes.data();

    return result;
}

VkPipelineInputAssemblyStateCreateInfo GraphicsVKIniter::createPipelineInputAssemblyState() const
{
    VkPipelineInputAssemblyStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    //Can just be used with indexed drawing to seperate geomitry which was batched into one index array
    result.primitiveRestartEnable = VK_FALSE;
    result.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    return result;
}

VkPipelineRasterizationStateCreateInfo GraphicsVKIniter::createPipelineRasterizationState() const
{
    VkPipelineRasterizationStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.polygonMode = VK_POLYGON_MODE_FILL;
    result.cullMode = VK_CULL_MODE_NONE;
    result.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    result.depthClampEnable = VK_FALSE;
    result.rasterizerDiscardEnable = VK_FALSE;
    result.depthBiasEnable = VK_FALSE;
    result.depthBiasClamp = 0;
    result.depthBiasConstantFactor = 0;
    result.depthBiasSlopeFactor = 0;
    result.lineWidth = 1.0f;

    return result;
}

VkPipelineColorBlendStateCreateInfo GraphicsVKIniter::createPipelineColorBlendState(VkPipelineColorBlendAttachmentState* attachmentState) const
{
    attachmentState->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
    attachmentState->blendEnable = VK_TRUE;
    attachmentState->colorBlendOp = VK_BLEND_OP_ADD;
    attachmentState->alphaBlendOp = VK_BLEND_OP_ADD;
    attachmentState->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachmentState->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachmentState->dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachmentState->dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    VkPipelineColorBlendStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.attachmentCount = 1;
    result.pAttachments = attachmentState;
    result.logicOpEnable = VK_FALSE;
    result.blendConstants[0] = 1.0f;
    result.blendConstants[1] = 1.0f;
    result.blendConstants[2] = 1.0f;
    result.blendConstants[3] = 1.0f;

    return result;
}

VkPipelineViewportStateCreateInfo GraphicsVKIniter::createPipelineViewportState(VkViewport* viewport, VkRect2D* scissor) const
{
    viewport->x = 0;
    viewport->y = 0;
    viewport->width = (float)view.getViewportSize().x;
    viewport->height = (float)view.getViewportSize().y;
    viewport->minDepth = 0.0f;
    viewport->maxDepth = 1.0f;

	scissor->extent = VkExtent2D{view.getViewportSize().x, view.getViewportSize().y};
	scissor->offset = VkOffset2D{ 0, 0 };

    VkPipelineViewportStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.scissorCount = 1;
    result.pScissors = scissor;
    result.viewportCount = 1;
    result.pViewports = viewport;

    return result;
}

VkPipelineMultisampleStateCreateInfo GraphicsVKIniter::createPipelineMultisampleState() const
{
    VkPipelineMultisampleStateCreateInfo result = {};

    result.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    result.pNext = nullptr;
    result.flags = 0;
    result.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    result.sampleShadingEnable = VK_FALSE;
    result.minSampleShading = 0;
    result.pSampleMask = 0;
    result.alphaToCoverageEnable = VK_FALSE;
    result.alphaToOneEnable = VK_FALSE;

    return result;
}

bool GraphicsVKIniter::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    VkResult vkResult = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
    if(vkResult != VK_SUCCESS)
        return false;

    vkResult = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &drawingCompleteSemaphore);

    return (vkResult == VK_SUCCESS);
}

bool GraphicsVKIniter::createPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheCreateInfo.pNext = nullptr;
	pipelineCacheCreateInfo.flags = 0;
	pipelineCacheCreateInfo.initialDataSize = 0;
	pipelineCacheCreateInfo.pInitialData = nullptr;

	VkResult vkResult = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
	return (vkResult == VK_SUCCESS);
}

VkPipeline GraphicsVKIniter::createPipeline(const VulkanShader& shader,
                                            const VulkanDescriptorSetLayout& descriptorSetLayout,
                                            VkRenderPass renderPass)
{
    auto vertexInputState = createPipelineVertexInputState(shader.vertexLayout);
    auto inputAssemblyState = createPipelineInputAssemblyState();
	//TODO: This has to be done better!
	VkViewport viewport = {};
	VkRect2D scissor = {};
    auto viewportState = createPipelineViewportState(&viewport, &scissor);
    auto rasterizationState = createPipelineRasterizationState();
    auto multisampleState = createPipelineMultisampleState();
	VkPipelineColorBlendAttachmentState attachmentState = {};
    auto colorBlendState = createPipelineColorBlendState(&attachmentState);

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pNext = nullptr;
    graphicsPipelineCreateInfo.flags = 0;
    graphicsPipelineCreateInfo.stageCount = shader.getStageCount();
    graphicsPipelineCreateInfo.pStages = shader.getStages();
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    graphicsPipelineCreateInfo.pTessellationState = nullptr;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
    graphicsPipelineCreateInfo.pDynamicState = nullptr;
    graphicsPipelineCreateInfo.layout = descriptorSetLayout.pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline result;
    VkResult vkResult = vkCreateGraphicsPipelines(device, pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &result);
	assert(vkResult == VK_SUCCESS);

    return result;
}

VkSampler GraphicsVKIniter::createSampler()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.maxAnisotropy = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.25f;

	VkSampler result;
	vkCreateSampler(device, &samplerCreateInfo, nullptr, &result);

	return result;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                                void *userData)
{
    const char* typeString = "";
    const char* severityString = "";
    const char* message = callbackData->pMessage;

    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        severityString = "ERROR";
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        severityString = "WARNING";
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        severityString = "INFO";
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        severityString = "VERBOSE";
    }
    if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    {
        typeString = "Validation";
    }
    else if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    {
        typeString = "Performance";
    }
    else if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
    {
        typeString = "General";
    }

    utils::logF("%s %s:", typeString, severityString);
	utils::log(message);

	if (typeString == String("Validation"))
		InvalidCodePath;

    // Returning false tells the layer not to stop when the event occurs, so
    // they see the same behavior with and without validation layers enabled.
    return VK_FALSE;
}

bool GraphicsVKIniter::createDebugExt()
{
    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    assert(vkCreateDebugUtilsMessengerEXT);

    VkDebugUtilsMessengerCreateInfoEXT  debugUtilsMessengerCreateInfo = {};
    debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCreateInfo.pNext = nullptr;
    debugUtilsMessengerCreateInfo.flags = 0;
    debugUtilsMessengerCreateInfo.pUserData = nullptr;
    debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugUtilsMessengerCreateInfo.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debugUtilsMessengerCreateInfo.pfnUserCallback = VulkanDebugUtilsMessenger;

    VkResult vkResult = vkCreateDebugUtilsMessengerEXT(vkInstance, &debugUtilsMessengerCreateInfo, nullptr, &debugUtilsMessenger);

    return (vkResult == VK_SUCCESS);
}
