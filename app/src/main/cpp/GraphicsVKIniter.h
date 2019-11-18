#pragma once

#include "IGraphics.h"
#include "vulkan_wrapper.h"
#include <android/native_window.h>
#include "CommandBuffer.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanVertexLayout.h"
#include "VulkanShader.h"

class GraphicsVKIniter : public IGraphics
{
public:
	VkDevice device = nullptr;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	uint32_t graphicsQueueFamilyIndex = (uint32_t)-1;
	//TODO: Maybe let other classes choose their own queue!
	VkQueue queue = nullptr;
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;
protected:
    View::ViewportType viewportType;
    View view;
    VkInstance vkInstance = nullptr;
    VkPhysicalDevice gpu = nullptr;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    static constexpr uint32_t NUM_BUFFERS = 2;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    uint32_t swapchainImageCount = 0;
	VkImageView swapchainImageViews[NUM_BUFFERS] = { VK_NULL_HANDLE };
	VkImage swapchainImages[NUM_BUFFERS] = { VK_NULL_HANDLE };
    uint32_t presentQueueFamilyIndex = (uint32_t)-1;
    VkFramebuffer framebuffers[NUM_BUFFERS] = { VK_NULL_HANDLE };
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent;
    VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE, drawingCompleteSemaphore = VK_NULL_HANDLE;
    VkFormat surfaceFormat = VK_FORMAT_UNDEFINED;
public:
    GraphicsVKIniter(uint32_t renderWidth, uint32_t renderHeight, View::ViewportType viewportType);

    bool startGfx(ANativeWindow* nativeWindow);
    void stopGfx();

    View& getDefaultView() { return view; }
	uint32_t getMemoryTypeIndex(uint32_t memoryRequirementsIndexBits, VkMemoryPropertyFlags memoryPropertyFlags) const;
	VkRenderPass createRenderPass(VkFormat surfaceFormat, VkImageLayout finalLayout);
	VkPipeline createPipeline(const VulkanShader& shader, const VulkanDescriptorSetLayout& descriptorSetLayout,
		VkRenderPass renderPass);
	VkSampler createSampler();
private:
    bool createInstance();
	bool createDebugExt();
    VkPhysicalDevice getGPU() const;
    bool createDevice(VkPhysicalDevice gpu, VkSurfaceKHR surface);
    bool createSurface(ANativeWindow* nativeWindow);

    VkFormat getSurfaceFormat(VkPhysicalDevice gpu, VkSurfaceKHR surface) const;
    VkPresentModeKHR getPresentMode(VkPhysicalDevice gpu, VkSurfaceKHR surface) const;
    uint32_t getNumSwapchainImageBuffers(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    VkExtent2D getSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    VkSurfaceTransformFlagBitsKHR getSwapchainTransformFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    VkCompositeAlphaFlagBitsKHR getSwapchainAlphaFlag(VkSurfaceCapabilitiesKHR surfaceCapabilities) const;
    bool createSwapchain(uint32_t minImageCount, VkSurfaceKHR surface, VkFormat surfaceFormat, VkExtent2D swapchainExtent,
            VkSurfaceTransformFlagBitsKHR transformFlag, VkCompositeAlphaFlagBitsKHR alphaBit);
    bool getSwapchainImagesAndViews(VkFormat surfaceFormat);

    bool createCommandPool();
    bool createFramebuffers(VkExtent2D swapchainExtent);
    bool createSemaphores();

	bool createPipelineCache();
protected:
    VkPipelineVertexInputStateCreateInfo createPipelineVertexInputState(const VulkanVertexLayout& vertexLayout) const;
    VkPipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyState() const;
    VkPipelineRasterizationStateCreateInfo createPipelineRasterizationState() const;
    VkPipelineColorBlendStateCreateInfo createPipelineColorBlendState(VkPipelineColorBlendAttachmentState* attachmentState) const;
    VkPipelineViewportStateCreateInfo createPipelineViewportState(VkViewport* viewport, VkRect2D* scissor) const;
    VkPipelineMultisampleStateCreateInfo createPipelineMultisampleState() const;
};
