#pragma once

#include "vulkan_wrapper.h"
#include "Types.h"
#include "GraphicsVKIniter.h"

class VulkanBuffer
{
    VkDevice device = nullptr;
    uint32_t size = 0;
public:
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
public:
    VulkanBuffer() = default;
    VulkanBuffer(void* data, uint32_t size, VkDevice device, uint32_t graphicsQueueFamilyIndex, VkBufferUsageFlags usageFlag,
		const GraphicsVKIniter* gfx);
    VulkanBuffer(const VulkanBuffer& other) = delete;
    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(const VulkanBuffer& rhs) = delete;
    VulkanBuffer& operator=(VulkanBuffer&& rhs) noexcept;
    ~VulkanBuffer();
    explicit operator bool() const;
    void bindVertexBuffer(VkCommandBuffer commandBuffer);
    void bindIndexBuffer(VkCommandBuffer commandBuffer);
    void subData(uint32_t offset, uint32_t size, const void* data);
	void cmdSubData(const CommandBuffer& commandBuffer, uint32_t offset, uint32_t size, const void* data);
    uint32_t getSize() const;
};
